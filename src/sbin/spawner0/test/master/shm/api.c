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
#include <fcntl.h>
#include <sys/mman.h>

#include <mppaipc.h>

#include <nanvix/limits.h>
#include <nanvix/syscalls.h>
#include <nanvix/mm.h>

#include "test.h"

/*==========================================================================*
 * API Test: Create Unlink                                                  *
 *==========================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_shm_create_unlink(void)
{
	int shm;
	char shm_name[SHM_NAME_MAX];

	/* Create and unlink shm. */
	sprintf(shm_name, "/shm");
	TEST_ASSERT((shm = shm_open(shm_name, O_CREAT, 0)) >= 0);
	TEST_ASSERT(shm_unlink(shm_name) == 0);
}

/*==========================================================================*
 * API Test: Create Unlink 2                                                *
 *==========================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_shm_create_unlink2(void)
{
	int shm;
	char shm_name[SHM_NAME_MAX];

	/* Create and unlink shm. */
	sprintf(shm_name, "/shm");
	TEST_ASSERT((shm = shm_open(shm_name, (O_CREAT | O_EXCL), 0)) >= 0);
	TEST_ASSERT(shm_unlink(shm_name) == 0);
}

/*==========================================================================*
 * API Test: Create Unlink 3                                                *
 *==========================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_shm_create_unlink3(void)
{
	int shm;
	char shm_name[SHM_NAME_MAX];

	/* Create and unlink shm. */
	sprintf(shm_name, "/shm");
	TEST_ASSERT((shm = shm_open(shm_name, O_CREAT, 0)) >= 0);
	TEST_ASSERT(shm_open(shm_name, (O_CREAT | O_EXCL), 0) < 0);
	TEST_ASSERT(shm_unlink(shm_name) == 0);
}

/*==========================================================================*
 * API Test: Create Unlink 4                                                *
 *==========================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_shm_create_unlink4(void)
{
	int shm1, shm2;
	char shm_name[SHM_NAME_MAX];

	/* Create and unlink shm. */
	sprintf(shm_name, "/shm");
	TEST_ASSERT((shm1 = shm_open(shm_name, (O_CREAT | O_EXCL), 0)) >= 0);
	TEST_ASSERT((shm2 = shm_open(shm_name, O_CREAT, 0)) >= 0);
	TEST_ASSERT(shm_unlink(shm_name) == 0);
	TEST_ASSERT(shm_unlink(shm_name) == 0);
}

/*==========================================================================*
 * API Test: Open Close                                                     *
 *==========================================================================*/

/**
 * @brief API Test: Open Close
 */
static void test_posix_shm_open_close(void)
{
	int shm1, shm2;
	char shm_name[SHM_NAME_MAX];

	/* Create and unlink shm. */
	sprintf(shm_name, "/shm");
	TEST_ASSERT((shm1 = shm_open(shm_name, O_CREAT, 0)) >= 0);
	TEST_ASSERT((shm2 = shm_open(shm_name, 0, 0)) >= 0);
	TEST_ASSERT(shm_unlink(shm_name) == 0);
	TEST_ASSERT(shm_unlink(shm_name) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test posix_shm_tests_api[] = {
	{ test_posix_shm_create_unlink,  "Create Unlink"   },
	{ test_posix_shm_create_unlink2, "Create Unlink 2" },
	{ test_posix_shm_create_unlink3, "Create Unlink 3" },
	{ test_posix_shm_create_unlink4, "Create Unlink 4" },
	{ test_posix_shm_open_close,     "Open Close"      },
	{ NULL,                          NULL              },
};
