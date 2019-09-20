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

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#include <nanvix/semaphores.h>
#include <nanvix/syscalls.h>
#include <nanvix/pm.h>

/**
 * @brief Semaphores server connection.
 */
static struct
{
	int initialized; /**< Is the connection initialized? */
	int outbox;      /**< Output mailbox for requests.   */
} server = { 0, -1 };

/**
 * @brief Mailbox module lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*============================================================================*
 * nanvix_sem_init()                                                          *
 *============================================================================*/

/**
 * @brief Initializes the semaphore client.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int nanvix_sem_init(void)
{
	/* Sanity check at compile time: Mailbox compliant */
	CHECK_MAILBOX_MSG_SIZE(struct sem_message);

	/* Nothing to do.  */
	if (server.initialized)
		return (0);

	/* Open output mailbox */
	if ((server.outbox = mailbox_open("/sem-server")) < 0)
	{
		printf("[nanvix][semaphores] cannot open outbox to server\n");
		return (server.outbox);
	}

	server.initialized = 1;

	return (0);
}

/*============================================================================*
 * nanvix_sem_finalize()                                                      *
 *============================================================================*/

/**
 * @brief Closes the semaphore client.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int nanvix_sem_finalize(void)
{
	/* Nothing to do. */
	if (!server.initialized)
		return (0);

	/* Close output mailbox. */
	if (mailbox_close(server.outbox) < 0)
	{
		printf("[nanvix][semaphores] cannot close outbox to server\n");
		return (-EAGAIN);
	}

	server.initialized = 0;

	return (0);
}

/*============================================================================*
 * nanvix_sem_is_valid()                                                      *
 *============================================================================*/

/**
 * @brief Asserts whether or not a semaphore is valid.
 *
 * @param name	Target semaphore.
 *
 * @returns One if the target semaphore is valid, and false
 * otherwise.
 */
static int nanvix_sem_is_valid(sem_t sem)
{
	return ((sem >= 0) && (sem < SEM_MAX));
}

/*============================================================================*
 * nanvix_sem_create()                                                        *
 *============================================================================*/

/**
 * @see nanvix_sem_create()
 */
static inline sem_t _nanvix_sem_create(const char *name, mode_t mode, unsigned value, int excl)
{
	int ret;                /* Return value.                */
	int inbox;              /* Mailbox for small messages.  */
	int nodenum;            /* NoC node number.             */
	struct sem_message msg; /* Semaphore message.         */

	if ((inbox = get_inbox()) < 0)
		return (-EAGAIN);

	nodenum = sys_get_node_num();

	/* Build message header. */
	msg.header.source = nodenum;
	msg.header.opcode = (excl) ? SEM_CREATE_EXCL : SEM_CREATE;

	pthread_mutex_lock(&lock);

		/* Build message 1.*/
		msg.seq = ((nodenum << 4) | 0);
		msg.op.create1.mode = mode;
		msg.op.create1.value = value;

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct sem_message))) != 0)
			goto error;

		/* Build message 2. */
		msg.seq = ((nodenum << 4) | 1);
		strcpy(msg.op.create2.name, name);

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct sem_message))) != 0)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct sem_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg.op.ret);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/**
 * @brief Creates a named semaphore.
 *
 * @param name	Target name.
 * @param mode	User permissions.
 * @param value	Semaphore count value.
 * @param excl  Exclusive creation?
 *
 * @returns Upon successful completion, the ID of the newly created
 * semaphore is returned. Upon failure, a negative error code is
 * returned instead.
 */
sem_t nanvix_sem_create(const char *name, mode_t mode, unsigned value, int excl)
{
	/* Invalid name. */
	if ((name == NULL) || (!strcmp(name, "")))
		return (-EINVAL);

	/* Name too long. */
	if (strlen(name) >= (NANVIX_SEM_NAME_MAX))
		return (-ENAMETOOLONG);

	/* Invalid semaphore value. */
	if (value > SEM_VALUE_MAX)
		return (-EINVAL);

	/* Initilize semaphore client. */
	if (!server.initialized)
		return (-EAGAIN);

	return (_nanvix_sem_create(name, mode, value, excl));
}

/*============================================================================*
 * nanvix_sem_open ()                                                         *
 *============================================================================*/

/**
 * @see nanvix_sem_open().
 */
static inline sem_t _nanvix_sem_open(const char *name)
{
	int ret;                /* Return value.                */
	int inbox;              /* Mailbox for small messages.  */
	int nodenum;            /* NoC node number.             */
	struct sem_message msg; /* Semaphore message.           */

	if ((inbox = get_inbox()) < 0)
		return (-EAGAIN);

	nodenum = sys_get_node_num();

	/* Build message header. */
	msg.header.source = nodenum;
	msg.header.opcode = SEM_OPEN;
	msg.seq = ((nodenum << 4) | 0);
	strcpy(msg.op.open.name, name);

	pthread_mutex_lock(&lock);

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct sem_message))) != 0)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct sem_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg.op.ret);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/**
 * @brief Opens a named semaphore.
 *
 * @param name	Target name.
 * @param mode	User permissions.
 * @param value	Semaphore count value.
 * @param excl  Exclusive creation?
 *
 * @returns Upon successful completion, the ID of the target semaphore
 * is returned. Upon failure, a negative error code is returned
 * instead.
 */
sem_t nanvix_sem_open(const char *name)
{
	/* Invalid name. */
	if ((name == NULL) || (!strcmp(name, "")))
		return (-EINVAL);

	/* Name too long. */
	if (strlen(name) >= (NANVIX_SEM_NAME_MAX))
		return (-ENAMETOOLONG);

	/* Initilize semaphore client. */
	if (!server.initialized)
		return (-EAGAIN);

	return (_nanvix_sem_open(name));
}

/*============================================================================*
 * nanvix_sem_post()                                                          *
 *============================================================================*/

/**
 * @see nanvix_set_post()
 */
static inline int _nanvix_sem_post(sem_t sem)
{
	int ret;                /* Return value.                */
	struct sem_message msg; /* Semaphore message.           */
	int nodenum;            /* NoC node number.             */
	int inbox;              /* Mailbox for small messages.  */

	if ((inbox = get_inbox()) < 0)
		return (-EAGAIN);

	nodenum = sys_get_node_num();

	/* Build message. */
	msg.header.source = nodenum;
	msg.header.opcode = SEM_POST;
	msg.seq = ((nodenum << 4) | 0);
	msg.op.post.semid = sem;

	pthread_mutex_lock(&lock);

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct sem_message))) != 0)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct sem_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg.op.ret);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/**
 * @brief Post on a named semaphore.
 *
 * @param sem Target semaphore.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int nanvix_sem_post(sem_t sem)
{
	/* Invalid semaphore. */
	if (!nanvix_sem_is_valid(sem))
		return (-EINVAL);

	/* Initilize semaphore client. */
	if (!server.initialized)
		return (-EAGAIN);
	
	return (_nanvix_sem_post(sem));
}

/*============================================================================*
 * nanvix_sem_wait()                                                          *
 *============================================================================*/

/**
 * @see nanvix_sem_wait()
 */
static inline int _nanvix_sem_wait(sem_t sem)
{
	int ret;                /* Return value.               */
	struct sem_message msg; /* Semaphore message.          */
	int nodenum;            /* NoC node number.            */
	int inbox;              /* Mailbox for small messages. */

	if ((inbox = get_inbox()) < 0)
		return (-EAGAIN);

	nodenum = sys_get_node_num();

	/* Build message. */
	msg.header.source = nodenum;
	msg.header.opcode = SEM_WAIT;
	msg.seq = ((nodenum << 4) | 0);
	msg.op.wait.semid = sem;

	pthread_mutex_lock(&lock);

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct sem_message))) != 0)
			goto error;

		do
		{
			if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct sem_message))) != MAILBOX_MSG_SIZE)
				goto error;
		} while (msg.header.opcode == SEM_WAIT);

	pthread_mutex_unlock(&lock);

	return (msg.op.ret);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/**
 * @brief Wait a named semaphore.
 *
 * @param sem Target semaphore.
 *
 * @returns Upon successful completion, zero is returned.  Upon
 * failure, a negative error code is returned instead.
 */
int nanvix_sem_wait(sem_t sem)
{
	/* Invalid semaphore. */
	if (!nanvix_sem_is_valid(sem))
		return (-EINVAL);

	/* Initilize semaphore client. */
	if (!server.initialized)
		return (-EAGAIN);

	return (_nanvix_sem_wait(sem));
}

/*============================================================================*
 * nanvix_sem_close()                                                         *
 *============================================================================*/

/**
 * @see nanvix_sem_close()
 */
static inline int _nanvix_sem_close(sem_t sem)
{
	int ret;                /* Return value.               */
	struct sem_message msg; /* Semaphore message.          */
	int nodenum;            /* NoC node number.            */
	int inbox;              /* Mailbox for small messages. */

	if ((inbox = get_inbox()) < 0)
		return (-EAGAIN);

	nodenum = sys_get_node_num();

	/* Build message. */
	msg.header.source = nodenum;
	msg.header.opcode = SEM_CLOSE;
	msg.seq = ((nodenum << 4) | 0);
	msg.op.close.semid = sem;

	pthread_mutex_lock(&lock);

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct sem_message))) != 0)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct sem_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg.op.ret);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/**
 * @brief Close a named semaphore.
 *
 * @param sem	Target semaphore.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int nanvix_sem_close(sem_t sem)
{
	/* Invalid semaphore. */
	if (!nanvix_sem_is_valid(sem))
		return (-EINVAL);

	/* Initilize semaphore client. */
	if (!server.initialized)
		return (-EAGAIN);

	return (_nanvix_sem_close(sem));
}

/*============================================================================*
 * nanvix_sem_unlink()                                                        *
 *============================================================================*/
/**
 * @see nanvix_sem_unlink()
 */
static inline int _nanvix_sem_unlink(const char *name)
{
	int ret;                /* Return value.               */
	struct sem_message msg; /* Semaphore message.          */
	int nodenum;            /* NoC node number.            */
	int inbox;              /* Mailbox for small messages. */

	if ((inbox = get_inbox()) < 0)
		return (-EAGAIN);

	nodenum = sys_get_node_num();

	/* Build message. */
	msg.header.source = nodenum;
	msg.header.opcode = SEM_UNLINK;
	msg.seq = ((nodenum << 4) | 0);
	strcpy(msg.op.unlink.name, name);

	pthread_mutex_lock(&lock);

		if ((ret = mailbox_write(server.outbox, &msg, sizeof(struct sem_message))) != 0)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct sem_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg.op.ret);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/**
 * @brief Unlinks a named semaphore.
 *
 * @param name	Target semaphore name.
 *
 * @returns Upon successful completion, 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
int nanvix_sem_unlink(const char *name)
{
	/* Invalid name. */
	if ((name == NULL) || (!strcmp(name, "")))
		return (-ENOENT);

	/* Name too long. */
	if (strlen(name) >= (NANVIX_SEM_NAME_MAX))
		return (-ENAMETOOLONG);

	/* Initilize semaphore client. */
	if (!server.initialized)
		return (-EAGAIN);

	return (_nanvix_sem_unlink(name));
}
