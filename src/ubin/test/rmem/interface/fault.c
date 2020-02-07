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

#define __NEED_RMEM_CACHE

#include <nanvix/runtime/rmem.h>
#include "../../test.h"

/**
 * @brief Dummy buffer used for tests.
 */
static char buffer[RMEM_BLOCK_SIZE];

/*============================================================================*
 * Fault Injection Test: Invalid Alloc                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Alloc
 */
static void test_rmem_interface_invalid_alloc(void)
{
	TEST_ASSERT((nanvix_ralloc(0)) == RMEM_NULL);
	TEST_ASSERT((nanvix_ralloc(RMEM_BLOCK_SIZE + 1)) == RMEM_NULL);
}

/*============================================================================*
 * Fault Injection Test: Invalid Free                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Free
 */
static void test_rmem_interface_invalid_free(void)
{
	TEST_ASSERT((nanvix_rfree(RMEM_NULL)) < 0);
	TEST_ASSERT((nanvix_rfree(NULL)) < 0);
	TEST_ASSERT((nanvix_rfree(&test_rmem_interface_invalid_free)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Read                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Read
 */
static void test_rmem_interface_invalid_read(void)
{
	void *ptr;

	TEST_ASSERT((ptr = nanvix_ralloc(RMEM_BLOCK_SIZE)) != RMEM_NULL);

		TEST_ASSERT(nanvix_rread(NULL, ptr, RMEM_BLOCK_SIZE) == 0);
		TEST_ASSERT(nanvix_rread(buffer, NULL, RMEM_BLOCK_SIZE) == 0);
		TEST_ASSERT(nanvix_rread(buffer, ptr, RMEM_BLOCK_SIZE + 1) == 0);

	TEST_ASSERT(nanvix_rfree(ptr) == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Write                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Write
 */
static void test_rmem_interface_invalid_write(void)
{
	void *ptr;

	TEST_ASSERT((ptr = nanvix_ralloc(RMEM_BLOCK_SIZE)) != RMEM_NULL);

		TEST_ASSERT(nanvix_rwrite(NULL, ptr, RMEM_BLOCK_SIZE) == 0);
		TEST_ASSERT(nanvix_rwrite(NULL, buffer, RMEM_BLOCK_SIZE) == 0);
		TEST_ASSERT(nanvix_rwrite(ptr, buffer, RMEM_BLOCK_SIZE + 1) == 0);

	TEST_ASSERT(nanvix_rfree(ptr) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_interface_fault[] = {
	{ test_rmem_interface_invalid_alloc, "invalid alloc" },
	{ test_rmem_interface_invalid_free,  "invalid free " },
	{ test_rmem_interface_invalid_read,  "invalid read " },
	{ test_rmem_interface_invalid_write, "invalid write" },
	{ NULL,                               NULL           },
};
