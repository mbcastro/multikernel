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

#define __NEED_RMEM_CLIENT

#include <nanvix/servers/rmem.h>
#include <ulibc/stdio.h>
#include <ulibc/string.h>
#include "../test.h"

/**
 * @brief Dummy buffer.
 */
char buffer[RMEM_BLOCK_SIZE];

/*============================================================================*
 * API Test: Invalid Free                                                     *
 *============================================================================*/

/**
 * @brief API Test: Invalid Free
 */
static void test_rmem_invalid_free(void)
{
	TEST_ASSERT(nanvix_rmem_free(RMEM_NULL) == -EINVAL);
	TEST_ASSERT(nanvix_rmem_free(RMEM_NUM_BLOCKS) == -EINVAL);
}

/*============================================================================*
 * API Test: Invalid Write                                                    *
 *============================================================================*/

/**
 * @brief API Test: Invalid Write
 */
static void test_rmem_invalid_write(void)
{
	rpage_t blknum;

	nanvix_memset(buffer, 1, RMEM_BLOCK_SIZE);

	/* Invalid block number. */
	TEST_ASSERT(nanvix_rmem_write(RMEM_NULL, buffer) == 0);
	TEST_ASSERT(nanvix_rmem_write(RMEM_NUM_BLOCKS, buffer) == 0);
	TEST_ASSERT(nanvix_rmem_write(RMEM_NUM_BLOCKS + 1, buffer) == 0);

	/* Invalid buffer. */
	TEST_ASSERT((blknum = nanvix_rmem_alloc()) != RMEM_NULL);
	TEST_ASSERT(nanvix_rmem_write(blknum, NULL) == 0);
	TEST_ASSERT(nanvix_rmem_free(blknum) == 0);
}

/*============================================================================*
 * API Test: Invalid Read                                                     *
 *============================================================================*/

/**
 * @brief API Test: Invalid Read
 */
static void test_rmem_invalid_read(void)
{
	rpage_t blknum;

	nanvix_memset(buffer, 1, RMEM_BLOCK_SIZE);

	/* Invalid block number. */
	TEST_ASSERT(nanvix_rmem_read(RMEM_NULL, buffer) == 0);
	TEST_ASSERT(nanvix_rmem_read(RMEM_NUM_BLOCKS, buffer) == 0);
	TEST_ASSERT(nanvix_rmem_read(RMEM_NUM_BLOCKS + 1, buffer) == 0);

	/* Invalid buffer. */
	TEST_ASSERT((blknum = nanvix_rmem_alloc()) != RMEM_NULL);
	TEST_ASSERT(nanvix_rmem_read(blknum, NULL) == 0);
	TEST_ASSERT(nanvix_rmem_free(blknum) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_fault[] = {
	{ test_rmem_invalid_free,  "invalid free"  },
	{ test_rmem_invalid_write, "invalid write" },
	{ test_rmem_invalid_read,  "invalid read"  },
	{ NULL,                     NULL           },
};
