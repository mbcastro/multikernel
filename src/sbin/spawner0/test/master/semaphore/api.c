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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <semaphore.h>

#include <mppaipc.h>

#include <nanvix/limits.h>

#include "test.h"

/*==========================================================================*
 * API Test: Create Unlink                                                  *
 *==========================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_semaphore_create_unlink(void)
{
	sem_t *sem;
	char semaphore_name[NANVIX_SEM_NAME_MAX];

	/* Create and unlink semaphore. */
	sprintf(semaphore_name, "/semaphore");
	TEST_ASSERT((sem = sem_open(semaphore_name, O_CREAT, 0, 0)) != SEM_FAILED);
	TEST_ASSERT(sem_unlink(semaphore_name) == 0);
}

/*==========================================================================*
 * API Test: Open Close                                                     *
 *==========================================================================*/

/**
 * @brief API Test: Open Close
 */
static void test_posix_semaphore_open_close(void)
{
	sem_t *sem;
	char semaphore_name[NANVIX_SEM_NAME_MAX];

	sprintf(semaphore_name, "/semaphore");
	TEST_ASSERT((sem = sem_open(semaphore_name, O_CREAT, 0, 0)) != SEM_FAILED);
	TEST_ASSERT((sem = sem_open(semaphore_name, 0)) != SEM_FAILED);
	TEST_ASSERT(sem_unlink(semaphore_name) == 0);
	TEST_ASSERT(sem_close(sem) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test posix_semaphore_tests_api[] = {
	{ test_posix_semaphore_create_unlink, "Create Unlink" },
	{ test_posix_semaphore_open_close,    "Open Close"    },
	{ NULL,                               NULL            },
};
