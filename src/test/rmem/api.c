/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <nanvix/servers/rmem.h>
#include <ulibc/stdio.h>
#include <ulibc/string.h>
#include "../test.h"

/*============================================================================*
 * API Test: Alloc                                                            *
 *============================================================================*/

/**
 * @brief API Test: Alloc
 */
static void test_rmem_alloc(void)
{
	TEST_ASSERT(nanvix_rmemalloc() == 0);
	TEST_ASSERT(nanvix_rmemalloc() == 1);
	TEST_ASSERT(nanvix_rmemalloc() == 2);
	TEST_ASSERT(nanvix_rmemalloc() == 3);
}

/*============================================================================*
 * API Test: Free                                                             *
 *============================================================================*/

/**
 * @brief API Test: Free
 */
static void test_rmem_free(void)
{
	TEST_ASSERT(nanvix_rmemfree(1) == 0);
	TEST_ASSERT(nanvix_rmemalloc() == 1);
	TEST_ASSERT(nanvix_rmemfree(0) == 0);
	TEST_ASSERT(nanvix_rmemfree(2) == 0);
	TEST_ASSERT(nanvix_rmemalloc() == 0);
	TEST_ASSERT(nanvix_rmemalloc() == 2);
	for (int i = 0; i < 4; i++)
	    TEST_ASSERT(nanvix_rmemfree(i) == 0);
}

/*============================================================================*
 * API Test: Read Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Read Write
 */
static void test_rmem_read_write(void)
{
	static char buffer[RMEM_BLOCK_SIZE];

	nanvix_memset(buffer, 1, RMEM_BLOCK_SIZE);
	nanvix_rmemalloc();
	TEST_ASSERT(nanvix_rmemwrite(0, buffer, RMEM_BLOCK_SIZE) == 0);

	nanvix_memset(buffer, 0, RMEM_BLOCK_SIZE);
	TEST_ASSERT(nanvix_rmemread(0, buffer, RMEM_BLOCK_SIZE) == 0);
	nanvix_rmemfree(0);

	/* Checksum. */
	for (int i = 0; i < RMEM_BLOCK_SIZE; i++)
		TEST_ASSERT(buffer[i] == 1);
}

/*============================================================================*/

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_api[] = {
	{ test_rmem_alloc,      "alloc"         },
	{ test_rmem_free,       "free"          },
	{ test_rmem_read_write, "read write"    },
	{ NULL,                  NULL           },
};
