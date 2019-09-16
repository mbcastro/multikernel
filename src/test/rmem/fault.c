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

/**
 * @brief Buffer size (in bytes).
 */
#define DATA_SIZE 256

/**
 * @brief Invalid alloc test flag.
 */
#define TEST_INV_ALLOC 0

/*============================================================================*
 * API Test: Invalid Write                                                    *
 *============================================================================*/

/**
 * @brief API Test: Invalid Write
 */
static void test_rmem_invalid_write(void)
{
	char buffer[DATA_SIZE];

	nanvix_memset(buffer, 1, DATA_SIZE);
	TEST_ASSERT(nanvix_rmemwrite(RMEM_SIZE, buffer, DATA_SIZE) < 0);
	TEST_ASSERT(nanvix_rmemwrite(RMEM_SIZE - DATA_SIZE/2, buffer, DATA_SIZE) < 0);
	TEST_ASSERT(nanvix_rmemwrite(RMEM_BLOCK_SIZE/2, buffer, RMEM_SIZE/RMEM_BLOCK_SIZE) < 0);
	TEST_ASSERT(nanvix_rmemwrite(0, buffer, RMEM_BLOCK_SIZE/2) < 0);
}

/*============================================================================*
 * API Test: Null Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Null Write
 */
static void test_rmem_null_write(void)
{
	TEST_ASSERT(nanvix_rmemwrite(0, NULL, DATA_SIZE) < 0);
}

/*============================================================================*
 * API Test: Invalid Write Size                                               *
 *============================================================================*/

/**
 * @brief API Test: Invalid Write Size
 */
static void test_rmem_invalid_write_size(void)
{
	char buffer[DATA_SIZE];

	nanvix_memset(buffer, 1, DATA_SIZE);
	TEST_ASSERT(nanvix_rmemwrite(0, buffer, RMEM_BLOCK_SIZE + 1) < 0);
	TEST_ASSERT(nanvix_rmemwrite(RMEM_BLOCK_SIZE/2, buffer, RMEM_SIZE/RMEM_BLOCK_SIZE) < 0);
	TEST_ASSERT(nanvix_rmemwrite(0, buffer, RMEM_BLOCK_SIZE/2) < 0);
}

/*============================================================================*
 * API Test: Invalid Read                                                     *
 *============================================================================*/

/**
 * @brief API Test: Invalid Read
 */
static void test_rmem_invalid_read(void)
{
	char buffer[DATA_SIZE];

	nanvix_memset(buffer, 1, DATA_SIZE);
	TEST_ASSERT(nanvix_rmemread(RMEM_SIZE, buffer, DATA_SIZE) < 0);
	TEST_ASSERT(nanvix_rmemread(RMEM_SIZE - DATA_SIZE/2, buffer, DATA_SIZE) < 0);
	TEST_ASSERT(nanvix_rmemread(RMEM_BLOCK_SIZE/2, buffer, RMEM_SIZE/RMEM_BLOCK_SIZE) < 0);
	TEST_ASSERT(nanvix_rmemread(0, buffer, RMEM_BLOCK_SIZE/2) < 0);
}

/*============================================================================*
 * API Test: Null Read                                                        *
 *============================================================================*/

/**
 * @brief API Test: Null Read
 */
static void test_rmem_null_read(void)
{
	TEST_ASSERT(nanvix_rmemread(0, NULL, DATA_SIZE) < 0);
}

/*============================================================================*
 * API Test: Invalid Read Size                                                *
 *============================================================================*/

/**
 * @brief API Test: Invalid Read Size
 */
static void test_rmem_invalid_read_size(void)
{
	char buffer[DATA_SIZE];

	nanvix_memset(buffer, 1, DATA_SIZE);
	TEST_ASSERT(nanvix_rmemread(0, buffer, RMEM_BLOCK_SIZE + 1) < 0);
	TEST_ASSERT(nanvix_rmemread(RMEM_BLOCK_SIZE/2, buffer, RMEM_SIZE/RMEM_BLOCK_SIZE) < 0);
	TEST_ASSERT(nanvix_rmemread(0, buffer, RMEM_BLOCK_SIZE/2) < 0);
}

/*============================================================================*
 * API Test: Invalid Alloc                                                    *
 *============================================================================*/

/**
 * @brief API Test: Invalid Alloc
 */
static void test_rmem_invalid_alloc(void)
{
	if (TEST_INV_ALLOC)
	{
	    for (int i = 0; i < RMEM_SIZE/RMEM_BLOCK_SIZE; i++)
	    {
	        TEST_ASSERT(nanvix_rmemalloc() == i);
	    }
	    TEST_ASSERT(nanvix_rmemalloc() < 0);
	}
}

/*============================================================================*
 * API Test: Invalid Free                                                     *
 *============================================================================*/

/**
 * @brief API Test: Invalid Free
 */
static void test_rmem_invalid_free(void)
{
	TEST_ASSERT(nanvix_rmemfree(-1) < 0);
	TEST_ASSERT(nanvix_rmemfree(RMEM_SIZE/RMEM_BLOCK_SIZE) < 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_fault[] = {
	{ test_rmem_invalid_write,      "invalid write"      },
	{ test_rmem_null_write,         "null write"         },
	{ test_rmem_invalid_write_size, "invalid write size" },
	{ test_rmem_invalid_read,       "invalid read"       },
	{ test_rmem_null_read,          "null read"          },
	{ test_rmem_invalid_read_size,  "invalid read size"  },
	{ test_rmem_invalid_free,       "invalid free"       },
	{ test_rmem_invalid_alloc,      "invalid alloc"      },
	{ NULL,                          NULL                },
};
