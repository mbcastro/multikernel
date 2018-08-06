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
#include <sys/types.h>
#include <stdarg.h>

#include <nanvix/spawner.h>
#include <nanvix/syscalls.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>

#include "shm.h"

/**
 * @brief Maximum number of shared memory regions.
 */
#define SHM_MAX 128

/**
 * @brief Shared memory region flags.
 */
/**@{*/
#define SHM_USED   (1 << 0) /**< Used?   */
#define SHM_REMOVE (1 << 1) /**< Remove? */
/**@}*/

/**
 * @brief Table of named shared memory regions.
 */
static struct
{
	char name[SHM_NAME_MAX];     /**< Shared memory region name.  */
	int flags;                   /**< Flags.                      */
	int owner;                   /**< ID of owner process.        */
	int refcount;                /**< Number of references.       */
	mode_t mode;                 /**< Access permissions.         */
	int nodes[HAL_NR_NOC_NODES]; /**< Process list.               */
} shmregs[SHM_MAX];

/**
 * @brief Input mailbox for small messages.
 */
static int inbox;

/*============================================================================*
 * shm_debug()                                                                *
 *============================================================================*/

/**
 * @brief Dumps debug information.
 */
static void shm_debug(const char *fmt, ...)
{
#ifndef DEBUG_SHM
	((void) fmt);
#else

	int len;
	va_list args;
	char strbuf[80];

	strcpy(strbuf, "[DEBUG][nanvix][shm] ");
	len = 80 - 2 - strlen(strbuf);
	strncat(strbuf, fmt, len);
	strcat(strbuf, "\n");

	va_start (args, fmt);
	vprintf (strbuf, args);
	va_end (args);

#endif
}

/*============================================================================*
 * shm_name_is_valid()                                                        *
 *============================================================================*/

/**
 * @brief Asserts whether or not a shared memory region name is valid.
 *
 * @param name Target name.
 *
 * @returns One if the target shared memory region name is valid, and
 * zero otherwise.
 */
static inline int shm_name_is_valid(const char *name)
{
	return ((name != NULL) &&
			(strlen(name) < (SHM_NAME_MAX - 1)) &&
			(strcmp(name, ""))
	);
}

/*============================================================================*
 * shm_is_valid()                                                             *
 *============================================================================*/

/**
 * @brief Asserts whether or not a shared memory region ID is valid.
 *
 * @param shmid ID of the target shared memory region.
 *
 * @returns Non-zero if the shared memory region valid, and zero otherwise.
 */
static inline int shm_is_valid(int shmid)
{
	return ((shmid >= 0) && (shmid < SHM_MAX));
}

/*============================================================================*
 * shm_is_used()                                                              *
 *============================================================================*/

/**
 * @brief Asserts whether or not a shared memory region is used.
 *
 * @param shmid ID of the target shared memory region.
 *
 * @returns Non-zero if the shared memory region is marked as used,
 * and zero otherwise.
 */
static inline int shm_is_used(int shmid)
{
	return (shmregs[shmid].flags & SHM_USED);
}

/*============================================================================*
 * shm_is_remove()                                                            *
 *============================================================================*/

/**
 * @brief Asserts whether or not a shared memory region is remove.
 *
 * @param shmid ID of the target shared memory region.
 *
 * @returns Non-zero if the shared memory region is marked to be
 * removed, and zero otherwise.
 */
static inline int shm_is_remove(int shmid)
{
	return (shmregs[shmid].flags & SHM_REMOVE);
}

/*============================================================================*
 * shm_set_used()                                                             *
 *============================================================================*/

/**
 * @brief Sets a shared memory region as used.
 *
 * @param shmid ID of the target shared memory region.
 */
static inline void shm_set_used(int shmid)
{
	shmregs[shmid].flags |= SHM_USED;
}

/*============================================================================*
 * shm_set_remove()                                                           *
 *============================================================================*/

/**
 * @brief Marks a shared memory region to be removed.
 *
 * @param shmid ID of the target shared memory region.
 */
static inline void shm_set_remove(int shmid)
{
	shmregs[shmid].flags |= SHM_REMOVE;
}

/*============================================================================*
 * shm_clear_flags()                                                          *
 *============================================================================*/

/**
 * @brief Clears the flags of a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 */
static inline void shm_clear_flags(int shmid)
{
	shmregs[shmid].flags = 0;
}

/*============================================================================*
 * shm_alloc()                                                                *
 *============================================================================*/

/**
 * @brief Allocates a shared memory region.
 *
 * @return Upon successful completion, the ID of the newly allocated
 * shared memory region is returned. Upon failure, -1 is returned instead.
 */
static int shm_alloc(void)
{
	/* Search for a free shared memory region. */
	for (int i = 0; i < SHM_MAX; i++)
	{
		/* Found. */
		if (!shm_is_used(i))
		{
			shm_set_used(i);
			return (i);
		}
	}

	return (-1);
}

/*============================================================================*
 * shm_free()                                                                 *
 *============================================================================*/

/**
 * @brief Free a shared memory region.
 */
static void shm_free(int shmid)
{
	shm_clear_flags(shmid);
}

/*============================================================================*
 * shm_open()                                                                 *
 *============================================================================*/

/**
 * @brief Opens a shared memory region
 *
 * @param node  ID of opening process.
 * @param name  Name of the targeted shared memory region.
 *
 * @returns Upon successful completion, the shared memory region ID is
 * returned. Upon failure, a negative error code is returned instead.
 */
static int shm_open(int node, const char *name)
{
	int shmid;    /* Shared memory region ID. */
	int refcount; /* Number of process.       */

	shm_debug("open node=%d name=%s", node, name);

	/* Invalid process. */
	if ((node < 0) || (node >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid shm name. */
	if (!shm_name_is_valid(name))
		return (-EINVAL);

	/* Look for shm. */
	for (int i = 0; i < SHM_MAX; i++)
	{
		/* Shared memory region not in use. */
		if (!shm_is_used(i))
			continue;

		/* Found.*/
		if (!strcmp(shmregs[i].name, name))
		{
			shmid = i;
			goto found;
		}
	}

	return (-EINVAL);

found:

	refcount = shmregs[shmid].refcount++;

	/*
	 * Check if we had opened this shm before.
	 * If so, there is nothing else to do.
	 */
	for (int i = 0; i < refcount; i++)
	{
		if (shmregs[shmid].nodes[i] == node)
			return (shmid);
	}

	shmregs[shmid].nodes[refcount] = node;

	return (shmid);
}

/*============================================================================*
 * shm_create()                                                               *
 *============================================================================*/

/**
 * @brief Creates a shared memory region
 *
 * @param owner ID of owner process.
 * @param name  Name of the targeted shm.
 * @param mode  Access permissions.
 *
 * @returns Upon successful completion, the ID of the newly created
 * shared memory region is returned. Upon failure, a negative error
 * code is returned instead.
 */
static int shm_create(int owner, const char *name, mode_t mode)
{
	int shmid;

	shm_debug("create node=%d name=%s mode=%d", owner, name, mode);

	/* Invalid process. */
	if ((owner < 0) || (owner >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid shm name. */
	if (!shm_name_is_valid(name))
		return (-EINVAL);

	/* Look for shm. */
	for (int i = 0; i < SHM_MAX; i++)
	{
		/* Shared memory region not in use. */
		if (!shm_is_used(i))
			continue;

		/* Found.*/
		if (!strcmp(shmregs[i].name, name))
			return (shm_open(owner, name));
	}

	/* Allocate a new shm. */
	if ((shmid = shm_alloc()) < 0)
		return (-EAGAIN);

	/* Initialize shm. */
	shmregs[shmid].refcount = 1;
	shmregs[shmid].owner = owner;
	shmregs[shmid].nodes[0] = owner;
	shmregs[shmid].mode =  mode;
	strcpy(shmregs[shmid].name, name);

	return (shmid);
}

/*============================================================================*
 * shm_create_exclusive()                                                     *
 *============================================================================*/

/**
 * @brief Open a shared memory region with existence check
 *
 * @param owner ID of owner process.
 * @param name  Name of the targeted shared memory region.
 * @param mode  Access permissions.
 *
 * @returns Upon successful completion, the newly created shared
 * memory region ID is returned. Upon failure, a negative error code
 * is returned instead.
 */
static int shm_create_exclusive(int owner, char *name, int mode)
{
	shm_debug("create-excl node=%d name=%s mode=%d", owner, name, mode);

	/* Invalid process. */
	if ((owner < 0) || (owner >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid shm name. */
	if (!shm_name_is_valid(name))
		return (-EINVAL);

	/* Look for shm. */
	for (int i = 0; i < SHM_MAX; i++)
	{
		/* Shared memory region not in use. */
		if (!shm_is_used(i))
			continue;

		/* Found.*/
		if (!strcmp(shmregs[i].name, name))
			return (-EEXIST);
	}

	return (shm_create(owner, name, mode));
}

/*============================================================================*
 * shm_close()                                                                *
 *============================================================================*/

/**
 * @brief Close a shared memory region
 *
 * @param node  ID of opening process.
 * @param shared memory regionid Target shared memory region.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int shm_close(int node, int shmid)
{
	int i;
	int refcount;

	shm_debug("close node=%d shmid=%d", node, shmid);

	/* Invalid process. */
	if ((node < 0) || (node >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid shm ID. */
	if (!shm_is_valid(shmid))
		return (-EINVAL);

	/* Shared memory region not in use. */
	if (!shm_is_used(shmid))
		return (-EINVAL);

	refcount = shmregs[shmid].refcount;

	/*
	 * The process should have opened
	 * the shared memory region before.
	 */
	for (i = 0; i < refcount; i++)
	{
		if (shmregs[shmid].nodes[i] == node)
			goto found;
	}

	return (-EINVAL);

found:

	/* Remove the process from the list. */
	for (int j = i; j < refcount - 1; j++)
		shmregs[shmid].nodes[j] = shmregs[shmid].nodes[j + 1];

	shmregs[shmid].refcount--;

	/* Unlink the shared memory region if no process is using it anymore. */
	if ((shmregs[shmid].refcount == 0) && (shm_is_remove(shmid)))
		shm_free(shmid);

	return (0);
}

/*============================================================================*
 * shm_unlink()                                                               *
 *============================================================================*/

/**
 * @brief Unlink a shared memory region
 *
 * @param node  ID the calling process.
 * @param shmid Target shared memory region.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int shm_unlink(int node, const char *name)
{
	int shmid;

	shm_debug("unlink node=%d name=%s", node, name);

	/* Invalid process ID. */
	if ((node < 0) || (node >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Look for shared memory region. */
	for (shmid = 0; shmid < SHM_MAX; shmid++)
	{
		/* Shared memory region not in use. */
		if (!shm_is_used(shmid))
			continue;

		if (!strcmp(shmregs[shmid].name, name))
			goto found;
	}

	return (-EAGAIN);

found:

	/* Do I own the shared memory region? */
	if (shmregs[shmid].owner != node)
		return (-EPERM);

	/*
	 * We cannot remote the shared memory region now,
	 * so let us just close it and schedule the
	 * operation.
	 **/
	if (shmregs[shmid].refcount > 1)
	{
		shm_set_remove(shmid);
		return (shm_close(node, shmid));
	}

	shmregs[shmid].refcount = 0;
	shm_clear_flags(shmid);

	return (0);
}

/*============================================================================*
 * shm_loop()                                                                 *
 *============================================================================*/

/**
 * @brief Handles shared memory region requests.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int shm_loop(void)
{
	while(1)
	{
		int reply = 0;
		struct shm_message msg;

		assert(sys_mailbox_read(inbox, &msg, sizeof(struct shm_message)) == MAILBOX_MSG_SIZE);

		shm_debug("request received");

		/* Handle shared memory region requests. */
		switch (msg.opcode)
		{
			/* Create a shared memory region. */
			case SHM_CREATE:
			{
				/* Persist first message. */
				if (!(msg.seq & 1))
					assert(buffer_put(msg.source, &msg) == 0);
				/* Parse second message.*/
				else
				{
					struct shm_message msg1;

					/* Get first message. */
					assert(buffer_get(msg.source, &msg1) == 0);
					assert(msg.seq == (msg1.seq | 1));

					msg.op.ret = shm_create(msg.source, msg1.op.create1.name, msg.op.create2.mode);
					msg.opcode = SHM_RETURN;
					reply = 1;
				}
			} break;

			/* Create a shared memory region with existence check. */
			case SHM_CREATE_EXCL:
			{
				/* Persist first message. */
				if (!(msg.seq & 1))
					assert(buffer_put(msg.source, &msg) == 0);
				/* Parse second message.*/
				else
				{
					struct shm_message msg1;

					/* Get first message. */
					assert(buffer_get(msg.source, &msg1) == 0);
					assert(msg.seq == (msg1.seq | 1));

					msg.op.ret = shm_create_exclusive(msg.source, msg1.op.create1.name, msg.op.create2.mode);
					msg.opcode = SHM_RETURN;
					reply = 1;
				}

			} break;

			/* Open a shared memory region. */
			case SHM_OPEN:
			{
				/* Persist first message. */
				if (!(msg.seq & 1))
					assert(buffer_put(msg.source, &msg) == 0);
				/* Parse second message.*/
				else
				{
					struct shm_message msg1;

					/* Get first message. */
					assert(buffer_get(msg.source, &msg1) == 0);
					assert(msg.seq == (msg1.seq | 1));

					msg.op.ret = shm_open(msg.source, msg1.op.create1.name);
					msg.opcode = SHM_RETURN;
					reply = 1;
				}

			} break;

			/* Unlink a shared memory region. */
			case SHM_UNLINK:
				msg.op.ret = shm_unlink(msg.source, msg.op.unlink.name);
				msg.opcode = SHM_RETURN;
				reply = 1;
			break;

			/* Should not happen. */
			default:
				break;
		}

		/* Send reply. */
		if (reply)
		{
			int outbox;
			shm_debug("response sent to %d", msg.source);
			assert((outbox = sys_mailbox_open(msg.source)) >= 0);
			assert(sys_mailbox_write(outbox, &msg, sizeof(struct shm_message)) == MAILBOX_MSG_SIZE);
			assert(sys_mailbox_close(outbox) == 0);
		}
	}

	return (0);
}

/*============================================================================*
 * shm_startup()                                                              *
 *============================================================================*/

/**
 * @brief Initializes the shared memory regions server.
 *
 * @param _inbox Input mailbox.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int shm_startup(int _inbox)
{
	/* Assign input mailbox. */
	inbox = _inbox;

	/* Initialize shmregs table. */
	for (int i = 0; i < SHM_MAX; i++)
	{
		shmregs[i].refcount = 0;
		shm_clear_flags(i);
	}

	buffer_init();

	return (0);
}

/*============================================================================*
 * shm_shutdown()                                                       *
 *============================================================================*/

/**
 * @brief Shutdowns the shared memory regions server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int shm_shutdown(void)
{
	return (0);
}

/*============================================================================*
 * shm_server()                                                         *
 *============================================================================*/

/**
 * @brief Handles remote shm requests.
 *
 * @param _inbox    Input mailbox.
 * @param _inportal Input portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int shm_server(int _inbox, int _inportal)
{
	int ret;

	((void) _inportal);

	printf("[nanvix][shm] booting up server\n");

	if ((ret = shm_startup(_inbox)) < 0)
		goto error;

	printf("[nanvix][shm] server alive\n");

	spawner_ack();

	if ((ret = shm_loop()) < 0)
		goto error;

	printf("[nanvix][shm] shutting down server\n");

	if ((ret = shm_shutdown()) < 0)
		goto error;

	return (0);

error:
	return (ret);
}

