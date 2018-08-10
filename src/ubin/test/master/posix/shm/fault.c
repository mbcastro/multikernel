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
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nanvix/mm.h>

#include "test.h"

/*============================================================================*
 * Fault Injection Test: Invalid Create                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Create
 */
static void test_posix_shm_invalid_create(void)
{
	char buf[SHM_NAME_MAX + 1];

	memset(buf, 'a', SHM_NAME_MAX + 1);
	buf[SHM_NAME_MAX] = '\0';

	/* Create invalid shms. */
	TEST_ASSERT(shm_open(NULL, O_CREAT, 0) < 0);
	TEST_ASSERT(shm_open(buf, O_CREAT, 0) < 0);
	TEST_ASSERT(shm_open(NULL, (O_CREAT | O_EXCL), 0) < 0);
	TEST_ASSERT(shm_open(buf, (O_CREAT | O_EXCL), 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Create                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Create 
 */
static void test_posix_shm_bad_create(void)
{
	TEST_ASSERT(shm_open("", O_CREAT, 0) < 0);
	TEST_ASSERT(shm_open("", (O_CREAT | O_EXCL), 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Double Create                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Create 
 */
static void test_posix_shm_double_create(void)
{
	int shm;

	TEST_ASSERT((shm = shm_open("cool-name", O_CREAT, 0)) >= 0);
	TEST_ASSERT(shm_open("cool-name", (O_CREAT | O_EXCL), 0) < 0);
	TEST_ASSERT(shm_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Open                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Open
 */
static void test_posix_shm_invalid_open(void)
{
	char buf[SHM_NAME_MAX + 1];

	memset(buf, 'a', SHM_NAME_MAX + 1);
	buf[SHM_NAME_MAX] = '\0';

	/* Open invalid shms. */
	TEST_ASSERT(shm_open(NULL, 0, 0) < 0);
	TEST_ASSERT(shm_open(buf, 0, 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Open                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Open 
 */
static void test_posix_shm_bad_open(void)
{
	TEST_ASSERT(shm_open("", 0, 0) < 0);
	TEST_ASSERT(shm_open("cool-name", 0, 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Unlink                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Unlink
 */
static void test_posix_shm_invalid_unlink(void)
{
	char buf[SHM_NAME_MAX + 1];

	memset(buf, 'a', SHM_NAME_MAX + 1);
	buf[SHM_NAME_MAX] = '\0';

	/* Unlink invalid shms. */
	TEST_ASSERT(shm_unlink(NULL) < 0);
	TEST_ASSERT(shm_unlink(buf) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Unlink                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Unlink 
 */
static void test_posix_shm_bad_unlink(void)
{
	TEST_ASSERT(shm_unlink("") < 0);
	TEST_ASSERT(shm_unlink("missing-name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Double Unlink                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Unlink 
 */
static void test_posix_shm_double_unlink(void)
{
	int shm;

	TEST_ASSERT((shm = shm_open("cool-name", O_CREAT, 0)) >= 0);
	TEST_ASSERT(shm_unlink("cool-name") == 0);
	TEST_ASSERT(shm_unlink("cool-name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Truncate                                     *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Truncate
 */
static void test_posix_shm_invalid_truncate(void)
{
	int shm;

	TEST_ASSERT((shm = shm_open("/shm", O_CREAT, O_RDWR)) >= 0);
	TEST_ASSERT(ftruncate(-1, REGION_SIZE) < 0);
	TEST_ASSERT(ftruncate(1000000, REGION_SIZE) < 0);
	TEST_ASSERT(ftruncate(shm, RMEM_SIZE + 1) < 0);
	TEST_ASSERT(shm_unlink("/shm") == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test posix_shm_tests_fault[] = {
	{ test_posix_shm_invalid_create,   "Invalid Create"   },
	{ test_posix_shm_bad_create,       "Bad Create"       },
	{ test_posix_shm_double_create,    "Double Create"    },
	{ test_posix_shm_invalid_open,     "Invalid Open"     },
	{ test_posix_shm_bad_open,         "Bad Open"         },
	{ test_posix_shm_invalid_unlink,   "Invalid Unlink"   },
	{ test_posix_shm_bad_unlink,       "Bad Unlink"       },
	{ test_posix_shm_double_unlink,    "Double Unlink"    },
	{ test_posix_shm_invalid_truncate, "Invalid Truncate" },
	{ NULL,                            NULL               },
};
