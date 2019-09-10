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
#include <errno.h>
#include <semaphore.h>

#include <nanvix/semaphores.h>

#include "semaphore.h"

/**
 * @brief Semaphores table.
 */
struct _semaphore _semaphores[SEM_MAX];

/*============================================================================*
 * _sem_is_valid()                                                            *
 *============================================================================*/

/**
 * @brief Asserts whether or not a semaphore is valid.
 *
 * @param semid ID of the target semaphore.
 *
 * @param Returns non-zero if the target semaphore is valid, and zero
 * otherwise.
 */
int _sem_is_valid(int semid)
{
	for (int i = 0; i < SEM_MAX; i++)
	{
		if (_semaphores[i].id == semid)
			return (1);
	}

	return (0);
}

/*============================================================================*
 * sem_is_used()                                                              *
 *============================================================================*/

/**
 * @brief Asserts whether or not a semaphore is used.
 *
 * @param sem Target semaphore.
 *
 * @param Returns non-zero if the target semaphore is used, and zero
 * otherwise.
 */
static int sem_is_used(int sem)
{
	return (_semaphores[sem].used);
}

/*============================================================================*
 * sem_set_used()                                                             *
 *============================================================================*/

/**
 * @brief Sets a semaphore as used.
 *
 * @param sem Target semaphore.
 */
static inline void sem_set_used(int sem)
{
	_semaphores[sem].used = 1;
}

/*============================================================================*
 * sem_set_unused()                                                           *
 *============================================================================*/

/**
 * @brief Sets a semaphore as unused.
 *
 * @param sem Target semaphore.
 */
static inline void sem_set_unused(int sem)
{
	_semaphores[sem].used = 0;
}

/*============================================================================*
 * sem_alloc()                                                                *
 *============================================================================*/

/**
 * @brief Allocates a semaphore.
 *
 * @returns Upon successful completion, the ID of a newly allocated
 * semaphore is returned. Upon failure a negative error code is
 * returned instead.
 */
int _sem_alloc(void)
{
	for (int i = 0; i < SEM_MAX; i++)
	{
		if (!sem_is_used(i))
		{
			sem_set_used(i);
			return (i);
		}
	}

	return (-EAGAIN);
}

/*============================================================================*
 * _sem_free()                                                                *
 *============================================================================*/

/**
 * @brief Frees a semaphore.
 *
 * @param semid Target semaphore.
 *
 * @returns Upon successful completion, zero is returned. Upon failure
 * a negative error code is returned instead.
 */
void _sem_free(int semid)
{
	for (int i = 0; i < SEM_MAX; i++)
	{
		if (_semaphores[i].id == semid)
			sem_set_unused(i);
	}
}

