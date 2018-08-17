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

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <nanvix/spawner.h>
#include <nanvix/klib.h>
#include <nanvix/syscalls.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>
#include <nanvix/semaphore.h>

#ifdef DEBUG_SEMAPHORE
	#define semaphore_debug(fmt, ...) debug("semaphore", fmt, __VA_ARGS__)
#else
	#define semaphore_debug(fmt, ...) { }
#endif

/**
 * @brief Semaphore flags.
 */
/**@{*/
#define SEMAPHORE_USED   (1 << 0) /**< Used?   */
#define SEMAPHORE_REMOVE (1 << 1) /**< Remove? */
/**@}*/

/**
 * @brief Table of named semaphores.
 */
static struct
{
	char name[NANVIX_SEM_NAME_MAX]; /**< Semaphore name.       */
	int flags;                      /**< Flags.                */
	int owner;                      /**< ID of owner process.  */
	int count;                      /**< Semaphore count.      */
	int refcount;                   /**< Number of references. */
	mode_t mode;                    /**< Access permissions.   */
	int nodes[HAL_NR_NOC_NODES];    /**< Process list.         */
	int head;                       /**< First element.        */
	int tail;                       /**< Last element.         */
	int queue[HAL_NR_NOC_NODES];    /**< Waiting list.         */
} semaphores[SEM_MAX];

/**
 * @brief Buffer of messages.
 */
static struct
{
	int valid;              /**< Valid slot?     */
	struct sem_message msg; /**< Bufferd message. */
} buffer[HAL_NR_NOC_NODES];

/**
 * @brief Node number.
 */
static int nodenum;

/**
 * @brief Input mailbox for small messages.
 */
static int inbox;

/*============================================================================*
 * semaphore_name_is_valid()                                                  *
 *============================================================================*/

/**
 * @brief Asserts whether or not a semaphore name is valid.
 *
 * @param name Target name.
 *
 * @returns One if the target semaphore name is valid, and zero
 * otherwise.
 */
static int semaphore_name_is_valid(const char *name)
{
	return ((name != NULL) &&
			(strlen(name) < (NANVIX_SEM_NAME_MAX - 1)) &&
			(strcmp(name, ""))
	);
}

/*============================================================================*
 * semaphore_is_valid()                                                       *
 *============================================================================*/

/**
 * @brief Asserts whether or not a semaphore Id is valid.
 *
 * @param semid ID of the target named semaphore.
 *
 * @returns Non-zero if the semaphore valid, and zero otherwise.
 */
static int semaphore_is_valid(int semid)
{
	return ((semid >= 0) && (semid < SEM_MAX));
}

/*============================================================================*
 * semaphore_is_used()                                                        *
 *============================================================================*/

/**
 * @brief Asserts whether or not a named semaphore is used.
 *
 * @param semid ID of the target named semaphore.
 *
 * @returns Non-zero if the semaphore is marked as used, and zero
 * otherwise.
 */
static inline int semaphore_is_used(int semid)
{
	return (semaphores[semid].flags & SEMAPHORE_USED);
}

/*============================================================================*
 * semaphore_is_remove()                                                      *
 *============================================================================*/

/**
 * @brief Asserts whether or not a named semaphore is remove.
 *
 * @param semid ID of the target named semaphore.
 *
 * @returns Non-zero if the semaphore is marked to be removed, and
 * zero otherwise.
 */
static inline int semaphore_is_remove(int semid)
{
	return (semaphores[semid].flags & SEMAPHORE_REMOVE);
}

/*============================================================================*
 * semaphore_set_used()                                                        *
 *============================================================================*/

/**
 * @brief Sets a named semaphore as used.
 *
 * @param semid ID of the target named semaphore.
 */
static inline void semaphore_set_used(int semid)
{
	semaphores[semid].flags |= SEMAPHORE_USED;
}

/*============================================================================*
 * semaphore_set_remove()                                                     *
 *============================================================================*/

/**
 * @brief Marks a named semaphore to be removed.
 *
 * @param semid ID of the target named semaphore.
 */
static inline void semaphore_set_remove(int semid)
{
	semaphores[semid].flags |= SEMAPHORE_REMOVE;
}

/*============================================================================*
 * semaphore_clear_flags()                                                    *
 *============================================================================*/

/**
 * @brief Clears the flags of a semaphore.
 *
 * @param semid ID of the target named semaphore.
 */
static inline void semaphore_clear_flags(int semid)
{
	semaphores[semid].flags = 0;
}

/*============================================================================*
 * semaphore_alloc()                                                          *
 *============================================================================*/

/**
 * @brief Allocates a named semaphore.
 *
 * @return Upon successful completion, the ID of the newly allocated
 * semaphore is returned. Upon failure, a negative error code is
 * returned instead.
 */
static int semaphore_alloc(void)
{
	/* Search for a free semaphore. */
	for (int i = 0; i < SEM_MAX; i++)
	{
		/* Found. */
		if (!semaphore_is_used(i))
		{
			semaphore_set_used(i);
			return (i);
		}
	}

	return (-ENOENT);
}

/*============================================================================*
 * semaphore_free()                                                           *
 *============================================================================*/

/**
 * @brief Free a named semaphore.
 */
static void semaphore_free(int semid)
{
	semaphore_clear_flags(semid);
}

/*============================================================================*
 * semaphore_put_message()                                                    *
 *============================================================================*/

/**
 * @brief Puts a message in the messages buffer.
 *
 * @param msg Target message
 *
 * @return Upon successful completion zero is returned.  Upon failure,
 * a negative error code is returned instead.
 */
static int semaphore_put_message(struct sem_message *msg)
{
	int i;

	/* Invalid message. */
	if (msg->source >= HAL_NR_NOC_NODES)
		return (-EAGAIN);

	i = msg->source;

	buffer[i].valid = 1;
	memcpy(&buffer[i].msg, msg, sizeof(struct sem_message));

	return (0);
}

/*============================================================================*
 * semaphore_get_message()                                                    *
 *============================================================================*/

/**
 * @brief Gets a message from the message buffer.
 *
 * @param message Address where the message will be stored.
 * @param i       Buffer slot.
 *
 * @return Upon successful completion zero is returned.
 * Upon failure, a negative error code is returned instead.
 */
static int semaphore_get_message(struct sem_message *msg, int i)
{
	/* Invalid slot. */
	if ((i < 0) || (i >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Bad slot. */
	if (!buffer[i].valid)
		return (-ENOENT);

	/* Get message. */
	buffer[i].valid = 0;
	memcpy(msg, &buffer[i].msg, sizeof(struct sem_message));

	return (0);
}

/*============================================================================*
 * semaphore_enqueue()                                                        *
 *============================================================================*/

/**
 * @brief Enqueue a process name.
 *
 * @param node   ID of the target process.
 * @param semid ID of the targeted semaphore.
 *
 * @return Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
void semaphore_enqueue(int node, int semid)
{
	int tail;

	tail = semaphores[semid].tail;
	semaphores[semid].queue[tail] = node;
	semaphores[semid].tail = (tail + 1)%HAL_NR_NOC_NODES;
}

/*============================================================================*
 * semaphore_dequeue()                                                        *
 *============================================================================*/

/**
 * @brief Dequeue a process name.
 *
 * @param node   ID of the target process.
 * @param semid ID of the targeted semaphore.
 *
 * @returns The ID of the first process in the sleeping queue.
 */
int semaphore_dequeue(int semid)
{
	int node;
	int head;

	head = semaphores[semid].head;
	node = semaphores[semid].queue[head];
	semaphores[semid].head = (head + 1)%HAL_NR_NOC_NODES;

	return (node);
}

/*============================================================================*
 * semaphore_open()                                                           *
 *============================================================================*/

/**
 * @brief Opens a named semaphore
 *
 * @param node  ID of opening process.
 * @param name  Name of the targeted semaphore.
 *
 * @returns Upon successful completion, the semaphore Id is
 * returned. Upon failure, a negative error code is returned instead.
 */
static int semaphore_open(int node, char *name)
{
	int semid;  /* Semaphore Id.      */
	int refcount; /* Number of process. */

	semaphore_debug("open nodenum=%d name=%s",
		node,
		name
	);

	/* Invalid process. */
	if ((node < 0) || (node >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid semaphore name. */
	if (!semaphore_name_is_valid(name))
		return (-EINVAL);

	/* Look for semaphore. */
	for (int i = 0; i < SEM_MAX; i++)
	{
		/* Semaphore not in use. */
		if (!semaphore_is_used(i))
			continue;

		/* Found.*/
		if (!strcmp(semaphores[i].name, name))
		{
			semid = i;
			goto found;
		}
	}

	return (-EINVAL);

found:

	refcount = semaphores[semid].refcount;

	/*
	 * Check if we had opened this semaphore before.
	 * If so, there is nothing else to do.
	 */
	for (int i = 0; i < refcount; i++)
	{
		if (semaphores[semid].nodes[i] == node)
			return (semid);
	}

	semaphores[semid].nodes[refcount] = node;
	semaphores[semid].refcount++;

	return (semid);
}

/*============================================================================*
 * semaphore_create()                                                         *
 *============================================================================*/

/**
 * @brief Creates a named semaphore
 *
 * @param owner ID of owner process.
 * @param name  Name of the targeted semaphore.
 * @param mode  Access permissions.
 * @param value	Semaphore count value.
 *
 * @returns Upon successful completion, the ID of the newly created
 * semaphore is returned. Upon failure, a negative error code is
 * returned instead.
 */
static int semaphore_create(int owner, char *name, mode_t mode, int value)
{
	int semid;

	semaphore_debug("create nodenum=%d name=%s mode=%d value=%d",
		owner,
		name,
		mode,
		value
	);

	/* Invalid process. */
	if ((owner < 0) || (owner >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid semaphore name. */
	if (!semaphore_name_is_valid(name))
		return (-EINVAL);

	/* Invalid value. */
	if (value > SEM_VALUE_MAX)
		return (-EINVAL);

	/* Look for semaphore. */
	for (int i = 0; i < SEM_MAX; i++)
	{
		/* Semaphore not in use. */
		if (!semaphore_is_used(i))
			continue;

		/* Found.*/
		if (!strcmp(semaphores[i].name, name))
			return (semaphore_open(owner, name));
	}

	/* Allocate a new semaphore. */
	if ((semid = semaphore_alloc()) < 0)
		return (-ENOENT);

	/* Initialize semaphore. */
	semaphores[semid].count = value;
	semaphores[semid].head = 0;
	semaphores[semid].tail = 0;
	semaphores[semid].refcount = 1;
	semaphores[semid].owner = owner;
	semaphores[semid].nodes[0] = owner;
	semaphores[semid].mode =  mode;
	strcpy(semaphores[semid].name, name);

	return (semid);
}

/*============================================================================*
 * semaphore_create_exclusive()                                               *
 *============================================================================*/

/**
 * @brief Open a semaphore with existence check
 *
 * @param owner ID of owner process.
 * @param name  Name of the targeted semaphore.
 * @param mode  Access permissions.
 * @param value	Semaphore count value.
 *
 * @returns Upon successful completion, the newly created semaphore Id is
 * returned. Upon failure, a negative error code is returned instead.
 */
static int semaphore_create_exclusive(int owner, char *name, int mode, int value)
{
	/* Invalid process. */
	if ((owner < 0) || (owner >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid semaphore name. */
	if (!semaphore_name_is_valid(name))
		return (-EINVAL);

	/* Invalid value. */
	if (value > SEM_VALUE_MAX)
		return (-EINVAL);

	/* Look for semaphore. */
	for (int i = 0; i < SEM_MAX; i++)
	{
		/* Semaphore not in use. */
		if (!semaphore_is_used(i))
			continue;

		/* Found.*/
		if (!strcmp(semaphores[i].name, name))
			return (-EEXIST);
	}

	return (semaphore_create(owner, name, mode, value));
}

/*============================================================================*
 * semaphore_close()                                                          *
 *============================================================================*/

/**
 * @brief Close a semaphore
 *
 * @param node  ID of opening process.
 * @param semid Target semaphore.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int semaphore_close(int node, int semid)
{
	int i;
	int refcount;

	semaphore_debug("close nodenum=%d semid=%d",
		node,
		semid
	);

	/* Invalid process. */
	if ((node < 0) || (node >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid semaphore ID. */
	if (!semaphore_is_valid(semid))
		return (-EINVAL);

	/* Semaphore not in use. */
	if (!semaphore_is_used(semid))
		return (-EINVAL);

	refcount = semaphores[semid].refcount;

	/*
	 * The process should have opened
	 * the semaphore before.
	 */
	for (i = 0; i < refcount; i++)
	{
		if (semaphores[semid].nodes[i] == node)
			goto found;
	}

	return (-EINVAL);

found:

	/* Remove the process from the list. */
	for (int j = i; j < refcount - 1; j++)
		semaphores[semid].nodes[j] = semaphores[semid].nodes[j + 1];

	semaphores[semid].refcount--;

	/* Unlink the semaphore if no process is using it anymore. */
	if ((semaphores[semid].refcount == 0) && (semaphore_is_remove(semid)))
		semaphore_free(semid);

	return (0);
}

/*============================================================================*
 * semaphore_unlink()                                                         *
 *============================================================================*/

/**
 * @brief Unlink a semaphore
 *
 * @param node  ID the calling process.
 * @param semid Target semaphore.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int semaphore_unlink(int node, const char *name)
{
	int semid;

	semaphore_debug("unlink nodenum=%d name=%s",
		node,
		name
	);

	/* Invalid process ID. */
	if ((node < 0) || (node >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Look for semaphore. */
	for (semid = 0; semid < SEM_MAX; semid++)
	{
		/* Semaphore not in use. */
		if (!semaphore_is_used(semid))
			continue;

		if (!strcmp(semaphores[semid].name, name))
			goto found;
	}

	return (-EAGAIN);

found:

	/* Do I own the semaphore? */
	if (semaphores[semid].owner != node)
		return (-EPERM);

	/*
	 * We cannot remote the semaphore now,
	 * so let us just close it and schedule the
	 * operation.
	 **/
	if (semaphores[semid].refcount > 1)
	{
		semaphore_set_remove(semid);
		return (semaphore_close(node, semid));
	}

	semaphores[semid].refcount = 0;
	semaphore_clear_flags(semid);

	return (0);
}

/*============================================================================*
 * semaphore_wait()                                                           *
 *============================================================================*/
 
/**
 * @brief Waits on a named semaphore
 * 
 * @param node  ID the calling process.
 * @param semid Target semaphore.
 * 
 * @returns Upon successful completion, either zero is returned, thus
 * signalling that the process has acquired the semaphore lock, or
 * zero, which means that the process has blocked. Upon failure, a
 * negative error code is returned instead.
 */ 
static int semaphore_wait(int node, int semid)
{
	int i;
	int refcount;

	semaphore_debug("wait nodenum=%d semid=%d",
		node,
		semid
	);

	/* Invalid process. */
	if ((node < 0) || (node >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid semaphore ID. */
	if (!semaphore_is_valid(semid))
		return (-EINVAL);

	/* Semaphore not in use. */
	if (!semaphore_is_used(semid))
		return (-EINVAL);

	refcount = semaphores[semid].refcount;

	/*
	 * The process should have opened
	 * the semaphore before.
	 */
	for (i = 0; i < refcount; i++)
	{
		if (semaphores[semid].nodes[i] == node)
			goto found;
	}

	return (-EINVAL);

found:

	/* Nothing to wait for. */
	if (semaphores[semid].count > 0)
	{
		semaphores[semid].count--;
		return (0);
	}

	semaphore_enqueue(node, semid);
	return (1);
}

/*============================================================================*
 * semaphore_post()                                                           *
 *============================================================================*/
 
/**
 * @brief Posts on a named semaphore
 * 
 * @param node  ID the calling process.
 * @param semid Target semaphore.
 * 
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */ 
static int semaphore_post(int node, int semid)
{
	int i;
	int outbox;
	int remote;
	int refcount;
	struct sem_message msg;

	semaphore_debug("post nodenum=%d semid=%d",
		node,
		semid
	);

	/* Invalid process. */
	if ((node < 0) || (node >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid semaphore ID. */
	if (!semaphore_is_valid(semid))
		return (-EINVAL);

	/* Semaphore not in use. */
	if (!semaphore_is_used(semid))
		return (-EINVAL);

	refcount = semaphores[semid].refcount;

	/*
	 * The process should have opened
	 * the semaphore before.
	 */
	for (i = 0; i < refcount; i++)
	{
		if (semaphores[semid].nodes[i] == node)
			goto found;
	}

	return (-EINVAL);

found:

	/* Empty sleeping queue. */
	if (semaphores[semid].head == semaphores[semid].tail)
	{
		semaphores[semid].count++;
		return (0);
	}

	/* Send wake up signal. */
	remote = semaphore_dequeue(semid);
	msg.opcode = SEM_RETURN;
	msg.op.ret = 0;
	assert((outbox = sys_mailbox_open(remote)) >= 0);
	assert(sys_mailbox_write(outbox, &msg, sizeof(struct sem_message)) == MAILBOX_MSG_SIZE);
	assert(sys_mailbox_close(outbox) == 0);

	return (0);
}

/*============================================================================*
 * semaphore_loop()                                                           *
 *============================================================================*/

/**
 * @brief Handles named semaphore requests.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int semaphore_loop(void)
{
	while(1)
	{
		int send_response = 0;  /* Reply?             */
		struct sem_message msg; /* Semaphore message. */

		assert(sys_mailbox_read(inbox, &msg, sizeof(struct sem_message)) == MAILBOX_MSG_SIZE);

		/* Handle semaphore requests. */
		switch (msg.opcode)
		{
			/* Create a named semaphore. */
			case SEM_CREATE:
			{
				/* Persist first message. */
				if (!(msg.seq & 1))
					assert(semaphore_put_message(&msg) == 0);
				/* Parse second message.*/
				else
				{
					struct sem_message msg1;

					/* Get first message. */
					assert(semaphore_get_message(&msg1, msg.source) == 0);
					assert(msg.seq == (msg1.seq | 1));

					msg.op.ret = semaphore_create(msg.source, msg.op.create2.name, msg1.op.create1.mode, msg1.op.create1.value);
					msg.opcode = SEM_RETURN;
					send_response = 1;
				}
			} break;

			/* Create a semaphore with existence check. */
			case SEM_CREATE_EXCL:
			{
				/* Persist first message. */
				if (!(msg.seq & 1))
					assert(semaphore_put_message(&msg) == 0);
				/* Parse second message.*/
				else
				{
					struct sem_message msg1;

					/* Get first message. */
					assert(semaphore_get_message(&msg1, msg.source) == 0);
					assert(msg.seq == (msg1.seq | 1));

					msg.op.ret = semaphore_create_exclusive(msg.source, msg.op.create2.name, msg1.op.create1.mode, msg1.op.create1.value);
					msg.opcode = SEM_RETURN;
					send_response = 1;
				}

			} break;

			/* Open a semaphore. */
			case SEM_OPEN:
				msg.op.ret = semaphore_open(msg.source, msg.op.open.name);
				msg.opcode = SEM_RETURN;
				send_response = 1;
			break;

			/* Close a semaphore. */
			case SEM_CLOSE:
				msg.op.ret = semaphore_close(msg.source, msg.op.close.semid);
				msg.opcode = SEM_RETURN;
				send_response = 1;
			break;

			/* Unlink a semaphore. */
			case SEM_UNLINK:
				msg.op.ret = semaphore_unlink(msg.source, msg.op.unlink.name);
				msg.opcode = SEM_RETURN;
				send_response = 1;
			break;

			/* Wait a semaphore. */
			case SEM_WAIT:
				msg.op.ret = semaphore_wait(msg.source, msg.op.close.semid);
				msg.opcode = (msg.op.ret == 1) ? SEM_WAIT: SEM_RETURN;
				send_response = 1;
			break;

			/* Post a semaphore. */
			case SEM_POST:
				msg.op.ret = semaphore_post(msg.source, msg.op.close.semid);
				msg.opcode = SEM_RETURN;
				send_response = 1;
			break;

			/* Should not happen. */
			default:
				break;
		}

		/* Send response. */
		if (send_response)
		{
			int outbox;
			assert((outbox = sys_mailbox_open(msg.source)) >= 0);
			assert(sys_mailbox_write(outbox, &msg, sizeof(struct sem_message)) == MAILBOX_MSG_SIZE);
			assert(sys_mailbox_close(outbox) == 0);
		}
	}

	return (0);
}

/*============================================================================*
 * semaphore_startup()                                                        *
 *============================================================================*/

/**
 * @brief Initializes the named semaphores server.
 *
 * @param _inbox Input mailbox.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int semaphore_startup(int _inbox)
{
	int ret;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = sys_get_node_num();

	/* Assign input mailbox. */
	inbox = _inbox;

	/* Link name. */
	sprintf(pathname, "/sem-server");
	if ((ret = name_link(nodenum, pathname)) < 0)
		return (ret);

	/* Initialize semaphores table. */
	for (int i = 0; i < SEM_MAX; i++)
	{
		semaphores[i].refcount = 0;
		semaphores[i].head = 0;
		semaphores[i].tail = 0;
		semaphore_clear_flags(i);
	}

	/* Initialize message buffer. */
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
		buffer[i].valid = 0;

	return (0);
}

/*============================================================================*
 * semaphore_shutdown()                                                       *
 *============================================================================*/

/**
 * @brief Shutdowns the named semaphores server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int semaphore_shutdown(void)
{
	return (0);
}

/*============================================================================*
 * semaphore_server()                                                         *
 *============================================================================*/

/**
 * @brief Handles remote semaphore requests.
 *
 * @param _inbox    Input mailbox.
 * @param _inportal Input portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int semaphore_server(int _inbox, int _inportal)
{
	int ret;

	((void) _inportal);

	printf("[nanvix][semaphore] booting up server\n");

	if ((ret = semaphore_startup(_inbox)) < 0)
		goto error;

	printf("[nanvix][semaphore] server alive\n");

	spawner_ack();

	if ((ret = semaphore_loop()) < 0)
		goto error;

	printf("[nanvix][semaphore] shutting down server\n");

	if ((ret = semaphore_shutdown()) < 0)
		goto error;

	return (0);

error:
	return (ret);
}

