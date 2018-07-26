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
#include <semaphore.h>
#include <stdarg.h>
#include <string.h>

#include <nanvix/semaphore.h>

#include "semaphore.h"

/**
 * @brief Initializes and opens a named semaphore.
 *
 * @param name	Target name.
 * @param oflag	Creation flags.
 * @param mode	User permissions.
 * @param value	Semaphore count value.
 *
 * @returns Upon successful completion, the  address of the semaphore
 * is returned. Upon failure, SEM_FAILEDED is returned and errno is set
 * ot indicate the error.
 *
 * @todo Rename NANVIX_SEM_NAME_MAX to _POSIX_PATH_MAX
 */
sem_t *sem_open(const char *name, int oflag, ...)
{
	int sem;
	sem_t semid = -1;

	/* Invalid name. */
	if ((name == NULL) || (!strcmp(name, "")))
	{
		errno = EINVAL;
		return (SEM_FAILED);
	}

	/* Name too long. */
	if (strlen(name) >= (NANVIX_SEM_NAME_MAX))
	{
		errno = ENAMETOOLONG;
		return (SEM_FAILED);
	}

	/* Allocate semaphore. */
	if ((sem = _sem_alloc()) < 0)
	{
		errno = EINVAL;
		return (SEM_FAILED);
	}

	/* Create semaphore. */
	if (oflag & O_CREAT)
	{
		va_list ap;     /* Arguments pointer. */
		mode_t mode;    /* Creation mode.     */
		unsigned value; /* Semaphore value.   */

		/* Retrieve additional arguments. */
		va_start(ap, oflag);
		mode = va_arg(ap, mode_t);
		value = va_arg(ap, unsigned);
		va_end(ap);

		/* Invalid semaphore value. */
		if (value > SEM_VALUE_MAX)
			goto error;

		semid = nanvix_sem_create(name, mode, value, (oflag & O_EXCL));
	}

	/* Open semaphore. */
	else
		semid = nanvix_sem_open(name);

	_semaphores[sem].id = semid;
	return (&_semaphores[sem].id);

error:
	_sem_free(sem);
	errno = EINVAL;
	return (SEM_FAILED);
}
