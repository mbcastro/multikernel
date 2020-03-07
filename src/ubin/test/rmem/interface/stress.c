/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
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

#define __NEED_RMEM_MANAGER

#include <nanvix/runtime/rmem.h>
#include <nanvix/ulib.h>
#include "../../test.h"

/**
 * @brief Verbose tests?
 */
#define __VERBOSE_TESTS 0

/**
 * @brief Number of blocks to allocate.
 */
#define NUM_BLOCKS 32

/**
 * @brief Dummy buffer 1.
 */
static char buffer1[RMEM_BLOCK_SIZE];

/**
 * @brief Receive buffer.
 */
static char buffer2[RMEM_BLOCK_SIZE];

/*============================================================================*
 * Stress Test: Alloc/Free Sequential                                         *
 *============================================================================*/

/**
 * @brief Stress Test: Alloc/Free Sequential
 */
static void test_rmem_interface_alloc_free_sequential(void)
{
	void *blks[NUM_BLOCKS];

	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		TEST_ASSERT((blks[i] = nanvix_vmem_alloc(1)) != NULL);
		#if (__VERBOSE_TESTS)
			uprintf("ralloc() blknum=%d", blks[i]);
		#endif
	}
	for (int i = NUM_BLOCKS - 1; i >= 0; i--)
	{
		#if (__VERBOSE_TESTS)
			uprintf("rfree()  blknum=%d", blks[i]);
		#endif
		TEST_ASSERT(nanvix_vmem_free(blks[i]) == 0);
	}
}

/*============================================================================*
 * Stress Test: Read/Write Sequential                                         *
 *============================================================================*/

/**
 * @brief Stress Test: Read/Write Sequential
 */
static void test_rmem_interface_read_write_sequential(void)
{
	void *blks[NUM_BLOCKS];

	/* Allocate many blocks.*/
	for (int i = 0; i < NUM_BLOCKS; i++)
		TEST_ASSERT((blks[i] = nanvix_vmem_alloc(1)) != NULL);

	/* Read and write. */
	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		umemset(buffer1, i + 1, RMEM_BLOCK_SIZE);
		umemset(buffer2, 0, RMEM_BLOCK_SIZE);

		#if (__VERBOSE_TESTS)
			uprintf("rwrite() blknum=%d", blks[i]);
		#endif
		TEST_ASSERT(nanvix_vmem_write(blks[i], buffer1, RMEM_BLOCK_SIZE) == RMEM_BLOCK_SIZE);

		#if (__VERBOSE_TESTS)
			uprintf("rread()  blknum=%d", blks[i]);
		#endif
		TEST_ASSERT(nanvix_vmem_read(buffer2, blks[i], RMEM_BLOCK_SIZE) == RMEM_BLOCK_SIZE);
		TEST_ASSERT(umemcmp(buffer1, buffer2, RMEM_BLOCK_SIZE) == 0);

	}

	/* Free all blocks. */
	for (int i = NUM_BLOCKS - 1; i >= 0; i--)
		TEST_ASSERT(nanvix_vmem_free(blks[i]) == 0);
}

/*============================================================================*
 * Test Driver Table                                                          *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_interface_stress[] = {
	{ test_rmem_interface_alloc_free_sequential,  "alloc/free sequential " },
	{ test_rmem_interface_read_write_sequential,  "read/write sequential " },
	{ NULL,                                       NULL                     },
};
