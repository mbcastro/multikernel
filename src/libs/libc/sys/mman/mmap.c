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

#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

#include <nanvix/mm.h>

/**
 * @brief Maps pages of memory.
 *
 * @param addr  Hint local address.
 * @param len   Length of mapping (in bytes).
 * @param prot  Protection for mapping.
 * @param flags Opening flags.
 * @param fd    Target file descriptor.
 * @param off   Offset within file.
 *
 * @retuns Upon successful completion, the address at which the
 * mapping was placed is returned. Otherwise, MAP_FAILED is returned
 * and errno is set to indicate the error.
 */
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
	void *map;

	((void)addr);

	/* Invalid length. */
	if (len == 0)
	{
		errno = EINVAL;
		return (MAP_FAILED);
	}

	/* Protection not supported.*/
	if ((prot & PROT_EXEC) || (prot & PROT_NONE))
	{
		errno = ENOTSUP;
		return (MAP_FAILED);
	}

	/* Fixed mapping not supported. */
	if (flags == MAP_FIXED)
	{
		errno = ENOTSUP;
		return (MAP_FAILED);
	}

	/* Invalid mapping. */
	if ((flags != MAP_SHARED) && (flags != MAP_PRIVATE))
	{
		errno = EINVAL;
		return (MAP_FAILED);
	}

	map = nanvix_mmap(len, prot & PROT_WRITE, flags == MAP_SHARED, fd, off);

	return ((map == NULL) ? MAP_FAILED : map);
}
