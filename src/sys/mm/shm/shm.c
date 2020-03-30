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
 * @brief Table of shared memory regions.
 */
static struct region
{
	/*
	 * XXX: Don't Touch! This Must Come First!
	 */
	struct resource resource; /**< Generic resource information.  */

	char name[SHM_NAME_MAX];  /**< Shared memory region name.     */
	int flags;                /**< Flags.                         */
	int owner;                /**< ID of owner process.           */
	int refcount;             /**< Number of references.          */
	mode_t mode;              /**< Access permissions.            */
	uint64_t base;            /**< Base address.                  */
	size_t size;              /**< Size (in bytes).               */
} regions[SHM_MAX];

/**
 * @brief Pool of shared memory regions.
 */
static struct resource_pool pool = {
	regions, SHM_MAX, sizeof(struct region)
};

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
int shm_is_used(int shmid)
{
	return (shm_is_valid(shmid) && (resource_is_used(&regions[shmid].resource)));
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
int shm_is_remove(int shmid)
{
	return (!resource_is_busy(&regions[shmid].resource));
}

/*============================================================================*
 * shm_is_owner()                                                             *
 *============================================================================*/

/**
 * @brief Asserts whether or not a given node owns a given shared
 * memory region.
 *
 * @param shmid Target shared memory region.
 * @param node  Target node.
 *
 * @returns Non-zero if the target shared memory region is owned by
 * the target node, and zero otherwise.
 */
int shm_is_owner(int shmid, int node)
{
	return (regions[shmid].owner == node);
}

/*============================================================================*
 * shm_is_readable()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a given shared memory region has
 * read permission.
 *
 * @param shmid Target shared memory region.
 *
 * @returns Non-zero if the target shared memory region is readable,
 * and zero otherwise.
 */
int shm_is_readable(int shmid)
{
	return (resource_is_readable(&regions[shmid].resource));
}

/*============================================================================*
 * shm_is_writable()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a given shared memory region has
 * write permission.
 *
 * @param shmid Target shared memory region.
 *
 * @returns Non-zero if the target shared memory region is writable,
 * and zero otherwise.
 */
int shm_is_writable(int shmid)
{
	return (resource_is_writable(&regions[shmid].resource));
}

/*============================================================================*
 * shm_get_base()                                                             *
 *============================================================================*/

/**
 * @brief Gets the base address of a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 *
 * @param The base address of the target shared memory region.
 */
uint64_t shm_get_base(int shmid)
{
	return (regions[shmid].base);
}

/*============================================================================*
 * shm_get_size()                                                             *
 *============================================================================*/

/**
 * @brief Gets the size (in bytes) of a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 *
 * @returns The size (in bytes) of the target shared memory region.
 */
size_t shm_get_size(int shmid)
{
	return (regions[shmid].size);
}

/*============================================================================*
 * shm_set_remove()                                                           *
 *============================================================================*/

/**
 * @brief Marks a shared memory region to be removed.
 *
 * @param shmid ID of the target shared memory region.
 */
void shm_set_remove(int shmid)
{
	resource_set_notbusy(&regions[shmid].resource);
}

/*============================================================================*
 * shm_set_perm()                                                             *
 *============================================================================*/

/**
 * @brief Sets the access permissions of a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 */
void shm_set_perm(int shmid, int owner, mode_t mode)
{
	regions[shmid].owner = owner;
	regions[shmid].mode = mode;

	if (regions[shmid].mode & S_IWUSR)
		resource_set_rdwr(&regions[shmid].resource);
}

/*============================================================================*
 * shm_set_name()                                                             *
 *============================================================================*/

/**
 * @brief Sets the name of a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 * @param name  Name for the target shared memory region.
 */
void shm_set_name(int shmid, const char *name)
{
	ustrcpy(regions[shmid].name, name);
}

/*============================================================================*
 * shm_set_base()                                                             *
 *============================================================================*/

/**
 * @brief Sets the base address of a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 * @param base  Base address of the target shared memory region.
 */
void shm_set_base(int shmid, uint64_t base)
{
	regions[shmid].base = base;
}

/*============================================================================*
 * shm_set_size()                                                             *
 *============================================================================*/

/**
 * @brief Sets the size (in bytes) of a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 * @param size  Size of the target shared memoy region.
 */
void shm_set_size(int shmid, size_t size)
{
	regions[shmid].size = size;
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
int shm_alloc(void)
{
	int shmid;

	if ((shmid = resource_alloc(&pool)) < 0)
		return (-1);

	regions[shmid].refcount = 1;
	resource_set_busy(&regions[shmid].resource);
	resource_set_rdonly(&regions[shmid].resource);

	return (shmid);
}

/*============================================================================*
 * shm_free()                                                                 *
 *============================================================================*/

/**
 * @brief Free a shared memory region.
 */
static void shm_free(int shmid)
{
	resource_free(&pool, shmid);
}

/*============================================================================*
 * shm_get()                                                                  *
 *============================================================================*/

/**
 * @brief Gets a shared memory region.
 *
 * @param name Name of the target shared memory region.
 *
 * @return If the target shared memory region is found, its ID is
 * returned. Otherwise, -1 is returned instead.
 */
int shm_get(const char *name)
{
	for (int i = 0; i < SHM_MAX; i++)
	{
		/* Shared memory region not in use. */
		if (!shm_is_used(i))
			continue;

		/* Found.*/
		if (!ustrcmp(regions[i].name, name))
		{
			regions[i].refcount++;
			return (i);
		}
	}

	return (-1);
}

/*============================================================================*
 * shm_put()                                                                  *
 *============================================================================*/

/**
 * @brief Releases a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 */
void shm_put(int shmid)
{
	regions[shmid].refcount--;

	/* Unlink the shared memory region if no process is using it anymore. */
	if ((regions[shmid].refcount == 0) && (shm_is_remove(shmid)))
		shm_free(shmid);
}

/*============================================================================*
 * shm_init()                                                                 *
 *============================================================================*/

/**
 * @brief Initializes the table of shared memory regions.
 */
void shm_init(void)
{
	for (int i = 0; i < SHM_MAX; i++)
	{
		regions[i].refcount = 0;
		regions[i].resource = RESOURCE_INITIALIZER;
	}
}
