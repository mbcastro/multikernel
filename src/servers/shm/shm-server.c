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

#include <nanvix/spawner.h>
#include <nanvix/syscalls.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>

#include "shm.h"

/**
 * @brief Flags for shared region.
 */
/*@{@*/
#define SHM_WRITE  (1 << 0) /**< Writable? Else read-only */
#define SHM_SHARED (1 << 1) /**< Shared? Else private.    */
#define SHM_MAPPED (1 << 2) /**< Mapped? Else unmapped.   */
/**@}*/

/**
 * @brief Table of processes.
 */
static struct
{
	/**
	 * @brief Number of opened shared memory regions.
	 */
	int nopen;

	/**
	 * Table of opened shared memory regions.
	 */
	struct
	{
		int shmid;
		int flags;
	} oregions[SHM_OPEN_MAX];
} procs[HAL_NR_NOC_NODES];

/**
 * @brief Input mailbox for small messages.
 */
static int inbox;

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
 * shm_is_opened()                                                            *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node has opened a given shared
 * memory region.
 *
 * @param node  Target node.
 * @param shmid ID of target shared memory region.
 *
 * @return If the target node has opened the target shared memory
 * region, one is returned. Otherwise, zero is returned instead.
 */
static int shm_is_opened(int node, int shmid)
{
	int nopen;

	nopen = procs[node].nopen;

	for (int i = 0; i < nopen; i++)
	{
		if (procs[node].oregions[i].shmid == shmid)
			return (i);
	}

	return (-1);
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
	int i;
	int shmid;

	shm_debug("open node=%d name=%s", node, name);

	/* Invalid name. */
	if (!shm_name_is_valid(name))
		return (-EINVAL);

	/* Get shared memory. */
	if ((shmid = shm_get(name)) < 0)
		return (-EINVAL);

	/* Shared memory region shall be removed soon. */
	if (shm_is_remove(shmid))
	{
		shm_put(shmid);
		return (-EAGAIN);
	}

	/* Too many files are opened. */
	if (procs[node].nopen >= SHM_OPEN_MAX)
	{
		shm_put(shmid);
		return (-ENFILE);
	}

	i = procs[node].nopen++;
	procs[node].oregions[i].shmid = shmid;
	procs[node].oregions[i].flags = SHM_WRITE;

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
	int i;     /* Index of opened region.  */
	int shmid; /* Shared memory region ID. */

	shm_debug("create node=%d name=%s mode=%d", owner, name, mode);

	/* Invalid name. */
	if (!shm_name_is_valid(name))
		return (-EINVAL);

	/* Look for shared memory region. */
	if ((shmid = shm_get(name)) >= 0)
	{
		shm_put(shmid);
		return (shm_open(owner, name));
	}

	/* Too many files are opened. */
	if (procs[owner].nopen >= SHM_OPEN_MAX)
		return (-ENFILE);

	/* Allocate a new shm. */
	if ((shmid = shm_alloc()) < 0)
		return (-EAGAIN);

	/* Initialize shared memory region. */
	shm_set_perm(shmid, owner, mode);
	shm_set_name(shmid, name);
	shm_set_base(shmid, 0);
	shm_set_size(shmid, RMEM_SIZE);

	i = procs[owner].nopen++;
	procs[owner].oregions[i].shmid = shmid;
	procs[owner].oregions[i].flags = SHM_WRITE;

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
	int shmid;

	shm_debug("create-excl node=%d name=%s mode=%d", owner, name, mode);

	/* Invalid name. */
	if (!shm_name_is_valid(name))
		return (-EINVAL);

	/* Shared memory region exists. */
	if ((shmid = shm_get(name)) >= 0)
	{
		shm_put(shmid);
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
	int nopen;

	shm_debug("close node=%d shmid=%d", node, shmid);

	/* Shared memory region not in use. */
	if (!shm_is_used(shmid))
		return (-EINVAL);

	/*
	 * The process should have opened
	 * the shared memory region before.
	 */
	if ((i = shm_is_opened(node, shmid)) < 0)
		return (-EACCES);

	/* Remove the shared region from the list. */
	nopen = --procs[node].nopen;
	for (int j = i; j < nopen; j++)
	{
		procs[node].oregions[j].shmid = procs[node].oregions[j + 1].shmid;
		procs[node].oregions[j].flags = procs[node].oregions[j + 1].flags;
	}

	shm_put(shmid);

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

	/* Shared memory region does not exist. */
	if ((shmid = shm_get(name)) < 0)
		return (-EINVAL);
	shm_put(shmid);

	/* Do I own the shared memory region? */
	if (!shm_is_owner(shmid, node))
		return (-EPERM);

	shm_set_remove(shmid);
	return (shm_close(node, shmid));
}

/*============================================================================*
 * shm_unmap()                                                                *
 *============================================================================*/

/**
 * @brief Maps a shared memory region.
 *
 * @param node     ID of the calling node.
 * @param shmid    ID of the target shared memory region.
 * @param size     Size of mapping.
 * @param writable Writable mapping? Else read-only.
 * @param shared   Shared mapping? Else private.
 * @param off      Offset within shared memory region.
 * @param mapblk   Place which the mapping address should be stored.
 *
 * @returns Upon successful completion, zero is returned and the
 * mapping address is stored into @p mapblk. Upon failure, a negative
 * error code is returned instead.
 */
static int shm_map(
	int node,
	int shmid,
	size_t size,
	int writable,
	int shared,
	off_t off,
	uint64_t *mapblk)
{
	int i;

	shm_debug("map node=%d name=%d", node, shmid);

	/* Shared memory region not in use. */
	if (!shm_is_used(shmid))
		return (-EINVAL);

	/*
	 * The process should have opened
	 * the shared memory region before.
	 */
	if ((i = shm_is_opened(node, shmid)) < 0)
		return (-EACCES);

	/* Invalid size. */
	if (size > shm_get_size(shmid))
		return (-ENOMEM);

	/* Invalid offset. */
	if (off > ((off_t) shm_get_size(shmid)))
		return (-ENXIO);

	/* Invalid range. */
	if ((off + size) > shm_get_size(shmid))
		return (-ENXIO);

	/* Cannot write. */
	if (writable && ((!procs[node].oregions[i].flags & SHM_WRITE)))
		return (-EACCES);

	/* Map. */
	if (!(procs[node].oregions[i].flags & SHM_MAPPED))
	{
		procs[node].oregions[i].flags |= SHM_MAPPED;
		procs[node].oregions[i].flags |= (shared) ? SHM_SHARED : 0;
	}

	*mapblk = shm_get_base(shmid) + off;

	return (0);
}

/*============================================================================*
 * shm_unmap()                                                                *
 *============================================================================*/

/**
 * @brief Unmaps a shared memory region.
 *
 * @param node     ID of the calling node.
 * @param shmid    ID of the target shared memory region.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int shm_unmap(int node, int shmid)
{
	int i;

	shm_debug("unmap node=%d name=%d", node, shmid);

	/* Shared memory region not in use. */
	if (!shm_is_used(shmid))
		return (-EINVAL);

	/*
	 * The process should have opened
	 * the shared memory region before.
	 */
	if ((i = shm_is_opened(node, shmid)) < 0)
		return (-EACCES);

	/* Not mapped. */
	if (!(procs[node].oregions[i].flags & SHM_MAPPED))
		return (-EINVAL);

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

		/* Invalid process ID. */
		if (msg.source >= HAL_NR_NOC_NODES)
			continue;

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

					msg.op.ret.status = shm_create(msg.source, msg1.op.create1.name, msg.op.create2.mode);
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

					msg.op.ret.status = shm_create_exclusive(msg.source, msg1.op.create1.name, msg.op.create2.mode);
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

					msg.op.ret.status = shm_open(msg.source, msg1.op.create1.name);
					msg.opcode = SHM_RETURN;
					reply = 1;
				}

			} break;

			/* Unlink a shared memory region. */
			case SHM_UNLINK:
				msg.op.ret.status = shm_unlink(msg.source, msg.op.unlink.name);
				msg.opcode = SHM_RETURN;
				reply = 1;
			break;

			/* Map a shared memory region. */
			case SHM_MAP:
			{
				int ret;
				uint64_t mapblk;

				ret = shm_map(
					msg.source,
					msg.op.map.shmid,
					msg.op.map.size,
					msg.op.map.writable,
					msg.op.map.shared,
					msg.op.map.off,
					&mapblk
				);
				if (ret == 0)
				{
					msg.opcode = SHM_RETURN;
					msg.op.ret.mapblk = mapblk;
				}
				else
				{
					msg.opcode = SHM_FAILED;
					msg.op.ret.status = ret;
				}
				reply = 1;
			} break;

			/* Unmap a shared memory region. */
			case SHM_UNMAP:
			{
				msg.op.ret.status = shm_unmap(msg.source, msg.op.unmap.shmid);
				msg.opcode = SHM_RETURN;
				reply = 1;
			} break;

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

	shm_init();
	buffer_init();

	/* Initialize process table. */
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
		procs[i].nopen = 0;


	return (0);
}

/*============================================================================*
 * shm_shutdown()                                                             *
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
 * shm_server()                                                               *
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

