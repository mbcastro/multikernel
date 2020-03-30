/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Must come first. */
#define __NEED_RESOURCE
#define __SHM_SERVER

#include <nanvix/servers/shm.h>
#include <nanvix/sys/noc.h>
#include <nanvix/ulib.h>
#include <posix/sys/stat.h>
#include <posix/errno.h>

/**
 * @brief Table of processes.
 */
static struct
{
	/**
	 * Table of opened shared memory regions.
	 */
	struct
	{
		/*
		 * XXX: Don't Touch! This Must Come First!
		 */
		struct resource resource; /**< Generic resource information.  */

		int shmid;
		int flags;
	} oregions[SHM_OPEN_MAX];
} procs[PROCESSOR_CLUSTERS_NUM];

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
static int oshm_may_write(int node, int id)
{
	return (resource_is_writable(&procs[node].oregions[id].resource));
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
static int oshm_is_shared(int node, int id)
{
	return (resource_is_shared(&procs[node].oregions[id].resource));
}

/*============================================================================*
 * oshm_has_mapped()                                                           *
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
static int oshm_has_mapped(int node, int id)
{
	return (resource_is_mapped(&procs[node].oregions[id].resource));
}

/*============================================================================*
 * oshm_is_valid()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a opened shared memory region ID is valid.
 *
 * @param oshmid ID of the target opened shared memory region.
 *
 * @returns Non-zero if the opened shared memory region valid, and
 * zero otherwise.
 */
static inline int oshm_is_valid(int oshmid)
{
	return ((oshmid >= 0) && (oshmid < SHM_OPEN_MAX));
}

/*============================================================================*
 * oshm_is_used()                                                           *
 *============================================================================*/

/**
 * @brief Asserts whether or not a opened shared memory region slot is used.
 *
 * @param node  Target node.
 * @param id ID of the opened shared memory region.
 *
 * @returns Non-zero if the slot is marked as used, and zero otherwise.
 */
static int oshm_is_used(int node, int id)
{
	return (oshm_is_valid(id) && (resource_is_used(&procs[node].oregions[id].resource)));
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
static void oshm_set_writable(int node, int id)
{
	resource_set_rdwr(&procs[node].oregions[id].resource);
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
static void oshm_set_mapped(int node, int id)
{
	resource_set_mapped(&procs[node].oregions[id].resource);
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
static void oshm_set_shared(int node, int id)
{
	resource_set_shared(&procs[node].oregions[id].resource);
}

/*============================================================================*
 * oshm_set_used()                                                            *
 *============================================================================*/

/**
 * @brief Sets a shared memory region as used.
 *
 * @param node Number of the target node.
 * @param id   ID of the target opened shared memory region.
 */
static void oshm_set_used(int node, int id)
{
	resource_set_used(&procs[node].oregions[id].resource);
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
static int shm_name_is_valid(const char *name)
{
	return ((name != NULL) &&
			(ustrlen(name) < (SHM_NAME_MAX - 1)) &&
			(ustrcmp(name, ""))
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
static int oshm_has_opened(int node, int shmid)
{
	for (int i = 0; i < SHM_OPEN_MAX; i++)
	{
		if (procs[node].oregions[i].shmid != shmid)
			continue;

		if (oshm_is_used(node, i))
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
 * @returns One if the target node has mapped the target mapped
 * memory region, and zero otherwise.
 */
static int oshm_is_mapped(int shmid)
{
	for (int i = 0; i < PROCESSOR_CLUSTERS_NUM; i++)
	{
		for (int j = 0; j < SHM_OPEN_MAX; j++)
		{
			if (procs[i].oregions[j].shmid != shmid)
				continue;

			if (oshm_is_used(i, j) && oshm_has_mapped(i, j))
				return (1);
		}
	}

	return (0);
}


/*============================================================================*
 * oshm_alloc()                                                                *
 *============================================================================*/

/**
 * @brief Allocates a shared memory region.
 *
 * @param node Number of the target node.
 *
 * @return Upon successful completion, the ID of the newly allocated
 * shared memory region is returned. Upon failure, -1 is returned instead.
 */
static int oshm_alloc(int node)
{
	/* Search for a free shared memory region. */
	for (int i = 0; i < SHM_OPEN_MAX; i++)
	{
		/* Found. */
		if (!oshm_is_used(node, i))
		{
			oshm_set_used(node, i);
			return (i);
		}
	}

	return (-1);
}

/*============================================================================*
 * shm_free()                                                              *
 *============================================================================*/

/**
 * @brief Free a shared memory region.
 *
 * @param node Number of the target node.
 * @param oshmid ID of the opened shared memory region.
 */
static void oshm_free(int node, int id)
{
	procs[node].oregions[id].resource = RESOURCE_INITIALIZER;
}

/*============================================================================*
 * do_open()                                                                  *
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
int __do_shm_open(int node, const char *name, int writable, int truncate)
{
	int shmid;  /* Shared memory region ID.        */
	int oshmid; /* Opened shared memory region ID. */

	shm_debug("open node=%d name=%s", node, name);

	/* Invalid name. */
	if (!shm_name_is_valid(name))
		return (-EINVAL);

	/* Get shared memory. */
	if ((shmid = shm_get(name)) < 0)
		return (-EINVAL);

	/* Incompatible opening flags */
	if ((!shm_is_writable(shmid) && writable) || !shm_is_readable(shmid))
	{
		shm_put(shmid);
		return  (-EINVAL);
	}

	/* Shared memory region shall be removed soon. */
	if (shm_is_remove(shmid))
	{
		shm_put(shmid);
		return (-EAGAIN);
	}

	/* Too many files are opened. */
	if ((oshmid = oshm_alloc(node)) < 0)
	{
		shm_put(shmid);
		return (-ENFILE);
	}

	/* Truncate. */
	if (truncate)
	{
		/* Cannot write. */
		if (!writable)
		{
			shm_put(shmid);
			oshm_free(node, oshmid);
			return (-EINVAL);
		}

		/* Already mapped. */
		if (oshm_is_mapped(shmid))
		{
			shm_put(shmid);
			oshm_free(node, oshmid);
			return (-EBUSY);
		}

		shm_set_size(shmid, 0);
	}

	procs[node].oregions[oshmid].shmid = shmid;
	if (writable)
		oshm_set_writable(node, oshmid);

	return (oshmid);
}

/*============================================================================*
 * do_create()                                                                *
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
 * opened shared memory region is returned. Upon failure, a negative
 * error code is returned instead.
 */
int __do_shm_create(int owner, const char *name, int writable, mode_t mode)
{
	int shmid;  /* Shared memory region ID.        */
	int oshmid; /* Opened shared memory region ID. */

	shm_debug("create node=%d name=%s mode=%d", owner, name, mode);

	/* Invalid name. */
	if (!shm_name_is_valid(name))
		return (-EINVAL);

	/* Look for shared memory region. */
	if ((shmid = shm_get(name)) >= 0)
	{
		shm_put(shmid);
		return (__do_shm_open(owner, name, writable, 0));
	}

	/* Allocate a new opened shm. */
	if ((oshmid = oshm_alloc(owner)) < 0)
		return (-ENFILE);

	/* Allocate a new shm. */
	if ((shmid = shm_alloc()) < 0)
	{
		oshm_free(owner, oshmid);
		return (-EAGAIN);
	}

	/* Initialize shared memory region. */
	shm_set_perm(shmid, owner, mode);
	shm_set_name(shmid, name);
	shm_set_base(shmid, 0);
	shm_set_size(shmid, 0);

	procs[owner].oregions[oshmid].shmid = shmid;
	if (writable)
		oshm_set_writable(owner, oshmid);

	return (oshmid);
}

/*============================================================================*
 * do_create_excl()                                                           *
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
int __do_shm_create_exclusive(int owner, char *name, int writable, mode_t mode)
{
	int shmid; /* Shared memory region ID.        */

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

	return (__do_shm_create(owner, name, writable, mode));
}

/*============================================================================*
 * __do_shm_close()                                                           *
 *============================================================================*/

/**
 * @brief Close a opened shared memory region
 *
 * @param node   ID of opening process.
 * @param oshmid Opened shared memory region id.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int __do_shm_close(int node, int oshmid)
{
	int shmid; /* Shared memory region ID.        */

	shm_debug("close node=%d oshmid=%d", node, oshmid);

	/* Opened shared memory region not in use. */
	if (!oshm_is_used(node, oshmid))
		return (-EINVAL);

	shmid = procs[node].oregions[oshmid].shmid;

	/* Shared memory region not in use. */
	if (!shm_is_used(shmid))
		return (-EINVAL);

	shm_put(shmid);

	oshm_free(node, oshmid);

	return (0);
}

/*============================================================================*
 * do_unlink()                                                                *
 *============================================================================*/

/**
 * @brief Unlink a shared memory region
 *
 * @param node ID the calling process.
 * @param name Name of the targeted shm.
 *
 * @returns Upon successful completion, oshmid is returned.
 * Upon failure, a negative error code is returned instead.
 */
int __do_shm_unlink(int node, const char *name)
{
	int ret;    /* Return of closing.              */
	int shmid;  /* Shared memory region ID.        */
	int oshmid; /* Opened shared memory region ID. */

	shm_debug("unlink node=%d name=%s", node, name);

	/* Shared memory region does not exist. */
	if ((shmid = shm_get(name)) < 0)
		return (-EINVAL);

	shm_put(shmid);

	/* Do I own the shared memory region? */
	if (!shm_is_owner(shmid, node))
		return (-EPERM);

	/* Opened shared memory region does not exist. */
	if ((oshmid = oshm_has_opened(node, shmid)) < 0)
		return (-EINVAL);

	shm_set_remove(shmid);

	/* Did I close the shared memory region correctly? */
	if ((ret = __do_shm_close(node, oshmid) < 0))
		return ret;

	return oshmid;
}
/*============================================================================*
 * do_map()                                                                   *
 *============================================================================*/

/**
 * @brief Maps a shared memory region.
 *
 * @param node     ID of the calling node.
 * @param oshmid   ID of the opened shared memory region.
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
int __do_shm_map(
	int node,
	int oshmid,
	size_t size,
	int writable,
	int shared,
	off_t off,
	uint64_t *mapblk)
{
	int shmid; /* Shared memory region ID. */

	shm_debug("map node=%d oshmid=%d", node, oshmid);

	/* Opened shared memory region not in use. */
	if (!oshm_is_used(node, oshmid))
		return (-EINVAL);

	shmid = procs[node].oregions[oshmid].shmid;

	/* Shared memory region not in use. */
	if (!shm_is_used(shmid))
		return (-EINVAL);

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
	if (writable && (!oshm_may_write(node, oshmid)))
		return (-EACCES);

	/* Map. */
	if (!oshm_has_mapped(node, oshmid))
	{
		oshm_set_mapped(node, oshmid);
		if (shared)
			oshm_set_shared(node, oshmid);
	}

	*mapblk = shm_get_base(shmid) + off;

	return (0);
}

/*============================================================================*
 * do_truncate()                                                              *
 *============================================================================*/

/**
 * @brief Truncates a shared memory region to a specified size.
 *
 * @param node   ID of opening process.
 * @param oshmid ID of the opened shared memory region.
 * @param length Shared memory region size (in bytes).
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, -1 is returned instead, and errno is set to indicate the
 * error.
 */
int __do_shm_truncate(int node, int oshmid, size_t size)
{
	int shmid; /* Shared memory region ID. */

	shm_debug("truncate node=%d oshmid=%d size=%d", node, oshmid, size);

	/* Opened shared memory region not in use. */
	if (!oshm_is_used(node, oshmid))
		return (-EINVAL);

	shmid = procs[node].oregions[oshmid].shmid;

	/* Not enought memory. */
	if (size > PAGE_SIZE)
		return (-ENOMEM);

	/* Shared memory region not in use. */
	if (!shm_is_used(shmid))
		return (-EINVAL);

	/* Cannot write. */
	if (!oshm_may_write(node, oshmid))
		return (-EINVAL);

	/* Already mapped. */
	if (oshm_is_mapped(shmid))
		return (-EBUSY);

	shm_set_size(shmid, size);

	return (0);
}

/**
 * @brief Unmaps a shared memory region.
 *
 * @param node   ID of the calling node.
 * @param oshmid ID of the opened shared memory region.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int __do_shm_unmap(int node, int oshmid)
{
	int shmid; /* Shared memory region ID. */

	shm_debug("unmap node=%d oshmid=%d", node, oshmid);

	/* Opened shared memory region not in use. */
	if (!oshm_is_used(node, oshmid))
		return (-EINVAL);

	shmid = procs[node].oregions[oshmid].shmid;

	/* Shared memory region not in use. */
	if (!shm_is_used(shmid))
		return (-EINVAL);

	/* Not mapped. */
	if (!oshm_has_mapped(node, oshmid))
		return (-EINVAL);

	return (0);
}
