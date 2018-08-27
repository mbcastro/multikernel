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

/* Implemented but not used. */
static int shm_is_shared(int, int) __attribute__((unused));

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
 * shm_may_write()                                                            *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node may write on a opened shared
 * memory region.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened shared memory region.
 *
 * @returns One if the target node may write into the target opened
 * shared memory region, and zero otherwise.
 */
static inline int shm_may_write(int node, int id)
{
	return (procs[node].oregions[id].flags & SHM_WRITE);
}

/*============================================================================*
 * shm_is_shared()                                                            *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node is sharing a target opened
 * shared memory region.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened shared memory region.
 *
 * @returns One if the target node is sharing the target opened
 * shared memory region, and zero otherwise.
 */
static int shm_is_shared(int node, int id)
{
	return (procs[node].oregions[id].flags & SHM_SHARED);
}

/*============================================================================*
 * shm_has_mapped()                                                           *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node has mapped a target opened
 * shared memory region.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened mapped memory region.
 *
 * @returns One if the target node has mapped the target opened mapped
 * memory region, and zero otherwise.
 */
static inline int shm_has_mapped(int node, int id)
{
	return (procs[node].oregions[id].flags & SHM_MAPPED);
}

/*============================================================================*
 * shm_clear_flags()                                                          *
 *============================================================================*/

/**
 * @brief Clears the flags of a opened shared memory region.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened shared memory region.
 */
static inline void shm_clear_flags(int node, int id)
{
	procs[node].oregions[id].flags = 0;
}


/*============================================================================*
 * shm_set_writable()                                                         *
 *============================================================================*/

/**
 * @brief Sets a target opened shared memory region as writable.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened shared memory region.
 */
static inline void shm_set_writable(int node, int id)
{
	procs[node].oregions[id].flags |= SHM_WRITE;
}

/*============================================================================*
 * shm_set_mapped()                                                           *
 *============================================================================*/

/**
 * @brief Sets a target opened shared memory region as mapped.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened shared memory region.
 */
static inline void shm_set_mapped(int node, int id)
{
	procs[node].oregions[id].flags |= SHM_MAPPED;
}

/*============================================================================*
 * shm_set_shared()                                                           *
 *============================================================================*/

/**
 * @brief Sets a target opened shared memory region as shared.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened shared memory region.
 */
static inline void shm_set_shared(int node, int id)
{
	procs[node].oregions[id].flags |= SHM_SHARED;
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
 * shm_has_opened()                                                           *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node has opened a given shared
 * memory region.
 *
 * @param node  Target node.
 * @param shmid ID of target shared memory region.
 *
 * @return If the target node has opened the target shared memory
 * region, its index in the table of opened shared memory regions is
 * returned. Otherwise, -1 is returned instead.
 */
static int shm_has_opened(int node, int shmid)
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
 * shm_has_mapped()                                                           *
 *============================================================================*/

/**
 * @brief Asserts whether or not a shared memory region is mapped by a
 * node.
 *
 * @param shmid ID of target shared memory region.
 *
 * @returns One if the target node has mapped the target opened mapped
 * memory region, and zero otherwise.
 */
static inline int shm_is_mapped(int shmid)
{
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
	{
		int nopen;

		nopen = procs[i].nopen;

		for (int j = 0; j < nopen; j++)
		{
			if (procs[i].oregions[j].shmid == shmid)
			{
				if (shm_has_opened(i, j))
					return (1);
			}
		}
	}

	return (0);
}

/*============================================================================*
 * shm_open()                                                                 *
 *============================================================================*/

/**
 * @brief Opens a shared memory region
 *
 * @param node     ID of opening process.
 * @param name     Name of the targeted shared memory region.
 * @param writable Writable? Else read-only.
 * @param truncate Truncate? Else not.
 *
 * @returns Upon successful completion, the shared memory region ID is
 * returned. Upon failure, a negative error code is returned instead.
 */
static int shm_open(int node, const char *name, int writable, int truncate)
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

	/* Truncate. */
	if (truncate)
	{
		/* Cannot write. */
		if (!writable)
			return (-EINVAL);

		/* Already mapped. */
		if (shm_is_mapped(shmid))
			return (-EBUSY);

		shm_set_size(shmid, 0);
	}

	i = procs[node].nopen++;
	procs[node].oregions[i].shmid = shmid;
	shm_clear_flags(node, i);
	if (writable)
		shm_set_writable(node, i);

	return (shmid);
}

/*============================================================================*
 * shm_create()                                                               *
 *============================================================================*/

/**
 * @brief Creates a shared memory region
 *
 * @param owner    ID of owner process.
 * @param name     Name of the targeted shm.
 * @param writable Writable? Else read-only.
 * @param mode     Access permissions.
 *
 * @returns Upon successful completion, the ID of the newly created
 * shared memory region is returned. Upon failure, a negative error
 * code is returned instead.
 */
static int shm_create(int owner, const char *name, int writable, mode_t mode)
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
		return (shm_open(owner, name, writable, 0));
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
	shm_set_size(shmid, 0);

	i = procs[owner].nopen++;
	procs[owner].oregions[i].shmid = shmid;
	shm_clear_flags(owner, i);
	if (writable)
		shm_set_writable(owner, i);

	return (shmid);
}

/*============================================================================*
 * shm_create_exclusive()                                                     *
 *============================================================================*/

/**
 * @brief Open a shared memory region with existence check
 *
 * @param owner    ID of owner process.
 * @param name     Name of the targeted shared memory region.
 * @param writable Writable? Else read-only.
 * @param mode     Access permissions.
 *
 * @returns Upon successful completion, the newly created shared
 * memory region ID is returned. Upon failure, a negative error code
 * is returned instead.
 */
static int shm_create_exclusive(int owner, char *name, int writable, mode_t mode)
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

	return (shm_create(owner, name, mode, writable));
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
	if ((i = shm_has_opened(node, shmid)) < 0)
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
 * shm_truncate()                                                             *
 *============================================================================*/

/**
 * @brief Truncates a shared memory region to a specified size.
 *
 * @param node   ID of opening process.
 * @param shmid  ID of the target shared memory region.
 * @param length Shared memory region size (in bytes).
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, -1 is returned instead, and errno is set to indicate the
 * error.
 */
static int shm_truncate(int node, int shmid, size_t size)
{
	int i;

	shm_debug("truncate node=%d shmid=%d size=%d", node, shmid, size);

	/* Not enought memory. */
	if (size > RMEM_SIZE)
		return (-ENOMEM);

	/* Shared memory region not in use. */
	if (!shm_is_used(shmid))
		return (-EINVAL);

	/*
	 * The process should have opened
	 * the shared memory region before.
	 */
	if ((i = shm_has_opened(node, shmid)) < 0)
		return (-EACCES);

	/* Cannot write. */
	if (!shm_may_write(node, i))
		return (-EINVAL);

	/* Already mapped. */
	if (shm_is_mapped(shmid))
		return (-EBUSY);

	shm_set_size(shmid, size);

	return (0);
}

/*============================================================================*
 * shm_map()                                                                  *
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
	if ((i = shm_has_opened(node, shmid)) < 0)
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
	if (writable && (!shm_may_write(node, i)))
		return (-EACCES);

	/* Map. */
	if (!shm_has_mapped(node, i))
	{
		shm_set_mapped(node, i);
		if (shared)
			shm_set_shared(node, i);
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
	if ((i = shm_has_opened(node, shmid)) < 0)
		return (-EACCES);

	/* Not mapped. */
	if (!shm_has_mapped(node, i))
		return (-EINVAL);

	return (0);
}

/*============================================================================*
 * do_create()                                                                *
 *============================================================================*/

/**
 * @brief Handles a create request.
 */
static inline int do_create(struct shm_message *msg, struct shm_message *response)
{
	int ret;
	struct shm_message msg1;

	/* Persist first message. */
	if (!(msg->seq & 1))
	{
		assert(buffer_put(msg->header.source, msg) == 0);
		return (0);
	}

	/* Get first message. */
	assert(buffer_get(msg->header.source, &msg1) == 0);
	assert(msg->seq == (msg1.seq | 1));

	ret = shm_create(
		msg->header.source,
		msg1.op.create1.name,
		msg->op.create2.mode,
		msg->op.create2.rw
	);
	
	response->header.source = msg->header.source;
	if (ret >= 0)
	{
		response->op.ret.shmid = ret;
		response->header.opcode = SHM_SUCCESS;
	}
	else
	{
		response->op.ret.status = -ret;
		response->header.opcode = SHM_FAILURE;
	}

	return (1);
}

/*============================================================================*
 * do_create_excl()                                                           *
 *============================================================================*/

/**
 * @brief Handles an exclusive create request.
 */
static int do_create_excl(struct shm_message *msg, struct shm_message *response)
{
	int ret;
	struct shm_message msg1;

	/* Persist first message. */
	if (!(msg->seq & 1))
	{
		assert(buffer_put(msg->header.source, msg) == 0);
		return (0);
	}

	/* Get first message. */
	assert(buffer_get(msg->header.source, &msg1) == 0);
	assert(msg->seq == (msg1.seq | 1));

	ret = shm_create_exclusive(
		msg->header.source,
		msg1.op.create1.name,
		msg->op.create2.mode,
		msg->op.create2.rw
	);

	response->header.source = msg->header.source;
	if (ret >= 0)
	{
		response->op.ret.shmid = ret;
		response->header.opcode = SHM_SUCCESS;
	}
	else
	{
		response->op.ret.status = -ret;
		response->header.opcode = SHM_FAILURE;
	}
	
	return (1);
}

/*============================================================================*
 * do_open()                                                                  *
 *============================================================================*/

/**
 * @brief Handles an open request.
 */
static int do_open(struct shm_message *msg, struct shm_message *response)
{
	int ret;
	struct shm_message msg1;

	/* Persist first message. */
	if (!(msg->seq & 1))
	{
		assert(buffer_put(msg->header.source, msg) == 0);
		return (0);
	}

	/* Get first message. */
	assert(buffer_get(msg->header.source, &msg1) == 0);
	assert(msg->seq == (msg1.seq | 1));

	ret = shm_open(
		msg->header.source,
		msg1.op.open1.name,
		msg->op.open2.rw,
		msg->op.open2.truncate
	);

	response->header.source = msg->header.source;
	if (ret >= 0)
	{
		response->op.ret.shmid = ret;
		response->header.opcode = SHM_SUCCESS;
	}
	else
	{
		response->op.ret.status = -ret;
		response->header.opcode = SHM_FAILURE;
	}

	return (1);
}

/*============================================================================*
 * do_unlink()                                                                *
 *============================================================================*/

/**
 * @brief Handles an unlink request.
 */
static int do_unlink(struct shm_message *msg, struct shm_message *response)
{
	int ret;

	ret = shm_unlink(msg->header.source, msg->op.unlink.name);

	response->header.source = msg->header.source;
	if (ret == 0)
	{
		response->op.ret.status = 0;
		response->header.opcode = SHM_SUCCESS;
	}
	else
	{
		response->op.ret.status = -ret;
		response->header.opcode = SHM_FAILURE;
	}

	return (1);
}

/*============================================================================*
 * do_map()                                                                   *
 *============================================================================*/

/**
 * @brief Handles a map request.
 */
static int do_map(struct shm_message *msg, struct shm_message *response)
{
	int ret;
	uint64_t mapblk;

	ret = shm_map(
		msg->header.source,
		msg->op.map.shmid,
		msg->op.map.size,
		msg->op.map.writable,
		msg->op.map.shared,
		msg->op.map.off,
		&mapblk
	);

	response->header.source = msg->header.source;
	if (ret == 0)
	{
		response->op.ret.mapblk = mapblk;
		response->header.opcode = SHM_SUCCESS;
	}
	else
	{
		response->op.ret.status = -ret;
		response->header.opcode = SHM_FAILURE;
	}
	
	return (1);
}

/*============================================================================*
 * do_unmap()                                                                 *
 *============================================================================*/

/**
 * @brief Handles an unmap request.
 */
static int do_unmap(struct shm_message *msg, struct shm_message *response)
{
	int ret;

	ret = shm_unmap(msg->header.source, msg->op.unmap.shmid);

	response->header.source = msg->header.source;
	if (ret == 0)
	{
		response->op.ret.status = 0;
		response->header.opcode = SHM_SUCCESS;
	}
	else
	{
		response->op.ret.status = -ret;
		response->header.opcode = SHM_FAILURE;
	}

	return (1);
}

/*============================================================================*
 * do_truncate()                                                              *
 *============================================================================*/

/**
 * @brief Handles a truncate request.
 */
static int do_truncate(struct shm_message *msg, struct shm_message *response)
{
	int ret;

	ret = shm_truncate(
		msg->header.source,
		msg->op.truncate.shmid,
		msg->op.truncate.size
	);

	response->header.source = msg->header.source;
	if (ret == 0)
	{
		response->op.ret.status = 0;
		response->header.opcode = SHM_SUCCESS;
	}
	else
	{
		response->op.ret.status = -ret;
		response->header.opcode = SHM_FAILURE;
	}

	return (1);
}

/*============================================================================*
 * do_null()                                                                  *
 *============================================================================*/

/**
 * @brief Handles a null request.
 */
static int do_null(struct shm_message *msg, struct shm_message *response)
{
	response->header.opcode = SHM_FAILURE;
	response->op.ret.status = EINVAL;
	response->header.source = msg->header.source;

	return (1);
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
	int shutdown = 0;

	while(!shutdown)
	{
		int reply = 0;
		struct shm_message request;
		struct shm_message response;

		assert(sys_mailbox_read(inbox, &request, sizeof(struct shm_message)) == MAILBOX_MSG_SIZE);

		/* Invalid process ID. */
		if (request.header.source >= HAL_NR_NOC_NODES)
			continue;

		/* Handle request. */
		switch (request.header.opcode)
		{
			case SHM_CREATE:
				reply = do_create(&request, &response);
				break;

			case SHM_CREATE_EXCL:
				reply = do_create_excl(&request, &response);
				break;

			case SHM_OPEN:
				reply = do_open(&request, &response);
				break;

			case SHM_UNLINK:
				reply = do_unlink(&request, &response);
				break;

			case SHM_MAP:
				reply = do_map(&request, &response);
				break;

			case SHM_UNMAP:
				reply = do_unmap(&request, &response);
				break;

			case SHM_TRUNCATE:
				reply = do_truncate(&request, &response);
				break;

			case SHM_EXIT:
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
			assert((outbox = sys_mailbox_open(response.header.source)) >= 0);
			assert(sys_mailbox_write(outbox, &response, sizeof(struct shm_message)) == MAILBOX_MSG_SIZE);
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

	spawner_ack();

	printf("[nanvix][shm] server alive\n");

	if ((ret = shm_loop()) < 0)
		goto error;

	printf("[nanvix][shm] shutting down server\n");

	if ((ret = shm_shutdown()) < 0)
		goto error;

	return (0);

error:
	return (ret);
}

