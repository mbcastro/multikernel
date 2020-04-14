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

#include <nanvix/runtime/rmem.h>
#include <nanvix/ulib.h>
#include "../test.h"

/* Import definitions. */
extern void *nanvix_malloc(size_t size);
extern void nanvix_free(void *ptr);

/**
 * @brief Number of blocks for stress tests.
 */
#define NUM_BLOCKS 256

/**
 * @brief Number of pages for stress tests.
 */
#define NUM_PAGES 4

/**
 * @brief Magic number.
 */
const unsigned MAGIC = 0xdeadbeef;

/*============================================================================*
 * API Test: Read/Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Read/Write
 */
static void test_api_mem_read_write(void)
{
	unsigned *ptr;

	TEST_ASSERT((ptr = nanvix_malloc(sizeof(unsigned))) != RMEM_NULL);

	*ptr = MAGIC;

	/* Checksum. */
	TEST_ASSERT(*ptr == MAGIC);

	nanvix_free(ptr);
}

/*============================================================================*
 * API Test: Read/Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Read/Write
 */
static void test_api_mem_debug(void)
{
	unsigned *ptr;
	unsigned *ptr2;

	TEST_ASSERT((ptr = nanvix_malloc(sizeof(unsigned))) != RMEM_NULL);
	uprintf("ptr: %x\n", ptr);
	nanvix_free(ptr);
	TEST_ASSERT((ptr2 = nanvix_malloc(sizeof(unsigned))) != RMEM_NULL);
	uprintf("ptr: %x\n", ptr2);
	nanvix_free(ptr2);
}

/*============================================================================*
 * Stress Test: Consistency                                                   *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency
 */
static void test_stress_mem_consistency(void)
{
	int *numbers;

	TEST_ASSERT((numbers = nanvix_malloc(NUM_PAGES*PAGE_SIZE*sizeof(int))) != RMEM_NULL);

	for ( size_t i = 0; i < NUM_PAGES*PAGE_SIZE; i++)
	{
		numbers[i] = (int)i;
		TEST_ASSERT(numbers[i] == (int)i);
	}

	nanvix_free(numbers);
}

/*============================================================================*
 * Stress Test: Consistency 2-Step                                            *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency 2-Step
 */
static void test_stress_mem_consistency2(void)
{
	/* uprintf("Malloc....\n"); */
	int *buffer;
	TEST_ASSERT(buffer = nanvix_malloc(NUM_PAGES*PAGE_SIZE*sizeof(int)));
	/* uprintf("For....\n"); */
	for (size_t i = 0; i < NUM_PAGES*PAGE_SIZE; i++)
	{
		/* uprintf("Num %d\n", i); */
		buffer[i] = (int)i;
	}

	uprintf("Ok \n");
	for (size_t i = 0; i < NUM_PAGES*PAGE_SIZE; i++)
	{
		if (buffer[i] != (int)i)
		{
			uprintf("Comp %d %d %d\n", buffer[i], i, (int)i);
			TEST_ASSERT(buffer[i] == (int)i);
		}
	}
	nanvix_free(buffer);

}

/*============================================================================*
 * Stress Test: Consistency 2-Step Reverse                                    *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency 2-Step Reverse
 */
static void test_stress_mem_consistency2r(void)
{

	int *buffer;
	TEST_ASSERT(buffer = nanvix_malloc(NUM_PAGES*PAGE_SIZE*sizeof(int)));
	for (size_t i = 0; i < NUM_PAGES*PAGE_SIZE; i++)
		buffer[i] = (int)i;

	for (size_t i = 0; i < NUM_PAGES*PAGE_SIZE; i++)
		TEST_ASSERT(buffer[(NUM_PAGES*PAGE_SIZE-1)-i] == (int)((NUM_PAGES*PAGE_SIZE-1)-i));

	nanvix_free(buffer);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_mem_api[] = {
	/* { test_api_mem_read_write,         "memory read/write"          }, */
	{ test_api_mem_debug,     "consistency "               },
	/* { test_stress_mem_consistency,     "consistency "               }, */
	/* { test_stress_mem_consistency2,    "consistency 2-step"         }, */
	/* { test_stress_mem_consistency2r,   "consistency 2-step Rverse"  }, */
	{ NULL,                            NULL                         },
};
