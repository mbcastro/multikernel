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

#include <nanvix/runtime/shm.h>
#include <posix/sys/mman.h>
#include <posix/sys/types.h>
#include <posix/sys/stat.h>
#include <posix/stddef.h>
#include <posix/errno.h>

/**
 * @todo TODO: provide a detailed description for this function.
 */
void *nanvix_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
	void *map;

	((void)addr);

	/* Invalid length. */
	if (len == 0)
	{
		errno = EINVAL;
		return (NANVIX_MAP_FAILED);
	}

	/* Protection not supported.*/
	if ((prot & NANVIX_PROT_EXEC) || (prot & NANVIX_PROT_NONE))
	{
		errno = ENOTSUP;
		return (NANVIX_MAP_FAILED);
	}

	/* Missing protection. */
	if (!((prot & NANVIX_PROT_WRITE) || (prot & NANVIX_PROT_READ)))
	{
		errno = EINVAL;
		return (NANVIX_MAP_FAILED);
	}

	/* Fixed mapping not supported. */
	if (flags == NANVIX_MAP_FIXED)
	{
		errno = ENOTSUP;
		return (NANVIX_MAP_FAILED);
	}

	/* Invalid mapping. */
	if ((flags != NANVIX_MAP_SHARED) && (flags != NANVIX_MAP_PRIVATE))
	{
		errno = EINVAL;
		return (NANVIX_MAP_FAILED);
	}

	map = __nanvix_mmap(
		len,
		prot & NANVIX_PROT_WRITE,
		flags == NANVIX_MAP_SHARED,
		fd,
		off
	);

	/* Map failed. */
	if (map == NULL)
		return (NANVIX_MAP_FAILED);

	return (map);
}
