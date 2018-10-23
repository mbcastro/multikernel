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
#include <assert.h>
#include <stdio.h>

#include <nanvix/spawner.h>
#include <nanvix/mqueue.h>
#include <nanvix/name.h>
#include <nanvix/syscalls.h>

#include "mqueue.h"

/**
 * @brief Flags for message queues.
 */
/*@{@*/
#define MQUEUE_READ  (1 << 0) /**< Readable? */
#define MQUEUE_WRITE (1 << 1) /**< Writable? */
#define MQUEUE_USED  (1 << 2) /**< Used?     */
/**@}*/

/**
 * @brief Table of processes.
 */
static struct
{
	/**
	 * Table of opened message queues.
	 */
	struct
	{
		int mqueueid; /**< Low-level Message Queue ID. */
		int flags;    /**< Opening Flags.              */
	} omqueues[MQUEUE_OPEN_MAX];
} procs[HAL_NR_NOC_NODES];

/**
 * @brief Input mailbox for requests.
 */
static int inbox;

/**
 * @biref Input portal for data transfers.
 */
static int inportal;

/*============================================================================*
 * mqueue_may_read()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node may read on a opened message
 * queue.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened message queue.
 *
 * @returns One if the target node may read into the target opened
 * message queue, and zero otherwise.
 */
static inline int mqueue_may_read(int node, int id)
{
	return (procs[node].omqueues[id].flags & MQUEUE_READ);
}

/*============================================================================*
 * mqueue_may_write()                                                         *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node may write on a opened message
 * queue.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened message queue.
 *
 * @returns One if the target node may write into the target opened
 * message queue, and zero otherwise.
 */
static inline int mqueue_may_write(int node, int id)
{
	return (procs[node].omqueues[id].flags & MQUEUE_WRITE);
}

/*============================================================================*
 * mqueue_name_is_valid()                                                     *
 *============================================================================*/

/**
 * @brief Asserts whether or not a message queue name is valid.
 *
 * @param name Target name.
 *
 * @returns One if the target message queue name is valid, and zero
 * otherwise.
 */
static inline int mqueue_name_is_valid(const char *name)
{
	return ((name != NULL) &&
			(strlen(name) < (NANVIX_MQUEUE_NAME_MAX - 1)) &&
			(strcmp(name, ""))
	);
}

/*============================================================================*
 * omqueue_is_valid()                                                         *
 *============================================================================*/

/**
 * @brief Asserts whether or not a opened message queue ID is valid.
 *
 * @param omqueueid ID of the target opened message queue.
 *
 * @returns Non-zero if the opened message queue valid, and 
 * zero otherwise.
 */
static inline int omqueue_is_valid(int omqueueid)
{
	return ((omqueueid >= 0) && (omqueueid < MQUEUE_OPEN_MAX));
}

/*============================================================================*
 * omqueue_is_used()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a opened message queue slot is used.
 *
 * @param node  Target node.
 * @param id ID of the opened message queue.
 *
 * @returns Non-zero if the slot is marked as used, and zero otherwise.
 */
int omqueue_is_used(int node, int id)
{
	return (omqueue_is_valid(id) && (procs[node].omqueues[id].flags & MQUEUE_USED));
}

/*============================================================================*
 * mqueue_clear_flags()                                                       *
 *============================================================================*/

/**
 * @brief Clears the flags of a opened message queue.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened message queue.
 */
static inline void mqueue_clear_flags(int node, int id)
{
	procs[node].omqueues[id].flags = 0;
}

/*============================================================================*
 * mqueue_set_readable()                                                      *
 *============================================================================*/

/**
 * @brief Sets a target opened message queue as readable.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened message queue.
 */
static inline void mqueue_set_readable(int node, int id)
{
	procs[node].omqueues[id].flags |= MQUEUE_READ;
}

/*============================================================================*
 * mqueue_set_writable()                                                      *
 *============================================================================*/

/**
 * @brief Sets a target opened message queue as writable.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened message queue.
 */
static inline void mqueue_set_writable(int node, int id)
{
	procs[node].omqueues[id].flags |= MQUEUE_WRITE;
}

/*============================================================================*
 * mqueue_set_used()                                                          *
 *============================================================================*/

/**
 * @brief Sets a message queue as used.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened message queue.
 */
static inline void mqueue_set_used(int node, int id)
{
	procs[node].omqueues[id].flags |= MQUEUE_USED;
}

/*============================================================================*
 * omqueue_alloc()                                                             *
 *============================================================================*/

/**
 * @brief Allocates a opened message queue.
 *
 * @param node Number of the target node.
 * 
 * @return Upon successful completion, the ID of the newly allocated
 * message queue is returned. Upon failure, -1 is returned instead.
 */
int omqueue_alloc(int node)
{
	/* Search for a free message queue. */
	for (int i = 0; i < MQUEUE_OPEN_MAX; i++)
	{
		/* Found. */
		if (!omqueue_is_used(node, i))
		{
			mqueue_clear_flags(node, i);
			mqueue_set_used(node, i);
			return (i);
		}
	}

	return (-1);
}

/*============================================================================*
 * omqueue_free()                                                              *
 *============================================================================*/

/**
 * @brief Free a openend message queue.
 * 
 * @param node Number of the target node.
 * @param omqueueid ID of the opened message queue.
 */
static void omqueue_free(int node, int id)
{
	mqueue_clear_flags(node, id);
}

/*============================================================================*
 * mqueue_has_opened()                                                        *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node has opened a given message
 * queue.
 *
 * @param node  Target node.
 * @param mqueueid ID of target message queue.
 *
 * @return If the target node has opened the target message queue, its
 * index in the table of opened message queues is returned. Otherwise,
 * -1 is returned instead.
 */
static int mqueue_has_opened(int node, int mqueueid)
{
	for (int i = 0; i < MQUEUE_OPEN_MAX; i++)
	{
		if (procs[node].omqueues[i].mqueueid != mqueueid)
			continue;

		if (omqueue_is_used(node, i))
			return (i);
	}

	return (-1);
}

/*============================================================================*
 * mqueue_valid_receive()                                                     *
 *============================================================================*/

/**
 * @brief Asserts whether or not a message queue allows receives.
 *
 * @param node      ID the calling process.
 * @param omqueueid Reference ID of the target message queue.
 *
 * @returns One if the message queue allows for receives, and zero otherwise.
 */
static int mqueue_valid_receive(int node, int omqueueid)
{
	return !mqueue_is_empty(procs[node].omqueues[omqueueid].mqueueid);
}

/*============================================================================*
 * mqueue_valid_send()                                                        *
 *============================================================================*/

/**
 * @brief Asserts whether or not a message queue allows sends.
 *
 * @param node      ID the calling process.
 * @param omqueueid Reference ID of the target message queue.
 *
 * @returns One if the message queue allows for sends, and zero otherwise.
 */
static int mqueue_valid_send(int node, int omqueueid)
{
	return !mqueue_is_full(procs[node].omqueues[omqueueid].mqueueid);
}

/*============================================================================*
 * mqueue_open()                                                              *
 *============================================================================*/

/**
 * @brief Opens a message queue
 *
 * @param node     ID of opening process.
 * @param name     Name of the targeted message queue.
 * @param readable Readable?
 * @param writable Writable?
 *
 * @returns Upon successful completion, the target message queue ID is
 * returned. Upon failure, a negative error code is returned instead.
 */
static int mqueue_open(int node, const char *name, int readable, int writable)
{
	int mqueueid;  /* Message queue ID. */
	int omqueueid; /* Opened Message queue ID.  */

	mqueue_debug("open node=%d name=%s", node, name);

	/* Invalid name. */
	if (!mqueue_name_is_valid(name))
		return (-EINVAL);

	/* Get message queue. */
	if ((mqueueid = mqueue_get(name)) < 0)
		return (-EINVAL);

	/* Incompatible read flags */
	if (!mqueue_is_readable(mqueueid) && readable)
	{
		mqueue_put(mqueueid);
		return (-EINVAL);
	}

	/* Incompatible write flags */
	if (!mqueue_is_writable(mqueueid) && writable)
	{
		mqueue_put(mqueueid);
		return (-EACCES);
	}

	/* Message queue shall be removed soon. */
	if (mqueue_is_remove(mqueueid))
	{
		mqueue_put(mqueueid);
		return (-EACCES);
	}

	/* Too many files are opened. */
	if ((omqueueid = omqueue_alloc(node)) < 0)
	{
		mqueue_put(mqueueid);
		return (-ENFILE);
	}

	procs[node].omqueues[omqueueid].mqueueid = mqueueid;
	if (writable)
		mqueue_set_writable(node, omqueueid);
	if (readable)
		mqueue_set_readable(node, omqueueid);

	return (omqueueid);
}

/*============================================================================*
 * mqueue_create()                                                            *
 *============================================================================*/

/**
 * @brief Creates a message queue
 *
 * @param owner    ID of owner process.
 * @param name     Name of the targeted mqueue.
 * @param readable Readable?
 * @param writable Writable?
 * @param mode     Access permissions.
 *
 * @returns Upon successful completion, the ID of the newly created
 * opened message queue is returned. Upon failure, a negative error
 * code is returned instead.
 */
static int mqueue_create(int owner, const char *name, int readable, int writable, mode_t mode)
{
	int mqueueid;  /* Message queue ID.        */
	int omqueueid; /* Opened Message queue ID. */

	mqueue_debug("create node=%d name=%s mode=%d", owner, name, mode);

	/* Invalid name. */
	if (!mqueue_name_is_valid(name))
		return (-EINVAL);

	/* Look for message queue. */
	if ((mqueueid = mqueue_get(name)) >= 0)
	{
		mqueue_put(mqueueid);
		return (mqueue_open(owner, name, readable, writable));
	}

	/* Allocate a new opened mqueue. */
	if ((omqueueid = omqueue_alloc(owner)) < 0)
		return (-ENFILE);

	/* Allocate a new mqueue. */
	if ((mqueueid = mqueue_alloc()) < 0)
	{
		omqueue_free(owner, omqueueid);
		return (-EAGAIN);
	}

	/* Initialize message queue. */
	mqueue_set_perm(mqueueid, owner, mode);
	mqueue_set_name(mqueueid, name);
	mqueue_set_size(mqueueid, MQUEUE_MESSAGE_SIZE);

	procs[owner].omqueues[omqueueid].mqueueid = mqueueid;
	if (writable)
		mqueue_set_writable(owner, omqueueid);
	if (readable)
		mqueue_set_readable(owner, omqueueid);

	return (omqueueid);
}

/*============================================================================*
 * mqueue_create_exclusive()                                                  *
 *============================================================================*/

/**
 * @brief Open a message queue with existence check
 *
 * @param owner    ID of owner process.
 * @param name     Name of the targeted message queue.
 * @param readable Readable?
 * @param writable Writable?
 * @param mode     Access permissions.
 *
 * @returns Upon successful completion, the newly created opened
 * message queue ID is returned. Upon failure, a negative error
 * code is returned instead.
 */
static int mqueue_create_exclusive(int owner, char *name, int readable, int writable, mode_t mode)
{
	int mqueueid; /* Message queue ID. */

	mqueue_debug("create-excl node=%d name=%s mode=%d", owner, name, mode);

	/* Invalid name. */
	if (!mqueue_name_is_valid(name))
		return (-EINVAL);

	/* Message queue exists. */
	if ((mqueueid = mqueue_get(name)) >= 0)
	{
		mqueue_put(mqueueid);
		return (-EEXIST);
	}

	return (mqueue_create(owner, name, readable, writable, mode));
}

/*============================================================================*
 * mqueue_close()                                                             *
 *============================================================================*/

/**
 * @brief Close a message queue
 *
 * @param node      ID of opening process.
 * @param omqueueid ID of the opened message queue.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int mqueue_close(int node, int omqueueid)
{
	int mqueueid; /* Message queue ID. */

	mqueue_debug("close node=%d omqueueid=%d", node, omqueueid);

	/* Opened message queue not in use. */
	if (!omqueue_is_used(node, omqueueid))
		return (-EINVAL);

	mqueueid = procs[node].omqueues[omqueueid].mqueueid;

	/* Message queue not in use. */
	if (!mqueue_is_used(mqueueid))
		return (-EINVAL);

	mqueue_put(mqueueid);

	omqueue_free(node, omqueueid);

	return (0);
}

/*============================================================================*
 * mqueue_unlink()                                                            *
 *============================================================================*/

/**
 * @brief Unlink a message queue
 *
 * @param node  ID the calling process.
 * @param mqueueid Target message queue.
 *
 * @returns Upon successful completion, mqueueid is returned.
 * Upon failure, a negative error code is returned instead.
 */
static int mqueue_unlink(int node, const char *name)
{
	int ret;       /* Return of closing.       */
	int mqueueid;  /* Message queue ID.        */
	int omqueueid; /* Opened Message queue ID. */

	mqueue_debug("unlink node=%d name=%s", node, name);

	/* Message queue does not exist. */
	if ((mqueueid = mqueue_get(name)) < 0)
		return (-EINVAL);
	mqueue_put(mqueueid);

	/* Do I own this message queue? */
	if (!mqueue_is_owner(mqueueid, node))
		return (-EPERM);

	/* Opened message queue does not exist. */
	if ((omqueueid = mqueue_has_opened(node, mqueueid)) < 0)
		return (-EINVAL);

	mqueue_set_remove(mqueueid);
	
	/* Did I close the message queue correctly? */
	if ((ret = mqueue_close(node, omqueueid) < 0))
		return ret;

	return omqueueid;
}

/*============================================================================*
 * mqueue_send()                                                              *
 *============================================================================*/

/**
 * @brief Sends a message to a message queue.
 *
 * @param node      ID the calling process.
 * @param omqueueid Reference ID of the target message queue.
 * @param len       Length of the target message (in bytes).
 * @param prio      Priority of the target message.
 *
 * @param Upon successful completion, zero is returned. Upon failure,
 * no message is enqueued, -1 is returned and errno is set to indicate
 * the error.
 */
static int mqueue_send(int node, int omqueueid, size_t len, unsigned prio)
{
	int ret;         /* Return's Cache.   */
	int mqueueid;    /* Message queue ID. */
	void * msg_slot; /* Message slot      */

	mqueue_debug("send node=%d omqueueid=%d len=%d prio=%d", node, omqueueid, len, prio);

	/* Invalid opened message queue ID. */
	if (!omqueue_is_used(node, omqueueid))
		return (-EINVAL);

	mqueueid = procs[node].omqueues[omqueueid].mqueueid;

	/* Invalid message queue. */
	if (!mqueue_is_used(mqueueid))
		return (-EINVAL);

	/* Cannot write. */
	if (!mqueue_may_write(node, omqueueid))
		return (-EACCES);

	/* Invalid size. */
	if (len > mqueue_get_size(mqueueid))
		return (-ENOMEM);

	/* Is the message queue full? */
	if ((msg_slot = mqueue_slot_alloc(mqueueid, prio)) == NULL)
		return (-EAGAIN);

	/* Enable write. */
	if ((ret = sys_portal_allow(inportal, node)) < 0)
	{
		mqueue_slot_free(mqueueid, msg_slot);
		return (ret);
	}

	/* Read message into the target message queue. */
	if ((ret = sys_portal_read(inportal, msg_slot, len)) != (int) len)
	{
		mqueue_slot_free(mqueueid, msg_slot);
		return (ret);
	}

	return (0);
}

/*============================================================================*
 * mqueue_receive()                                                           *
 *============================================================================*/

/**
 * @brief Receives a message from a message queue.
 *
 * @param node      ID the calling process.
 * @param omqueueid Reference ID of the target message queue.
 * @param len       Length of the target message (in bytes).
 * @param prio      Priority of the target message.
 *
 * @param Upon successful completion, the length of the received
 * message in bytes is returned, and the message is removed from the
 * queue. Upon failure, no message is removed from the queue, -1 is
 * returned and errno is set to indicate the error.
 */
static int mqueue_receive(int node, int omqueueid, size_t len, unsigned *prio)
{
	int ret;         /* Return's Cache.   */
	int mqueueid;    /* Message queue ID. */
	int outportal;   /* Output portal.    */
	void * msg_slot; /* Message slot      */

	mqueue_debug("receive node=%d omqueueid=%d len=%d", node, omqueueid, len);
	
	/* Invalid message queue reference. */
	if (!omqueue_is_used(node, omqueueid))
		return (-EINVAL);

	mqueueid = procs[node].omqueues[omqueueid].mqueueid;

	/* Invalid message queue. */
	if (!mqueue_is_used(mqueueid))
		return (-EINVAL);

	/* Cannot read. */
	if (!mqueue_may_read(node, omqueueid))
		return (-EACCES);

	/* Invalid size. */
	if (len < mqueue_get_size(mqueueid))
		return (-ENOMEM);

	/* Is the message queue empty? */
	if ((msg_slot = mqueue_get_first_slot(mqueueid, prio)) == NULL)
		return (-EAGAIN);

	/* Read message from the target message queue. */
	if ((outportal = ret = sys_portal_open(node)) < 0)
		goto error0;
	if ((ret = sys_portal_write(outportal, msg_slot, len)) != (int) len)
		goto error1;
	if ((ret = sys_portal_close(outportal)) < 0)
		goto error1;

	mqueue_remove_first_slot(mqueueid);

	return (0);

error1:
	sys_portal_close(outportal);
error0:
	return (ret);
}

/*============================================================================*
 * do_create()                                                                *
 *============================================================================*/

/**
 * @brief Handles a create request.
 */
static inline int do_create(struct mqueue_message *msg, struct mqueue_message *response)
{
	int ret;
	struct mqueue_message msg1;

	/* Persist first message. */
	if (!(msg->seq & 1))
	{
		assert(buffer_put(msg->source, msg) == 0);
		return (0);
	}

	/* Get first message. */
	assert(buffer_get(msg->source, &msg1) == 0);
	assert(msg->seq == (msg1.seq | 1));

	ret = mqueue_create(
		msg->source,
		msg->op.create2.name,
		msg1.op.create1.readable,
		msg1.op.create1.writable,
		msg1.op.create1.mode
	);

	response->source = msg->source;
	if (ret >= 0)
	{
		response->op.ret.mqueueid = ret;
		response->opcode = MQUEUE_SUCCESS;
	}
	else
	{
		response->op.ret.status = -ret;
		response->opcode = MQUEUE_FAILURE;
	}

	return (1);
}

/*============================================================================*
 * do_create_excl()                                                           *
 *============================================================================*/

/**
 * @brief Handles an exclusive create request.
 */
static int do_create_excl(struct mqueue_message *msg, struct mqueue_message *response)
{
	int ret;
	struct mqueue_message msg1;

	/* Persist first message. */
	if (!(msg->seq & 1))
	{
		assert(buffer_put(msg->source, msg) == 0);
		return (0);
	}

	/* Get first message. */
	assert(buffer_get(msg->source, &msg1) == 0);
	assert(msg->seq == (msg1.seq | 1));

	ret = mqueue_create_exclusive(
		msg->source,
		msg->op.create2.name,
		msg1.op.create1.readable,
		msg1.op.create1.writable,
		msg1.op.create1.mode
	);

	response->source = msg->source;
	if (ret >= 0)
	{
		response->op.ret.mqueueid = ret;
		response->opcode = MQUEUE_SUCCESS;
	}
	else
	{
		response->op.ret.status = -ret;
		response->opcode = MQUEUE_FAILURE;
	}

	return (1);
}

/*============================================================================*
 * do_open()                                                                  *
 *============================================================================*/

/**
 * @brief Handles an open request.
 */
static int do_open(struct mqueue_message *msg, struct mqueue_message *response)
{
	int ret;
	struct mqueue_message msg1;

	/* Persist first message. */
	if (!(msg->seq & 1))
	{
		assert(buffer_put(msg->source, msg) == 0);
		return (0);
	}

	/* Get first message. */
	assert(buffer_get(msg->source, &msg1) == 0);
	assert(msg->seq == (msg1.seq | 1));

	ret = mqueue_open(
		msg->source,
		msg->op.open2.name,
		msg1.op.open1.readable,
		msg1.op.open1.writable
	);

	response->source = msg->source;
	if (ret >= 0)
	{
		response->op.ret.mqueueid = ret;
		response->opcode = MQUEUE_SUCCESS;
	}
	else
	{
		response->op.ret.status = -ret;
		response->opcode = MQUEUE_FAILURE;
	}

	return (1);
}

/*============================================================================*
 * do_send()                                                                  *
 *============================================================================*/

/**
 * @brief Handles a send request.
 */
static int do_send(struct mqueue_message *msg, struct mqueue_message *response)
{
	int ret;

	/* Send permission?
	 * Block when trying to write to a full queue not yet implemented.
	 */
	if (mqueue_valid_send(msg->source, msg->op.send.mqueueid))
	{
		response->op.ret.status = 0;
		response->opcode = MQUEUE_SUCCESS;

		int outbox;
		assert((outbox = sys_mailbox_open(response->source)) >= 0);
		assert(sys_mailbox_write(outbox, response, sizeof(struct mqueue_message)) == MAILBOX_MSG_SIZE);
		assert(sys_mailbox_close(outbox) == 0);
	}
	else
	{
		response->op.ret.status = -EAGAIN;
		response->opcode = MQUEUE_FAILURE;

		return (1);
	}

	ret = mqueue_send(
		msg->source,
		msg->op.send.mqueueid,
		msg->op.send.len,
		msg->op.send.prio
	);

	response->source = msg->source;
	if (ret == 0)
	{
		response->op.ret.status = 0;
		response->opcode = MQUEUE_SUCCESS;
	}
	else
	{
		response->op.ret.status = -ret;
		response->opcode = MQUEUE_FAILURE;
	}

	return (1);
}

/*============================================================================*
 * do_receive()                                                               *
 *============================================================================*/

/**
 * @brief Handles a receive request.
 */
static int do_receive(struct mqueue_message *request, struct mqueue_message *response)
{
	int ret;

	/* Send permission?
	 * Block when trying to read an empty queue not yet implemented.
	 */
	if (mqueue_valid_receive(request->source, request->op.receive.mqueueid))
	{
		response->op.ret.status = 0;
		response->opcode = MQUEUE_SUCCESS;

		int outbox;
		assert((outbox = sys_mailbox_open(response->source)) >= 0);
		assert(sys_mailbox_write(outbox, response, sizeof(struct mqueue_message)) == MAILBOX_MSG_SIZE);
		assert(sys_mailbox_close(outbox) == 0);
	}
	else
	{
		response->op.ret.status = -EAGAIN;
		response->opcode = MQUEUE_FAILURE;

		return (1);
	}

	ret = mqueue_receive(
		request->source,
		request->op.receive.mqueueid,
		request->op.receive.len,
		&response->op.ret.prio
	);

	response->source = request->source;
	if (ret == 0)
		response->opcode = MQUEUE_SUCCESS;
	else
	{
		response->op.ret.status = -ret;
		response->opcode = MQUEUE_FAILURE;
	}

	return (1);
}

/*============================================================================*
 * do_unlink()                                                                *
 *============================================================================*/

/**
 * @brief Handles an unlink request.
 */
static int do_unlink(struct mqueue_message *msg, struct mqueue_message *response)
{
	int ret;

	ret = mqueue_unlink(msg->source, msg->op.unlink.name);

	response->source = msg->source;
	if (ret >= 0)
	{
		response->op.ret.mqueueid = ret;
		response->opcode = MQUEUE_SUCCESS;
	}
	else
	{
		response->op.ret.status = -ret;
		response->opcode = MQUEUE_FAILURE;
	}

	return (1);
}

/*============================================================================*
 * do_close()                                                                *
 *============================================================================*/

/**
 * @brief Handles an close request.
 */
static int do_close(struct mqueue_message *msg, struct mqueue_message *response)
{
	int ret;

	ret = mqueue_close(msg->source, msg->op.close.mqueueid);

	response->source = msg->source;
	if (ret == 0)
	{
		response->op.ret.status = 0;
		response->opcode = MQUEUE_SUCCESS;
	}
	else
	{
		response->op.ret.status = -ret;
		response->opcode = MQUEUE_FAILURE;
	}

	return (1);
}

/*============================================================================*
 * do_null()                                                                  *
 *============================================================================*/

/**
 * @brief Handles a null request.
 */
static int do_null(struct mqueue_message *msg, struct mqueue_message *response)
{
	mqueue_debug("null request nodenum=%d", msg->source);

	response->opcode = MQUEUE_FAILURE;
	response->op.ret.status = EINVAL;
	response->source = msg->source;

	return (1);
}

/*============================================================================*
 * mqueue_loop()                                                              *
 *============================================================================*/

/**
 * @brief Handles message queue requests.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int mqueue_loop(void)
{
	int shutdown = 0;

	while(!shutdown)
	{
		int reply = 0;
		struct mqueue_message request;
		struct mqueue_message response;

		assert(sys_mailbox_read(inbox, &request, sizeof(struct mqueue_message)) == MAILBOX_MSG_SIZE);

		/* Invalid process ID. */
		if (request.source >= HAL_NR_NOC_NODES)
			continue;

		/* Handle request. */
		switch (request.opcode)
		{
			/* */
			case MQUEUE_CREATE:
				reply = do_create(&request, &response);
				break;

			case MQUEUE_CREATE_EXCL:
				reply = do_create_excl(&request, &response);
				break;

			case MQUEUE_OPEN:
				reply = do_open(&request, &response);
				break;

			case MQUEUE_UNLINK:
				reply = do_unlink(&request, &response);
				break;

			case MQUEUE_CLOSE:
				reply = do_close(&request, &response);
				break;

			case MQUEUE_SEND:
				reply = do_send(&request, &response);
				break;

			case MQUEUE_RECEIVE:
				reply = do_receive(&request, &response);
				break;

			case MQUEUE_EXIT:
				shutdown = 1;
				break;

			default:
				reply = do_null(&request, &response);
				break;
		}

		/* Send reply. */
		if (reply)
		{
			int outbox;
			assert((outbox = sys_mailbox_open(response.source)) >= 0);
			assert(sys_mailbox_write(outbox, &response, sizeof(struct mqueue_message)) == MAILBOX_MSG_SIZE);
			assert(sys_mailbox_close(outbox) == 0);
		}
	}

	return (0);
}

/*============================================================================*
 * mqueue_startup()                                                           *
 *============================================================================*/

/**
 * @brief Initializes the message queue server.
 *
 * @param _inbox    Input mailbox.
 * @param _inportal Input portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int mqueue_startup(int _inbox, int _inportal)
{
	int ret;
	int nodenum;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = sys_get_node_num();

	inbox = _inbox;
	inportal = _inportal;

	mqueue_init();
	buffer_init();

	/* Initialize process table. */
	for (int node = 0; node < HAL_NR_NOC_NODES; node++)
		for (int id = 0; id < MQUEUE_OPEN_MAX; id++)
			mqueue_clear_flags(node, id);

	/* Link name. */
	sprintf(pathname, "/mqueue-server");
	if ((ret = name_link(nodenum, pathname)) < 0)
		return (ret);

	return (0);
}

/*============================================================================*
 * mqueue_shutdown()                                                          *
 *============================================================================*/

/**
 * @brief Shutdowns the message queues server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int mqueue_shutdown(void)
{
	return (0);
}

/*============================================================================*
 * mqueue_server()                                                            *
 *============================================================================*/

/**
 * @brief Handles mqueue requests.
 *
 * @param _inbox    Input mailbox.
 * @param _inportal Input portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int mqueue_server(int _inbox, int _inportal)
{
	int ret;

	printf("[nanvix][mqueue] booting up server\n");

	/* Startup server. */
	if ((ret = mqueue_startup(_inbox, _inportal)) < 0)
		goto error;

	spawner_ack();

	printf("[nanvix][mqueue] server alive\n");

	if ((ret = mqueue_loop()) < 0)
		goto error;

	printf("[nanvix][mqueue] shutting down server\n");

	if ((ret = mqueue_shutdown()) < 0)
		goto error;

	return (0);

error:
	return (ret);
}
