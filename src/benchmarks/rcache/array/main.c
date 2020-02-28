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

#define __NEED_RMEM_CACHE

#include <nanvix/runtime/rmem.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/ulib.h>
#include <nanvix/ulib.h>
#include "benchmark.h"

/**
 * @brief Number of pages.
 */
#ifndef __NUM_PAGES
#define NUM_PAGES (RMEM_SERVERS_NUM*(RMEM_NUM_BLOCKS - 1))
#endif

#ifndef __WORKLOAD_CUSTOM

/**
 * @brief Workload Size.
 */
#define __WORKLOAD_SIZE 64

/*
 * Override number of pages.
 */
#undef NUM_PAGES
#define NUM_PAGES (__WORKLOAD_SIZE + 1)

/**
 * @brief Workload size.
 */
const int workload_size = __WORKLOAD_SIZE;

/**
 * @brief Workload.
 */
struct workload work[__WORKLOAD_SIZE];

/**
 * @brief Builds a workload.
 */
static void workload_build(void)
{
	for (int i = 0; i < __WORKLOAD_SIZE; ++i)
	{
		work[i].type = i%2;
		work[i].page = i%RMEM_CACHE_LENGTH;
	}
}

#endif /* !WORKLOAD_CUSTOM */

/**
 * @brief Syntetic Benchmark
 */
int __main2(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	int skipped = 0;
	static rpage_t raw_pages[NUM_PAGES];

#ifndef __WORKLOAD_CUSTOM

	workload_build();

#endif /* !WORKLOAD_CUSTOM */

	__runtime_setup(0);

		/* Unblock spawner. */
		uassert(stdsync_fence() == 0);
		uprintf("[nanvix][benchmark] server alive");

		__runtime_setup(3);

		uprintf("[nanvix][benchmark] allocating pages: %d", NUM_PAGES);
		for (int i = 0; i < NUM_PAGES; i++)
			uassert((raw_pages[i] = nanvix_rcache_alloc()) != 0);

		uprintf("[nanvix][benchmark] applying puts and gets");
		for (int i = 0; i < workload_size; i++)
		{
			if (work[i].page >= NUM_PAGES)
			{
				skipped++;
				continue;
			}

			uassert(nanvix_rcache_get(raw_pages[work[i].page]) != NULL);
			uassert(nanvix_rcache_put(raw_pages[work[i].page], 1) == 0);
		}
		uprintf("[nanvix][benchmark] %d lines skipped", skipped);

		uprintf("[nanvix][benchmark] freeing pages: %d", NUM_PAGES);
		for (int i = 0; i < NUM_PAGES; i++)
			uassert(nanvix_rcache_free(raw_pages[i]) == 0);

		nanvix_shutdown();

	__runtime_cleanup();

	return (0);
}
