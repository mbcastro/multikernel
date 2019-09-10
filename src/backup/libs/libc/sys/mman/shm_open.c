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
#include <fcntl.h>
#include <sys/types.h>

#include <nanvix/mm.h>

/**
 * @brief Opens a shared memory region.
 *
 * @details The shm_open() function establishes a connection between a
 * shared memory region and a file descriptor. It creates an open file
 * description that refers to the shared memory region and a file
 * descriptor that refers to that open file description. The file
 * descriptor can be used by other functions to refer to that shared
 * memory region.
 *
 * When a shared memory region is created, the state of the shared
 * memory region, including all data associated with the shared memory
 * region, persists until the shared memory region is unlinked and all
 * other references are gone.
 *
 * @param name  String naming a shared memory region.
 * @param oflag Opening flags.
 * @param mode  Access mode.
 *
 * @returns Upon successful completion, a non-negative integer
 * representing the file descriptor is returned.  Otherwise, -1 is
 * returned instead and set errno to indicate the error.
 */
int nanvix2_shm_open(const char *name, int oflag, mode_t mode)
{
	int rw;
	int ret;
	int truncate;

	rw = oflag & O_RDWR;
	truncate = oflag & O_TRUNC;

	/* Cannot truncate region. */
	if ((!rw) && truncate)
	{
		errno = EACCES;
		return (-1);
	}

	/* Exclusive create. */
	if ((oflag & O_CREAT) && (oflag & O_EXCL))
		ret = nanvix_shm_create_excl(name, rw, mode);

	/* Create. */
	else if (oflag & O_CREAT)
		ret = nanvix_shm_create(name, rw, truncate, mode);

	/* Open. */
	else
		ret = nanvix_shm_open(name, rw, truncate);

	return (ret);
}
