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
#define __NEED_RMEM_CACHE

#include <nanvix/runtime/rmem.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

/**
 * @brief Is the page cache initialized?
 */
static int initialized = 0;

/**
 * @brief Cache statistics.
 */
static struct
{
	unsigned nmisses; /**< Number of misses.     */
	unsigned nhits;   /**< Number of hits.       */
	unsigned nallocs; /**< Number of allocations */
} stats = { 0, 0, 0 };

/**
 * @brief Page cache.
 */
typedef struct
{
	rpage_t pgnum;
	char pages[RMEM_BLOCK_SIZE] ALIGN(PAGE_SIZE);
#ifdef __RMEM_CACHE_AGING
	uint32_t age;
#else
	int age;
#endif
	int ref_count;
} cache_slot;

static cache_slot cache_lines[RMEM_CACHE_SIZE] = {
	[0 ... ((RMEM_CACHE_SIZE) - 1)] = {.pgnum = RMEM_NULL, .age = 0, .ref_count = 0}
};

/**
 * @brief Discrete cache time.
 */
static unsigned cache_time = 0;

/**
 * @brief Default cache replacement policy.
 */
#ifndef __RMEM_CACHE_DEFAULT_REPLACEMENT
#define __RMEM_CACHE_DEFAULT_REPLACEMENT RMEM_CACHE_FIFO
#endif

/**
 * @brief Default cache write policy.
 */
#ifndef __RMEM_CACHE_DEFAULT_WRITE
#define __RMEM_CACHE_DEFAULT_WRITE RMEM_CACHE_WRITE_BACK
#endif

/**
 * @brief Cache replacement policy.
 */
static int cache_policy = __RMEM_CACHE_DEFAULT_REPLACEMENT;

/**
 * @brief Cache write policy.
 */
static int write_policy = __RMEM_CACHE_DEFAULT_WRITE;

/*============================================================================*
 * nanvix_rcache_clean()                                                      *
 *============================================================================*/

/**
 * @brief Cleans the cache.
 */
void nanvix_rcache_clean(void)
{
	for (int i = 0; i < RMEM_CACHE_LENGTH*RMEM_CACHE_BLOCK_SIZE; i++)
	{
		cache_lines[i].pgnum = RMEM_NULL;
		cache_lines[i].age = 0;
	}
}

/*============================================================================*
 * nanvix_rcache_page_search()                                                *
 *============================================================================*/

/**
 * @brief Searches for a page in the cache.
 *
 * @param pgnum Number of the target page.
 *
 * @returns Upon successful completion, the page index is
 * returned. Upon failure a negative error code is returned instead.
 */
static int nanvix_rcache_page_search(rpage_t pgnum)
{
	cache_time++;
	int pgnum_block;

	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		pgnum_block = (int)(pgnum) - (int)(cache_lines[i*RMEM_CACHE_BLOCK_SIZE].pgnum);
		if (pgnum_block >= 0 && pgnum_block < RMEM_CACHE_BLOCK_SIZE && cache_lines[i*RMEM_CACHE_BLOCK_SIZE].pgnum != RMEM_NULL)
		{
			for (int j = 0; j < RMEM_CACHE_BLOCK_SIZE; j++)
			{
				if (cache_lines[i*RMEM_CACHE_BLOCK_SIZE+j].pgnum == pgnum)
				{
					return (i*RMEM_CACHE_BLOCK_SIZE+j);
				}
			}
		}
	}

	return (-EFAULT);
}

static void nanvix_update_aging(rpage_t pgnum)
{
	int temp_age;
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		temp_age = cache_lines[i*RMEM_CACHE_BLOCK_SIZE].age;
		temp_age = (unsigned)(temp_age) >> 1;
		if (cache_lines[i*RMEM_CACHE_BLOCK_SIZE].pgnum == pgnum)
			temp_age = 1 << 31 | temp_age;
		cache_lines[i*RMEM_CACHE_BLOCK_SIZE].age = temp_age;
	}
}

/*============================================================================*
 * nanvix_rcache_age_update_lru()                                             *
 *============================================================================*/

/**
 * @brief Evicts pages from the cache based on the LRU replacement policy.
 *
 * @param pgnum Number of the target page.
 *
 * @returns Upon successful completion, the index of the page is
 * returned. Upon failure a negative error code is returned instead.
 */
static int nanvix_rcache_age_update_lru(rpage_t pgnum)
{
	int idx;

	cache_time++;

	if (cache_policy == RMEM_CACHE_LRU)
	{
		if ((idx = nanvix_rcache_page_search(pgnum)) < 0)
		    return (-EFAULT);

		cache_lines[idx].age += cache_time;
	} else if (cache_policy == RMEM_CACHE_AGING) {
		if ((idx = nanvix_rcache_page_search(pgnum)) < 0)
		    return (-EFAULT);

		nanvix_update_aging(pgnum);
	}

	return (0);
}

/*============================================================================*
 * nanvix_rcache_age_update()                                                 *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
static int nanvix_rcache_age_update(rpage_t pgnum)
{
	int idx;

	cache_time++;

	if (cache_policy == RMEM_CACHE_AGING) {
		nanvix_update_aging(pgnum);
	} else {
		if ((idx = nanvix_rcache_page_search(pgnum)) < 0)
			return (-EFAULT);

		cache_lines[idx].age = cache_time;
	}
	return 0;
}

/*============================================================================*
 * nanvix_rcache_fifo()                                                       *
 *============================================================================*/

/**
 * @brief Evicts pages from the cache based on the FIFO replacement policy.
 *
 * @param pgnum Number of the target page.
 *
 * @returns Upon successful completion, the index of the page is
 * returned. Upon failure a negative error code is returned instead.
 */
static int nanvix_rcache_fifo(void)
{
	int idx;
	int age;
	int min_age;

	cache_time++;

	/* Cache has space. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		if (cache_lines[i*RMEM_CACHE_BLOCK_SIZE].pgnum == RMEM_NULL)
		{
		    return (i*RMEM_CACHE_BLOCK_SIZE);
		}
	}

	/* No space. Make evict. */
	min_age = cache_lines[idx = 0].age;
	for (int i = 1; i < RMEM_CACHE_LENGTH; i++)
	{
		if ((age = cache_lines[i*RMEM_CACHE_BLOCK_SIZE].age) < min_age)
		{
		    idx = i*RMEM_CACHE_BLOCK_SIZE;
		    min_age = age;
		}
	}
	if (nanvix_rcache_flush(cache_lines[idx].pgnum) < 0)
		return (-EFAULT);

	return (idx);
}

/*============================================================================*
 * nanvix_rcache_lru()                                                        *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
static int nanvix_rcache_lru(void)
{
	return (nanvix_rcache_fifo());
}

/*============================================================================*
 * nanvix_rcache_lifo()                                                       *
 *============================================================================*/

/**
 * @brief Evicts pages from the cache based on the LIFO replacement policy.
 *
 * @param pgnum Number of the target page.
 *
 * @returns Upon successful completion, the index of the page is
 * returned. Upon failure a negative error code is returned instead.
 */
static int nanvix_rcache_lifo(void)
{
	int idx;
	int age;
	int max_age;

	cache_time++;

	/* Cache has space. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		if (cache_lines[i*RMEM_CACHE_BLOCK_SIZE].pgnum == RMEM_NULL)
		    return (i*RMEM_CACHE_BLOCK_SIZE);
	}

	/* No space. Make evict. */
	max_age = cache_lines[idx = 0].age;
	for (int i = 1; i < RMEM_CACHE_LENGTH; i++)
	{
		if ((age = cache_lines[i*RMEM_CACHE_BLOCK_SIZE].age) > max_age)
		{
		    idx = i*RMEM_CACHE_BLOCK_SIZE;
		    max_age = age;
		}
	}

	if (nanvix_rcache_flush(cache_lines[idx].pgnum) < 0)
		return (-EFAULT);

	return idx;
}

/*============================================================================*
 * nanvix_rcache_replacement_policies()                                       *
 *============================================================================*/

/**
 * @brief Selects the replacement policy function based on the
 * replacement policy number.
 *
 * @returns Upon successful completion, the free index of a page is
 * returned. Upon failure a negative error code is returned instead.
 */
static int nanvix_rcache_replacement_policies(void)
{
	if (cache_policy == RMEM_CACHE_FIFO)
		return (nanvix_rcache_fifo());

	if (cache_policy == RMEM_CACHE_LIFO)
		return (nanvix_rcache_lifo());

	return (nanvix_rcache_lru());
}

/*============================================================================*
 * nanvix_rcache_select_replacement_policy()                                  *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_select_replacement_policy(int num)
{
	cache_time++;

	switch (num)
	{
		case RMEM_CACHE_FIFO:
		case RMEM_CACHE_LIFO:
		case RMEM_CACHE_LRU:
			cache_policy = num;
			break;
		default:
			return (-EFAULT);
	}
	return (0);
}

/*============================================================================*
 * nanvix_rcache_select_write()                                               *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_select_write(int num)
{
	cache_time++;

	switch (num)
	{
		case RMEM_CACHE_WRITE_THROUGH:
		case RMEM_CACHE_WRITE_BACK:
			write_policy = num;
			break;
		default:
			return (-EFAULT);
	}
	return (0);
}

/*============================================================================*
 * nanvix_rcache_ralloc()                                                     *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
rpage_t nanvix_rcache_alloc(void)
{
	rpage_t pgnum;

	cache_time++;

	/* Forward allocation to remote memory. */
	if ((pgnum = nanvix_rmem_alloc()) == (rpage_t) -ENOMEM)
		return (RMEM_NULL);

	stats.nallocs++;
	return (pgnum);
}

/*============================================================================*
 * nanvix_rcache_flush()                                                      *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_flush(rpage_t pgnum)
{
	int err;
	int idx;
	int pgnum_block;
	int pgnum_abs;
	int idx_abs;

	cache_time++;

	/* Invalid page number. */
	if ((pgnum == RMEM_NULL) || (RMEM_BLOCK_NUM(pgnum) >= RMEM_NUM_BLOCKS))
		return (-EFAULT);

	/* Search for page in the cache. */
	if ((idx = nanvix_rcache_page_search(pgnum)) < 0)
		return (-EFAULT);

	pgnum_block = idx%RMEM_CACHE_BLOCK_SIZE;
	pgnum_abs = (int)(pgnum - pgnum_block);
	idx_abs = idx - pgnum_block;
	/* Write page back to remote memory. */
	for (int i = 0; i < RMEM_CACHE_BLOCK_SIZE; i++)
	{
		if ((err = nanvix_rmem_write((rpage_t)(pgnum_abs+i), cache_lines[idx_abs+i].pages)) < 0)
			return (err);
	}
#ifdef CACHE_DEBUG
	uprintf("[benchmark] %d misses, %d hits", stats.nmisses, stats.nhits);
#endif
	return (0);
}

/*============================================================================*
 * nanvix_rcache_free()                                                       *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_free(rpage_t pgnum)
{
	cache_time++;

	/* Invalid page number. */
	if ((pgnum == RMEM_NULL) || (RMEM_BLOCK_NUM(pgnum) >= RMEM_NUM_BLOCKS))
		return (-EFAULT);

	/* Check if target page is loaded into the cache. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		if (cache_lines[i].pgnum == pgnum)
		    cache_lines[i].pgnum = RMEM_NULL;
	}

	stats.nallocs--;
	return (nanvix_rmem_free(pgnum));
}

/*============================================================================*
 * nanvix_rcache_get()                                                        *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
void *nanvix_rcache_get(rpage_t pgnum)
{
	int err;
	int idx;

	cache_time++;

	/* Invalid page number. */
	if ((pgnum == RMEM_NULL) || (RMEM_BLOCK_NUM(pgnum) >= RMEM_NUM_BLOCKS))
		return (NULL);

	if ((idx = nanvix_rcache_page_search(pgnum)) >= 0)
	{
	    stats.nhits++;
		nanvix_rcache_age_update_lru(pgnum);
		cache_lines[idx].ref_count++;
		return (cache_lines[idx].pages);
	}

	stats.nmisses++;
	if ((idx = nanvix_rcache_replacement_policies()) < 0)
		return (NULL);
	/* Load page remote page. */
	for (int i = 0; i < RMEM_CACHE_BLOCK_SIZE; i++)
	{
		if ((err = nanvix_rmem_read((rpage_t)(pgnum+i), cache_lines[idx+i].pages)) < 0)
			return (NULL);
		cache_lines[idx+i].pgnum = (rpage_t)(pgnum+i);
	}

	cache_lines[idx].ref_count++;
	nanvix_rcache_age_update(pgnum);

#ifdef CACHE_DEBUG
	uprintf("[benchmark] %d misses, %d hits", stats.nmisses, stats.nhits);
#endif
	return (cache_lines[idx].pages);
}

/*============================================================================*
 * nanvix_rcache_put()                                                        *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_put(rpage_t pgnum, int strike)
{
	int idx;

	cache_time++;

	/* Invalid page number. */
	if ((pgnum == RMEM_NULL) || (RMEM_BLOCK_NUM(pgnum) >= RMEM_NUM_BLOCKS))
		return (-EFAULT);

	if ((idx = nanvix_rcache_page_search(pgnum)) < 0)
		return (-EFAULT);

	if (cache_policy == RMEM_CACHE_LRU)
		cache_lines[idx].age += strike;

	if (cache_lines[idx].ref_count <= 0)
		return (-EFAULT);

	if ((write_policy == RMEM_CACHE_WRITE_THROUGH) && (nanvix_rcache_flush(pgnum) < 0))
		return (-EFAULT);

	cache_lines[idx].ref_count--;

#ifdef CACHE_DEBUG
	uprintf("[benchmark] %d misses, %d hits", stats.nmisses, stats.nhits);
#endif
	return (0);
}

/*============================================================================*
 * nanvix_rcache_setup()                                                      *
 *============================================================================*/

/**
 * The nanvix_rcache_setup() function initializes the page cache.
 */
int __nanvix_rcache_setup(void)
{
	/* Page cache already initialized. */
	if (!initialized)
		return (0);

	/* Initialize page cache statistics. */
	stats.nmisses = 0;
	stats.nhits = 0;
	stats.nallocs = 0;

	/* Page cache lines. */
	for (int i = 0; i < RMEM_CACHE_SIZE; i++)
	{
		cache_lines[i].pgnum = RMEM_NULL;
		cache_lines[i].age = 0;
		cache_lines[i].ref_count = 0;
	}

	initialized = 1;

	return (0);
}
