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

/**
 * @brief Magic number.
 */
const unsigned MAGIC = 0xdeadbeef;

extern void *nanvix_malloc(size_t size);
extern void nanvix_free(void *ptr);

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

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_mem_api[] = {
	{ test_api_mem_read_write, "memory read/write" },
	{ NULL,                     NULL               },
};
