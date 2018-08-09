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
#include <semaphore.h>
#include <string.h>

#include <nanvix/limits.h>
#include <nanvix/semaphore.h>

#include "test.h"

/*============================================================================*
 * Fault Injection Test: Invalid Create                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Create
 */
static void test_posix_semaphore_invalid_create(void)
{
	char buf[NANVIX_SEM_NAME_MAX + 1];

	memset(buf, 'a', NANVIX_SEM_NAME_MAX + 1);
	buf[NANVIX_SEM_NAME_MAX] = '\0';

	/* Create invalid semaphores. */
	TEST_ASSERT(sem_open(NULL, O_CREAT, 0, 0) == SEM_FAILED);
	TEST_ASSERT(sem_open(buf, O_CREAT, 0, 0) == SEM_FAILED);
	TEST_ASSERT(sem_open("cool-name", O_CREAT, 0, (SEM_MAX + 1)) == SEM_FAILED);
}

/*============================================================================*
 * Fault Injection Test: Bad Create                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Create 
 */
static void test_posix_semaphore_bad_create(void)
{
	TEST_ASSERT(sem_open("", O_CREAT, 0, 0) == SEM_FAILED);
}

/*============================================================================*
 * Fault Injection Test: Double Create                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Create 
 */
static void test_posix_semaphore_double_create(void)
{
	sem_t *sem;

	TEST_ASSERT((sem = sem_open("cool-name", O_CREAT, 0, 0)) != SEM_FAILED);
	TEST_ASSERT(sem_open("cool-name", O_CREAT, 0, 0) == SEM_FAILED);
	TEST_ASSERT(sem_open("cool-name", (O_CREAT | O_EXCL), 0, 0) == SEM_FAILED);
	TEST_ASSERT(sem_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Open                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Open
 */
static void test_posix_semaphore_invalid_open(void)
{
	char buf[NANVIX_SEM_NAME_MAX + 1];

	memset(buf, 'a', NANVIX_SEM_NAME_MAX + 1);
	buf[NANVIX_SEM_NAME_MAX] = '\0';

	/* Open invalid semaphores. */
	TEST_ASSERT(sem_open(NULL, 0) == SEM_FAILED);
	TEST_ASSERT(sem_open(buf, 0) == SEM_FAILED);
}

/*============================================================================*
 * Fault Injection Test: Bad Open                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Open 
 */
static void test_posix_semaphore_bad_open(void)
{
	TEST_ASSERT(sem_open("", 0) == SEM_FAILED);
	TEST_ASSERT(sem_open("cool-name", 0) == SEM_FAILED);
}

/*============================================================================*
 * Fault Injection Test: Invalid Unlink                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Unlink
 */
static void test_posix_semaphore_invalid_unlink(void)
{
	char buf[NANVIX_SEM_NAME_MAX + 1];

	memset(buf, 'a', NANVIX_SEM_NAME_MAX + 1);
	buf[NANVIX_SEM_NAME_MAX] = '\0';

	/* Unlink invalid semaphores. */
	TEST_ASSERT(sem_unlink(NULL) < 0);
	TEST_ASSERT(sem_unlink(buf) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Unlink                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Unlink 
 */
static void test_posix_semaphore_bad_unlink(void)
{
	TEST_ASSERT(sem_unlink("") < 0);
	TEST_ASSERT(sem_unlink("missing-name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Double Unlink                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Unlink 
 */
static void test_posix_semaphore_double_unlink(void)
{
	sem_t *sem;

	TEST_ASSERT((sem = sem_open("cool-name", O_CREAT, 0, 0)) != SEM_FAILED);
	TEST_ASSERT(sem_unlink("cool-name") == 0);
	TEST_ASSERT(sem_unlink("cool-name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Close                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Close
 */
static void test_posix_semaphore_invalid_close(void)
{
	TEST_ASSERT(sem_close(NULL) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Close                                            *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Close
 */
static void test_posix_semaphore_bad_close(void)
{
	sem_t sem;

	TEST_ASSERT(sem_close(&sem) < 0);
}

/*============================================================================*
 * Fault Injection Test: Double Close                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Close 
 */
static void test_posix_semaphore_double_close(void)
{
	sem_t *sem;

	TEST_ASSERT((sem = sem_open("cool-name", O_CREAT, 0, 0)) != SEM_FAILED);
	TEST_ASSERT(sem_close(sem) == 0);
	TEST_ASSERT(sem_close(sem) < 0);
	TEST_ASSERT(sem_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Post                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Post
 */
static void test_posix_semaphore_invalid_post(void)
{
	TEST_ASSERT(sem_post(NULL) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Post                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Post
 */
static void test_posix_semaphore_bad_post(void)
{
	sem_t sem;

	TEST_ASSERT(sem_post(&sem) < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Wait                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Wait
 */
static void test_posix_semaphore_invalid_wait(void)
{
	TEST_ASSERT(sem_wait(NULL) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Wait                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Wait
 */
static void test_posix_semaphore_bad_wait(void)
{
	sem_t sem;

	TEST_ASSERT(sem_wait(&sem) < 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test posix_semaphore_tests_fault[] = {
	{ test_posix_semaphore_invalid_create, "Invalid Create" },
	{ test_posix_semaphore_bad_create,     "Bad Create"     },
	{ test_posix_semaphore_double_create,  "Double Create"  },
	{ test_posix_semaphore_invalid_open,   "Invalid Open"   },
	{ test_posix_semaphore_bad_open,       "Bad Open"       },
	{ test_posix_semaphore_invalid_unlink, "Invalid Unlink" },
	{ test_posix_semaphore_bad_unlink,     "Bad Unlink"     },
	{ test_posix_semaphore_double_unlink,  "Double Unlink"  },
	{ test_posix_semaphore_invalid_close,  "Invalid Close"  },
	{ test_posix_semaphore_bad_close,      "Bad Close"      },
	{ test_posix_semaphore_double_close,   "Double Close"   },
	{ test_posix_semaphore_invalid_post,   "Invalid Post"   },
	{ test_posix_semaphore_bad_post,       "Bad Post"       },
	{ test_posix_semaphore_invalid_wait,   "Invalid Wait"   },
	{ test_posix_semaphore_bad_wait,       "Bad Wait"       },
	{ NULL,                                NULL             },
};