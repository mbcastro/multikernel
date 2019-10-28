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

#define __NEED_RMEM_CACHE

#include <nanvix/runtime/rmem.h>
#include <nanvix/ulib.h>
#include "../../test.h"

/*============================================================================*
 * API Test: Alloc Free                                                       *
 *============================================================================*/

/**
 * @brief API Test: Alloc Free
 */
static void test_rmem_rcache_alloc_free(void)
{
	rpage_t page_num;
	rpage_t page_num1;

	TEST_ASSERT((page_num = nanvix_rcache_alloc()) != RMEM_NULL);
	TEST_ASSERT((page_num1 = nanvix_rcache_alloc()) != RMEM_NULL);
	TEST_ASSERT(nanvix_rcache_free(page_num1) == 0);
	TEST_ASSERT(nanvix_rcache_free(page_num) == 0);
}

/*============================================================================*
 * API Test: Cache Put Write                                                  *
 *============================================================================*/

/**
 * @brief API Test: Cache Put Write
 */
static void test_rmem_rcache_put_write(void)
{
	rpage_t page_num;
	static char *cache_data;

	TEST_ASSERT((page_num = nanvix_rcache_alloc()) != RMEM_NULL);
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num)) != NULL);
	umemset(cache_data, 1, RMEM_BLOCK_SIZE);

	nanvix_rcache_select_write(RMEM_CACHE_WRITE_BACK);
	TEST_ASSERT(nanvix_rcache_put(page_num) == 0);
	TEST_ASSERT(nanvix_rcache_put(page_num) < 0);

	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num)) != NULL);

	nanvix_rcache_select_write(RMEM_CACHE_WRITE_THROUGH);
	TEST_ASSERT(nanvix_rcache_put(page_num) == 0);
	TEST_ASSERT(nanvix_rcache_put(page_num) < 0);

	TEST_ASSERT(nanvix_rcache_free(page_num) == 0);
}

/*============================================================================*
 * API Test: Cache Get Flush                                                  *
 *============================================================================*/

/**
 * @brief API Test: Cache Get Flush
 */
static void test_rmem_rcache_get_flush(void)
{
	rpage_t page_num[5];
	static char *cache_data;

	for (int i = 0; i < 5; i++)
	{
		TEST_ASSERT((page_num[i] = nanvix_rcache_alloc()) != RMEM_NULL);
		TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[i])) != NULL);
		umemset(cache_data, i, RMEM_BLOCK_SIZE);
	}

	for (int i = 0; i < 5; i++)
		TEST_ASSERT(nanvix_rcache_flush(page_num[i]) == 0);

	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[3])) != NULL);

	/* Checksum */
	for (int i = 0; i < RMEM_BLOCK_SIZE; i++)
		TEST_ASSERT(cache_data[i] == 3);

	for (int i = 0; i < 5; i++)
		TEST_ASSERT(nanvix_rcache_free(page_num[i]) == 0);
}

/*============================================================================*
 * API Test: Cache FiFo                                                       *
 *============================================================================*/

/**
 * @brief API Test: Cache FiFo
 */
static void test_rmem_rcache_fifo(void)
{
	rpage_t page_num[6];
	static char *cache_data;

	nanvix_rcache_select_replacement_policy(RMEM_CACHE_FIFO);

	for (int i = 0; i < 5; i++)
	{
		TEST_ASSERT((page_num[i] = nanvix_rcache_alloc()) != RMEM_NULL);
		TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[i])) != NULL);
		umemset(cache_data, i, RMEM_BLOCK_SIZE);
	}

	for (int i = 0; i < 5; i++)
		TEST_ASSERT(nanvix_rcache_flush(page_num[i]) == 0);

	/* Eviction will occur */
	TEST_ASSERT((page_num[5] = nanvix_rcache_alloc()) != RMEM_NULL);
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[5])) != NULL);

	/* Checksum */
	for (int i = 0; i < RMEM_BLOCK_SIZE; i++)
		TEST_ASSERT(cache_data[i] == 0);

	/* Check if the page was evicted */
	TEST_ASSERT(nanvix_rcache_flush(page_num[0]) < 0);

	/* Another eviction will occur */
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[0])) != NULL);

	/* Checksum */
	for (int i = 0; i < RMEM_BLOCK_SIZE; i++)
		TEST_ASSERT(cache_data[i] == 0);

	/* Check if the page was evicted */
	TEST_ASSERT(nanvix_rcache_flush(page_num[1]) < 0);

	for (int i = 0; i < 5; i++)
		TEST_ASSERT(nanvix_rcache_free(page_num[i]) == 0);
	TEST_ASSERT(nanvix_rcache_free(page_num[5]) == 0);
}

/*============================================================================*
 * API Test: Cache LiFo                                                       *
 *============================================================================*/

/**
 * @brief API Test: Cache LiFo
 */
static void test_rmem_rcache_lifo(void)
{
	rpage_t page_num[6];
	static char *cache_data;

	nanvix_rcache_select_replacement_policy(RMEM_CACHE_LIFO);

	for (int i = 0; i < 5; i++)
	{
		TEST_ASSERT((page_num[i] = nanvix_rcache_alloc()) != RMEM_NULL);
		TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[i])) != NULL);
		umemset(cache_data, i, RMEM_BLOCK_SIZE);
	}

	for (int i = 0; i < 5; i++)
		TEST_ASSERT(nanvix_rcache_flush(page_num[i]) == 0);

	/* Eviction will occur */
	TEST_ASSERT((page_num[6] = nanvix_rcache_alloc()) != RMEM_NULL);
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[6])) != NULL);

	/* Checksum */
	for (int i = 0; i < RMEM_BLOCK_SIZE; i++)
		TEST_ASSERT(cache_data[i] == 0);

	/* Check if the page was evicted */
	TEST_ASSERT(nanvix_rcache_flush(page_num[4]) < 0);

	/* Another eviction will occur */
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[4])) != NULL);

	/* Checksum */
	for (int i = 0; i < RMEM_BLOCK_SIZE; i++)
		TEST_ASSERT(cache_data[i] == 4);

	/* Check if the page was evicted */
	TEST_ASSERT(nanvix_rcache_flush(page_num[5]) < 0);

	for (int i = 0; i < 5; i++)
		TEST_ASSERT(nanvix_rcache_free(page_num[i]) == 0);
	TEST_ASSERT(nanvix_rcache_free(page_num[5]) == 0);
}

/*============================================================================*
 * API Test: Cache lru                                                        *
 *============================================================================*/

/**
 * @brief API Test: Cache lru
 */
static void test_rmem_rcache_lru(void)
{
	rpage_t page_num[6];
	static char *cache_data;

	nanvix_rcache_select_replacement_policy(RMEM_CACHE_LRU);

	for (int i = 0; i < 5; i++)
	{
		TEST_ASSERT((page_num[i] = nanvix_rcache_alloc()) != RMEM_NULL);
		TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[i])) != NULL);
		umemset(cache_data, i, RMEM_BLOCK_SIZE);
	}

	for (int i = 0; i < 5; i++)
		TEST_ASSERT(nanvix_rcache_flush(page_num[i]) == 0);

	/* Eviction will occur */
	TEST_ASSERT((page_num[6] = nanvix_rcache_alloc()) != RMEM_NULL);
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[6])) != NULL);

	/* Checksum */
	for (int i = 0; i < RMEM_BLOCK_SIZE; i++)
		TEST_ASSERT(cache_data[i] == 0);

	/* Check if the page was evicted */
	TEST_ASSERT(nanvix_rcache_flush(page_num[0]) < 0);

	/* Another access on page 1 */
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[1])) != NULL);

	/* Checksum */
	for (int i = 0; i < RMEM_BLOCK_SIZE; i++)
		TEST_ASSERT(cache_data[i] == 1);

	/* Another eviction will occur */
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[0])) != NULL);

	/* Check if the page was evicted */
	TEST_ASSERT(nanvix_rcache_flush(page_num[2]) < 0);

	for (int i = 0; i < 5; i++)
		TEST_ASSERT(nanvix_rcache_free(page_num[i]) == 0);
	TEST_ASSERT(nanvix_rcache_free(page_num[5]) == 0);
}

/*============================================================================*
 * Test Driver Table                                                          *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_cache_api[] = {
	{ test_rmem_rcache_alloc_free, "alloc free" },
	{ test_rmem_rcache_put_write,  "put write"  },
	{ test_rmem_rcache_get_flush,  "get flush"  },
	{ test_rmem_rcache_fifo,       "fifo"       },
	{ test_rmem_rcache_lifo,       "lifo"       },
	{ test_rmem_rcache_lru,        "lru"        },
	{ NULL,                         NULL        },
};
