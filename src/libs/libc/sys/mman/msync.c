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
#include <errno.h>
#include <stddef.h>

#include <nanvix/fs.h>

/**
 * @brief Synchronizes memory with physical storage.
 *
 * @param addr  Target local address.
 * @param len   Number of bytes to synchronize.
 * @param flags Synchronization flags.
 *
 * @para Upon successful completion, zero is returned. Upon failure,
 * -1 is returned instead and errno is set to indicate the error.
 */
int msync(void *addr, size_t len, int flags)
{
	/* Invalid flags. */
	if ((flags & MS_ASYNC) && (flags & MS_SYNC))
	{
		errno = EINVAL;
		return (-1);
	}

	return (nanvix_msync(addr, len, flags & MS_SYNC, flags & MS_INVALIDATE));
}
