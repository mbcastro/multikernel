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
#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/servers/spawn.h>
#include <nanvix/ulib.h>
#include "benchmark.h"
#include "apps.h"

/**
 * @brief Ulibc is missing this.
 */
#define URAND_MAX (127773*16807 + 2836)

/**
 * @brief Define which struct to use.
 */
#define APPS_STRUCT apps

/**
 * @brief Number of trials.
 */
#ifndef __NTRIALS
#define NTRIALS 200
#endif

/**
 * @brief Number of pages.
 */
#ifndef __NUM_PAGES
#define NUM_PAGES (RMEM_SERVERS_NUM*(RMEM_NUM_BLOCKS - 1))
#endif

/**
 * @brief Indexer for 2D arrays.
 */
#define ARRAY2D(a,w,i,j) (a)[(i)*(w) + (j)]

/**
 * @brief Gets a random number from an interval.
 */
int random_number(int min, int max)
{
	int r;
	int range = 1 + max - min;
	int buckets = URAND_MAX/range;
	int limit = buckets*range;
	do
		r = urand();
	while (r >= limit);
	return min + (r/buckets);
}

static rpage_t raw_pages[NUM_PAGES];

/**
 * @brief Syntetic Benchmark
 */
int __main2(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	int rand_number;
	int page_value = 0;
	int page_strike;
	int sum = 0;
	int selection = -1;
    int skipped = 0;
    int access_time = 1;
	int total_ocurrences;
	int row_size;
	int column_size;
	int random_num = 0;
	int trials;
	usrand(9876);

	__runtime_setup(3);

		/* Allocate pages. */
		uprintf("[benchmark] allocating pages: %d\n", NUM_PAGES);
		for (int i = 0; i < NUM_PAGES; i++)
			uassert((raw_pages[i] = nanvix_rcache_alloc()) != 0);

		/* Run matrix. */
		uprintf("[benchmark] applying puts and gets\n");
		column_size = APPS_STRUCT.col[0];
		for (int j = 0; j < column_size; j++)
		{
			total_ocurrences = 0;
			trials = NTRIALS;


			/* Run trials on column. */
			for (int trial = 0; trial < trials; trial++, access_time++)
			{
				random_num = 0;
				random_num = urand()%APPS_STRUCT.size;

				/* Computer number of occurences in a column. */
				row_size = APPS_STRUCT.row[random_num];
				for (int i = 0; i < row_size; i++)
				{
					total_ocurrences += ARRAY2D(APPS_STRUCT.work[random_num], column_size, i, j);
				}

				rand_number = random_number(0, total_ocurrences);
				/* Probability Roulette */
				sum = 0;
				selection = -1;
				for (int i = 0; i < row_size; i++)
				{
					sum += ARRAY2D(APPS_STRUCT.work[random_num], column_size, i, j);

					/* If first elements of column are equal to zero. */
					if (rand_number == 0 && sum <= 0)
						continue;

					/* Roulette core. */
					if ((rand_number - sum) <= 0)
					{
						selection = i;
						break;
					}
				}
				/* If all column elements are equal to zero, skip. */
				if (selection == -1)
					continue;

				for (int i = 0; i <= random_num-1; i++)
					page_value += APPS_STRUCT.row[i]-1;
				page_value += APPS_STRUCT.pages_interval[random_num][selection].high;

				page_strike = APPS_STRUCT.pages_strike[random_num][selection];

				uprintf("[benchmark][heatmap] %d %d\n", access_time, page_value);
                uprintf("[benchmark] iteration %d of %d\n", access_time, column_size*NTRIALS);

				uassert(nanvix_rcache_get(raw_pages[page_value]-1) != NULL);
				uassert(nanvix_rcache_put(raw_pages[page_value]-1, page_strike) == 0);
				uprintf("[benchmark] Access %d\n", j);
	#ifdef USE_STIKES
				trials -= page_strike;
	#endif
				total_ocurrences = 0;
				page_value = 0;
			}
		}
		uprintf("[benchmark] %d lines skipped\n", skipped);

		/* Free pages. */
		uprintf("[benchmark] freeing pages: %d\n", NUM_PAGES);
		for (int i = 0; i < NUM_PAGES; i++)
			uassert(nanvix_rcache_free(raw_pages[i]) != 0);

		uassert(stdsync_fence() == 0);

	__runtime_cleanup();

	return (0);
}
