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
#include <stdint.h>
#include <stdlib.h>

#include <nanvix/mm.h>

/**
 * @brief Number of current mappings.
 */
static int nmappings = 0;

/**
 * @brief Mapping quota usage.
 */
static size_t quota = 0;

/**
 * @brief Opened mappings.
 */
static struct
{
	int shmid;       /**< UNderlying shared memory region. */
	void *local;     /**< Local address.                   */
	uint64_t remote; /**< Remote address.                  */
	size_t size;     /**< Mapping size.                    */
	int shared;      /**< Shared? Else private.            */
	int writable;    /**< Writable? Else read-only.        */
} mappings[SHM_OPEN_MAX];

/*============================================================================*
 * nanvix_get_mapping()                                                       *
 *============================================================================*/

/**
 * @brief Gets a memory mapping with a given shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 * 
 * @returns If a memory mapping is currently opened with the target
 * shared memory region, its index in the table of opened memory
 * mappings is returned. Otherwise, -1 is returned instead.
 */
static int nanvix_get_mapping1(int shmid)
{
	for (int i = 0; i < nmappings; i++)
	{
		if (mappings[i].shmid == shmid)
			return (i);
	}

	return (-1);
}

/**
 * @brief Gets a memory mapping at a given local address.
 *
 * @param local Target local address.
 * 
 * @returns If a memory mapping is currently opened at the target
 * local address, its index in the table of opened memory mappings is
 * returned. Otherwise, -1 is returned instead.
 */
static int nanvix_get_mapping2(const void *local)
{
	for (int i = 0; i < nmappings; i++)
	{
		if (mappings[i].local == local)
			return (i);
	}

	return (-1);
}

/*============================================================================*
 * nanvix_mmap()                                                              *
 *============================================================================*/

/**
 * @brief Maps pages of memory.
 *
 * @param len      Length of mapping (in bytes).
 * @param writable Writable? Else read-only.
 * @param shared   Shared? Else private.
 * @param fd       Target file descriptor.
 * @param off      Offset within file.
 *
 * @retuns Upon successful completion, the address at which the
 * mapping was placed is returned. Otherwise, NULL is returned
 * and errno is set to indicate the error.
 */
void *nanvix_mmap(size_t len, int writable, int shared, int fd, off_t off)
{
	void *map;
	uint64_t mapblk;

	/* Invalid length. */
	if (len == 0)
	{
		errno = EINVAL;
		return (NULL);
	}

	/* Too mapning opened mappings. */
	if (nmappings >= SHM_OPEN_MAX)
	{
		errno = ENFILE;
		return (NULL);
	}

	/* Not enough memory. */
	if ((quota + len) > SHM_MAP_SIZE_MAX)
	{
		errno = ENOMEM;
		return (NULL);
	}

	/* Cannot allocate local region. */
	if ((map = malloc(len)) == NULL)
		return (NULL);

	/* Map shared memory region. */
	if (nanvix_map(&mapblk, len, writable, shared, fd, off) < 0)
		goto error0;

	mappings[nmappings].shmid = fd;
	mappings[nmappings].size = len;
	mappings[nmappings].local = map;
	mappings[nmappings].remote = mapblk;
	mappings[nmappings].shared = shared;
	mappings[nmappings].writable = writable;
	quota += len;
	nmappings++;

	/* Synchronize region. */
	memread(mapblk, map, len);

	return (map);

error0:
	free(map);
	return (NULL);
}

/*============================================================================*
 * nanvix_munmap()                                                            *
 *============================================================================*/

/**
 * @brief Unmaps pages of memory.
 *
 * @param addr  Mapping address.
 * @param len   Length of mapping (in bytes).
 *
 * @retuns Upon successful completion, zero is returned. Otherwise, -1
 * is returned and errno is set to indicate the error.
 */
int nanvix_munmap(void *addr, size_t len)
{
	int i;

	/* Invalid length. */
	if (len == 0)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Invalid shared memory region. */
	if ((i = nanvix_get_mapping2(addr)) < 0)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Invalid size. */
	if (len != mappings[i].size)
		return (-EINVAL);

	/* Unamp region. */
	if (nanvix_unmap(mappings[i].shmid, mappings[i].size) < 0)
		return (-1);

	/* Synchronize region. */
	if ((mappings[i].shared) && (mappings[i].writable))
		memwrite(mappings[i].remote, mappings[i].local, len);

	/* Remove mapping. */
	free(mappings[i].local);
	quota -= len;

	/* Remove from list of opened mappings. */
	nmappings--;
	for (int j = i; j < nmappings; j++)
	{
		mappings[j].shmid = mappings[j + 1].shmid;
		mappings[j].size = mappings[j + 1].size;
		mappings[j].local = mappings[j + 1].local;
		mappings[j].remote = mappings[j + 1].remote;
	}

	return (0);
}

/*============================================================================*
 * nanvix_msync()                                                             *
 *============================================================================*/

/**
 * @brief Synchronizes memory with physical storage.
 *
 * @param addr       Target local address.
 * @param len        Number of bytes to synchronize.
 * @param async      Asynchronous write? Else synchronous.
 * @param invalidate Invaldiate cached data? Else no.
 *
 * @para Upon successful completion, zero is returned. Upon failure,
 * -1 is returned instead and errno is set to indicate the error.
 */
int nanvix_msync(void *addr, size_t len, int async, int invalidate)
{
	int i;

	/* Not supported. */
	if (async)
	{
		errno = ENOTSUP;
		return (-1);
	}

	/* Invalid shared memory region. */
	if ((i = nanvix_get_mapping2(addr)) < 0)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Invalidate cached data. */
	if (invalidate)
	{
		memread(mappings[i].remote, mappings[i].local, len);
		return (0);
	}

	/* Synchronize region. */
	if ((mappings[i].shared) && (mappings[i].writable))
		memwrite(mappings[i].remote, mappings[i].local, len);

	return (0);
}

/*============================================================================*
 * nanvix_mtruncate()                                                         *
 *============================================================================*/

/**
 * @brief Truncates a file to a specified length.
 *
 * @param fd     Target file descriptor.
 * @param length File length (in bytes).
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, -1 is returned instead, and errno is set to indicate the
 * error.
 */
int nanvix_ftruncate(int fd, off_t length)
{
	int i;

	/* Invalid file descriptor. */
	if (fd < 0)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Invalid length. */
	if (length <= 0)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Bad shared memory region. */
	if ((i = nanvix_get_mapping1(fd)) >= 0)
	{
		errno = EINVAL;
		return (-1);
	}

	return (nanvix_mtruncate(fd, length));
}
