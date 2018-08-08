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

#include <nanvix/mm.h>

/**
 * @brief Removes a shared memory region.
 *
 * @details The shm_unlink() removes the name of the shared memory
 * region named by the string pointed to by @p name. 
 *
 * If one or more references to the shared memory region exist when
 * the region is unlinked, the @p name is removed before shm_unlink()
 * returns, but the removal of the memory region contents are
 * postponed until all open and map references to the shared memory
 * region have been removed.
 *
 * Even if the region continues to exist after the last shm_unlink(),
 * reuse of the name shall subsequently cause shm_open() to behave as
 * if no shared memory region of this name exists.
 *
 * @returns Upon successful completion, zero is returned returned.
 * Otherwise, -1 is returned and errno set to indicate the error. If
 * -1 is returned, the named shared memory object is not changed by
 *  this function call. 
 */
int shm_unlink(const char *name)
{
	return (nanvix_shm_unlink(name));
}
