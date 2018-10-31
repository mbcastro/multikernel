/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include <nanvix/syscalls.h>
#include <nanvix/const.h>
#include <nanvix/pm.h>
#include <nanvix/mqueue.h>

/**
 * @brief Semaphores server connection.
 */
static struct
{
	int initialized; /**< Is the connection initialized?       */
	int outbox;      /**< Output mailbox for sending requests. */
	int outportal;   /**< Output mailbox for sending messages. */
} server = { 0, -1, -1 };

/**
 * @brief Mailbox module lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*============================================================================*
 * Client cache                                                               *
 *============================================================================*/

/**
 * @brief Flags for message queues.
 */
/*@{@*/
#define MQUEUE_READ  (1 << 0) /**< Readable? */
#define MQUEUE_WRITE (1 << 1) /**< Writable? */
/**@}*/

/**
 * @brief Number of opened message queues.
 */
int nopen;

/**
 * Table of opened message queues.
 */
struct
{
	int mqueueid;
	int flags;
} omqueues[MQUEUE_OPEN_MAX];

/*============================================================================*
 * mqueue_may_read()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node may read on a opened message
 * queue.
 *
 * @param id   ID of the target opened message queue.
 *
 * @returns One if the target node may read into the target opened
 * message queue, and zero otherwise.
 */
static inline int mqueue_may_read(int id)
{
	return (omqueues[id].flags & MQUEUE_READ);
}

/*============================================================================*
 * mqueue_may_write()                                                         *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node may write on a opened message
 * queue.
 *
 * @param id   ID of the target opened message queue.
 *
 * @returns One if the target node may write into the target opened
 * message queue, and zero otherwise.
 */
static inline int mqueue_may_write(int id)
{
	return (omqueues[id].flags & MQUEUE_WRITE);
}

/*============================================================================*
 * mqueue_has_opened()                                                        *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node has opened a given message
 * queue.
 *
 * @param mqueueid ID of target message queue.
 *
 * @return If the target node has opened the target message queue, its
 * index in the table of opened message queues is returned. Otherwise,
 * -1 is returned instead.
 */
static int mqueue_has_opened(int mqueueid)
{
	for (int i = 0; i < nopen; i++)
	{
		if (omqueues[i].mqueueid == mqueueid)
			return (i);
	}

	return (-1);
}

/*============================================================================*
 * mqueue_clear_flags()                                                       *
 *============================================================================*/

/**
 * @brief Clears the flags of a opened message queue.
 *
 * @param id   ID of the target opened message queue.
 */
static inline void mqueue_clear_flags(int id)
{
	omqueues[id].flags = 0;
}

/*============================================================================*
 * mqueue_set_readable()                                                      *
 *============================================================================*/

/**
 * @brief Sets a target opened message queue as readable.
 *
 * @param id   ID of the target opened message queue.
 */
static inline void mqueue_set_readable(int id)
{
	omqueues[id].flags |= MQUEUE_READ;
}

/*============================================================================*
 * mqueue_set_writable()                                                      *
 *============================================================================*/

/**
 * @brief Sets a target opened message queue as writable.
 *
 * @param id   ID of the target opened message queue.
 */
static inline void mqueue_set_writable(int id)
{
	omqueues[id].flags |= MQUEUE_WRITE;
}

/*============================================================================*
 * nanvix_mqueue_is_invalid_name()                                            *
 *============================================================================*/

/**
 * @brief Asserts whether or not a message queue has a valid name.
 *
 * @param name Target name.
 *
 * @returns Zero if the target @p name is valid and a negative error
 * code otherwise.
 */
static inline int nanvix_mqueue_is_invalid_name(const char *name)
{
	/* Invalid name. */
	if ((name == NULL) || (!strcmp(name, "")))
	{
		errno = EINVAL;
		return (-1);
	}

	/* Name too long. */
	if (strlen(name) >= (NANVIX_MQUEUE_NAME_MAX - 1))
	{
		errno = ENAMETOOLONG;
		return (-1);
	}

	return (0);
}

/*============================================================================*
 * nanvix_mqueue_init()                                                       *
 *============================================================================*/

/**
 * @brief Initializes the message queue client.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int nanvix_mqueue_init(void)
{
	/* Sanity check at compile time: Mailbox compliant */
	CHECK_MAILBOX_MSG_SIZE(struct mqueue_message);

	/* Nothing to do.  */
	if (server.initialized)
		return (0);

	/* Open output mailbox */
	if ((server.outbox = mailbox_open("/mqueue-server")) < 0)
	{
		printf("[nanvix][mqueue] cannot open outbox to server\n");
		return (server.outbox);
	}

	/* Open output portal */
	if ((server.outportal = portal_open("/mqueue-server")) < 0)
	{
		printf("[nanvix][mqueue] cannot open outportal to server\n");
		return (server.outportal);
	}

	server.initialized = 1;

	return (0);
}

/*============================================================================*
 * nanvix_mqueue_cleanup()                                                    *
 *============================================================================*/

/**
 * @brief Closes the message queue client.
 *
 * @param Upon successful completion, zero is returned. Upon failure,
 * -1 is returned instead, and errno is set to indicate the error.
 */
int nanvix_mqueue_cleanup(void)
{
	/* Nothing to do. */
	if (!server.initialized)
		return (0);

	/* Close underlying output mailbox. */
	if (mailbox_close(server.outbox) < 0)
	{
		printf("[nanvix][mqueue] cannot close outbox to server\n");
		errno = EAGAIN;
		return (-1);
	}

	/* Close underlying output portal. */
	if (portal_close(server.outportal) < 0)
	{
		printf("[nanvix][mqueue] cannot close outportal to server\n");
		errno = EAGAIN;
		return (-1);
	}

	server.initialized = 0;

	return (0);
}

/*============================================================================*
 * nanvix_mqueue_create()                                                     *
 *============================================================================*/

/**
 * @brief Creates a message queue.
 *
 * @param name     Name of the target message queue.
 * @param readable Is the target message queue readable?
 * @param writable Is the target message queue writable?
 * @param mode     Access permissions.
 *
 * @param Upon successful completion, a descriptor of the target
 * message queue is returned. Upon failure, -1 is returned instead and
 * errno is set to indicate the error.
 */
int nanvix_mqueue_create(
	const char *name,
	int readable,
	int writable,
	mode_t mode)
{
	int i;
	int ret;
	int inbox;
	int nodenum;
	struct mqueue_message msg;

	/* Invalid name. */
	if (nanvix_mqueue_is_invalid_name(name))
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Invalid opening mode. */
	if ((!readable) && (!writable))
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Uninitalized server. */
	if (!server.initialized)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Get input mailbox. */
	if ((inbox = get_inbox()) < 0)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Too many files are opened. */
	if (nopen >= MQUEUE_OPEN_MAX)
	{
		errno = ENFILE;
		return (-1);
	}

	nodenum = sys_get_node_num();

	/* Build message header. */
	msg.source = nodenum;
	msg.opcode = MQUEUE_CREATE;

	pthread_mutex_lock(&lock);

		/* Build message 1.*/
		msg.seq = ((nodenum << 4) | 0);
		msg.op.create1.mode = mode;
		msg.op.create1.readable = readable;
		msg.op.create1.writable = writable;

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct mqueue_message))) != 0)
			goto error;

		/* Build message 2. */
		msg.seq = ((nodenum << 4) | 1);
		strcpy(msg.op.create2.name, name);

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct mqueue_message))) != 0)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct mqueue_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	/* Failed to create. */
	if (msg.opcode == MQUEUE_FAILURE)
	{
		errno = msg.op.ret.status;
		return (-1);
	}

	i = nopen++;
	omqueues[i].mqueueid = msg.op.ret.mqueueid;
	mqueue_clear_flags(i);
	if (writable)
		mqueue_set_writable(i);
	if (readable)
		mqueue_set_readable(i);

	return (msg.op.ret.mqueueid);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/*============================================================================*
 * nanvix_mqueue_create_excl()                                                *
 *============================================================================*/

/**
 * @brief Creates an exclusive message queue.
 *
 * @param name     Name of the target message queue.
 * @param readable Is the target message queue readable?
 * @param writable Is the target message queue writable?
 * @param mode     Access permissions.
 *
 * @param Upon successful completion, a descriptor of the newly
 * created message queue is returned. Upon failure, -1 is returned
 * instead and errno is set to indicate the error.
 */
int nanvix_mqueue_create_excl(
	const char *name,
	int readable,
	int writable,
	mode_t mode)
{
	int i;
	int ret;
	int inbox;
	int nodenum;
	struct mqueue_message msg;

	/* Invalid name. */
	if (nanvix_mqueue_is_invalid_name(name))
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Invalid opening mode. */
	if ((!readable) && (!writable))
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Uninitalized server. */
	if (!server.initialized)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Get input mailbox. */
	if ((inbox = get_inbox()) < 0)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Too many files are opened. */
	if (nopen >= MQUEUE_OPEN_MAX)
	{
		errno = ENFILE;
		return (-1);
	}

	nodenum = sys_get_node_num();

	/* Build message header. */
	msg.source = nodenum;
	msg.opcode = MQUEUE_CREATE_EXCL;

	pthread_mutex_lock(&lock);

		/* Build message 1.*/
		msg.seq = ((nodenum << 4) | 0);
		msg.op.create1.mode = mode;
		msg.op.create1.readable = readable;
		msg.op.create1.writable = writable;

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct mqueue_message))) != 0)
			goto error;

		/* Build message 2. */
		msg.seq = ((nodenum << 4) | 1);
		strcpy(msg.op.create2.name, name);

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct mqueue_message))) != 0)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct mqueue_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	/* Failed to create. */
	if (msg.opcode == MQUEUE_FAILURE)
	{
		errno = msg.op.ret.status;
		return (-1);
	}

	i = nopen++;
	omqueues[i].mqueueid = msg.op.ret.mqueueid;
	mqueue_clear_flags(i);
	if (writable)
		mqueue_set_writable(i);
	if (readable)
		mqueue_set_readable(i);

	return (msg.op.ret.mqueueid);

error:
	pthread_mutex_unlock(&lock);
	errno = -ret;
	return (ret);
}

/*============================================================================*
 * nanvix_mqueue_open()                                                       *
 *============================================================================*/

/**
 * @brief Opens a message queue.
 *
 * @param name     Name of the target message queue.
 * @param readable Is the target message queue readable?
 * @param writable Is the target message queue writable?
 *
 * @param Upon successful completion, a descriptor of the target
 * message queue is returned. Upon failure, -1 is returned instead and
 * errno is set to indicate the error.
 */
int nanvix_mqueue_open(const char *name, int readable, int writable)
{
	int i;
	int ret;
	int inbox;
	int nodenum;
	struct mqueue_message msg;

	/* Invalid name. */
	if (nanvix_mqueue_is_invalid_name(name))
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Invalid opening mode. */
	if ((!readable) && (!writable))
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Uninitalized server. */
	if (!server.initialized)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Get input mailbox. */
	if ((inbox = get_inbox()) < 0)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Too many files are opened. */
	if (nopen >= MQUEUE_OPEN_MAX)
	{
		errno = ENFILE;
		return (-1);
	}

	nodenum = sys_get_node_num();

	/* Build message header. */
	msg.source = nodenum;
	msg.opcode = MQUEUE_OPEN;

	pthread_mutex_lock(&lock);

		/* Build message 1.*/
		msg.seq = ((nodenum << 4) | 0);
		msg.op.open1.readable = readable;
		msg.op.open1.writable = writable;

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct mqueue_message))) != 0)
			goto error;

		/* Build message 2. */
		msg.seq = ((nodenum << 4) | 1);
		strcpy(msg.op.open2.name, name);

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct mqueue_message))) != 0)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct mqueue_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	/* Failed to create. */
	if (msg.opcode == MQUEUE_FAILURE)
	{
		errno = msg.op.ret.status;
		return (-1);
	}
	
	i = nopen++;
	omqueues[i].mqueueid = msg.op.ret.mqueueid;
	mqueue_clear_flags(i);
	if (writable)
		mqueue_set_writable(i);
	if (readable)
		mqueue_set_readable(i);

	return (msg.op.ret.mqueueid);

error:
	pthread_mutex_unlock(&lock);
	errno = -ret;
	return (ret);
}

/*============================================================================*
 * nanvix_mqueue_unlink()                                                     *
 *============================================================================*/

/**
 * @brief removes a message queue.
 *
 * @param name Name of the target message queue.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, -1 is returned instead, and errno is set to indicate the
 * error.
 */
int nanvix_mqueue_unlink(const char *name)
{
	int i;
	int ret;
	int inbox;
	int nodenum;
	struct mqueue_message msg;

	/* Invalid name. */
	if (nanvix_mqueue_is_invalid_name(name))
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Uninitalized server. */
	if (!server.initialized)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Get input mailbox. */
	if ((inbox = get_inbox()) < 0)
	{
		errno = EAGAIN;
		return (-1);
	}

	nodenum = sys_get_node_num();

	/* Build message. */
	msg.source = nodenum;
	msg.opcode = MQUEUE_UNLINK;
	msg.seq = ((nodenum << 4) | 0);
	strcpy(msg.op.unlink.name, name);

	pthread_mutex_lock(&lock);

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct mqueue_message))) != 0)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct mqueue_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	/* Failed to unlink. */
	if (msg.opcode == MQUEUE_FAILURE)
	{
		errno = msg.op.ret.status;
		return (-1);
	}

	/*
	 * The process should have opened
	 * the mqueue before.
	 */
	if ((i = mqueue_has_opened(msg.op.ret.mqueueid)) < 0)
	{
		errno = EACCES;
		return (-1);
	}

	/* Remove the message queue from the list. */
	--nopen;
	for (int j = i; j < nopen; j++)
	{
		omqueues[j].mqueueid = omqueues[j + 1].mqueueid;
		omqueues[j].flags    = omqueues[j + 1].flags;
	}

	return (0);

error:
	pthread_mutex_unlock(&lock);
	errno = -ret;
	return (ret);
}


/*============================================================================*
 * nanvix_mqueue_unlink()                                                     *
 *============================================================================*/

/**
 * @brief Closes a message queue.
 *
 * @param mqueueid ID of the target message queue.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, -1 is returned instead, and errno is set to indicate the
 * error.
 */
int nanvix_mqueue_close(int mqueueid)
{
	int i;
	int ret;
	int inbox;
	int nodenum;
	struct mqueue_message request;
	struct mqueue_message response;

	/* Invalid descriptor. */
	if (mqueueid < 0)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Uninitalized server. */
	if (!server.initialized)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Get input mailbox. */
	if ((inbox = get_inbox()) < 0)
	{
		errno = EAGAIN;
		return (-1);
	}

	/*
	 * The process should have opened
	 * this message queue before.
	 */
	if ((i = mqueue_has_opened(mqueueid)) < 0)
	{
		errno = EACCES;
		return (-1);
	}

	/* Cannot write. */
	if (!mqueue_may_write(i))
	{
		errno = EACCES;
		return (-1);
	}

	nodenum = sys_get_node_num();

	/* Build message. */
	request.source = nodenum;
	request.opcode = MQUEUE_CLOSE;
	request.seq = ((nodenum << 4) | 0);
	request.op.close.mqueueid = mqueueid;

	pthread_mutex_lock(&lock);

		/* Send request. */
		if ((ret = mailbox_write(server.outbox, &request, sizeof(struct mqueue_message))) != 0)
			goto error;

		/* Wait response. */
		if ((ret = sys_mailbox_read(inbox, &response, sizeof(struct mqueue_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	/* Failed to send. */
	if (response.opcode == MQUEUE_FAILURE)
	{
		errno = response.op.ret.status;
		return (-1);
	}

	/* Remove the message queue from the list. */
	--nopen;
	for (int j = i; j < nopen; j++)
	{
		omqueues[j].mqueueid = omqueues[j + 1].mqueueid;
		omqueues[j].flags = omqueues[j + 1].flags;
	}

	return (0);

error:
	pthread_mutex_unlock(&lock);
	errno = -ret;
	return (ret);
}

/*============================================================================*
 * nanvix_mqueue_send()                                                       *
 *============================================================================*/

/**
 * @brief Sends a message to a message queue.
 *
 * @param mqueueid ID of the target message queue.
 * @param msg      Target message.
 * @param len      Length of the target message (in bytes).
 * @param prio     Priority of the target message.
 *
 * @param Upon successful completion, zero is returned. Upon failure,
 * no message is enqueued, -1 is returned and errno is set to indicate
 * the error.
 */
int nanvix_mqueue_send(int mqueueid, const char *msg, size_t len, unsigned prio)
{
	int i;
	int ret;
	int inbox;
	int nodenum;
	struct mqueue_message request;
	struct mqueue_message response;

	/* Invalid descriptor. */
	if (mqueueid < 0)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Invalid message. */
	if (msg == NULL)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Invalid length. */
	if (len > MQUEUE_MESSAGE_SIZE)
	{
		errno = EMSGSIZE;
		return (-1);
	}

	/* Invalid priority. */
	if (prio >= MQUEUE_PRIO_MAX)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Uninitalized server. */
	if (!server.initialized)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Get input mailbox. */
	if ((inbox = get_inbox()) < 0)
	{
		errno = EAGAIN;
		return (-1);
	}

	/*
	 * The process should have opened
	 * this message queue before.
	 */
	if ((i = mqueue_has_opened(mqueueid)) < 0)
	{
		errno = EACCES;
		return (-1);
	}

	/* Cannot write. */
	if (!mqueue_may_write(i))
	{
		errno = EACCES;
		return (-1);
	}

	nodenum = sys_get_node_num();

	/* Build message. */
	request.source = nodenum;
	request.opcode = MQUEUE_SEND;
	request.seq = ((nodenum << 4) | 0);
	request.op.send.mqueueid = mqueueid;
	request.op.send.len = len;
	request.op.send.prio = prio;

	pthread_mutex_lock(&lock);

		/* Send request. */
		if ((ret = mailbox_write(server.outbox, &request, sizeof(struct mqueue_message))) != 0)
			goto error;
		
		/* Wait permission. */
		if ((ret = sys_mailbox_read(inbox, &response, sizeof(struct mqueue_message))) != MAILBOX_MSG_SIZE)
			goto error;
	
	pthread_mutex_unlock(&lock);
		
	/* Failed to send. */
	if (response.opcode == MQUEUE_FAILURE)
	{
		errno = -response.op.ret.status;
		return (-1);
	}
	
	/* Send message. */
	if ((ret = portal_write(server.outportal, msg, len)) != (int) len)
	{
		errno = -ret;
		return (-1);
	}

	pthread_mutex_lock(&lock);

		/* Wait response. */
		if ((ret = sys_mailbox_read(inbox, &response, sizeof(struct mqueue_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	/* Failed to send. */
	if (response.opcode == MQUEUE_FAILURE)
	{
		errno = response.op.ret.status;
		return (-1);
	}

	return (0);

error:
	pthread_mutex_unlock(&lock);
	errno = -ret;
	return (ret);
}

/*============================================================================*
 * nanvix_mqueue_receive()                                                    *
 *============================================================================*/

/**
 * @brief Sends a message to a message queue.
 *
 * @param mqueueid ID of the target message queue.
 * @param msg      Location to store the received message.
 * @param len      Length of the received message (in bytes).
 * @param prio     Location to store the priority of the received message.
 *
 * @param Upon successful completion, zero is returned. Upon failure,
 * no message is enqueued, -1 is returned and errno is set to indicate
 * the error.
 */
ssize_t nanvix_mqueue_receive(int mqueueid, char *msg, size_t len, unsigned *prio)
{
	int i;
	int ret;
	int inbox;
	int nodenum;
	int inportal;
	struct mqueue_message request;
	struct mqueue_message response;

	/* Invalid descriptor. */
	if (mqueueid < 0)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Invalid message. */
	if (msg == NULL)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Invalid length. */
	if (len > MQUEUE_MESSAGE_SIZE)
	{
		errno = EMSGSIZE;
		return (-1);
	}

	/* Uninitalized server. */
	if (!server.initialized)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Get input mailbox. */
	if ((inbox = get_inbox()) < 0)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Get input portal. */
	if ((inportal = get_inportal()) < 0)
	{
		errno = EAGAIN;
		return (-1);
	}

	/*
	 * The process should have opened
	 * this message queue before.
	 */
	if ((i = mqueue_has_opened(mqueueid)) < 0)
	{
		errno = EACCES;
		return (-1);
	}

	/* Cannot read. */
	if (!mqueue_may_read(i))
	{
		errno = EACCES;
		return (-1);
	}

	nodenum = sys_get_node_num();

	/* Build message. */
	request.source = nodenum;
	request.opcode = MQUEUE_RECEIVE;
	request.seq = ((nodenum << 4) | 0);
	request.op.receive.mqueueid = mqueueid;
	request.op.receive.len = len;

	pthread_mutex_lock(&lock);

		/* Send request. */
		if ((ret = mailbox_write(server.outbox, &request, sizeof(struct mqueue_message))) != 0)
			goto error;
		
		/* Wait permission. */
		if ((ret = sys_mailbox_read(inbox, &response, sizeof(struct mqueue_message))) != MAILBOX_MSG_SIZE)
			goto error;

		/* Enable write. */
		if ((ret = sys_portal_allow(inportal, MQUEUE_SERVER_NODE)) < 0)
			goto error;

	pthread_mutex_unlock(&lock);

	/* Failed to receive. */
	if (response.opcode == MQUEUE_FAILURE)
	{
		errno = response.op.ret.status;
		return (-1);
	}

	/* Receive message. */
	if ((ret = sys_portal_read(inportal, msg, len)) != (int) len)
	{
		errno = -ret;
		return (-1);
	}

	pthread_mutex_lock(&lock);

		/* Wait response. */
		if ((ret = sys_mailbox_read(inbox, &response, sizeof(struct mqueue_message))) != MAILBOX_MSG_SIZE)
			goto error;
	
	pthread_mutex_unlock(&lock);

	/* Save priority. */
	if (prio != NULL)
		*prio = response.op.ret.prio;

	return (len);

error:
	pthread_mutex_unlock(&lock);
	errno = -ret;
	return (ret);
}
