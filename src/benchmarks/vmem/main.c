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
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>
#include <nanvix/ulib.h>
#include "benchmark.h"

/**
 * @brief Number of pages.
 */
#ifndef __NUM_PAGES
#define NUM_PAGES (RMEM_SERVERS_NUM*(RMEM_NUM_PAGES - 1))
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
 * @brief Dummy buffer 1.
 */
static char buffer1[RMEM_BLOCK_SIZE];

/**
 * @brief Receive buffer.
 */
static char buffer2[RMEM_BLOCK_SIZE];

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
	void *pages[NUM_PAGES];
	uint64_t time_alloc, time_rw, time_free;

#ifndef __WORKLOAD_CUSTOM

	workload_build();

#endif /* !WORKLOAD_CUSTOM */

	__runtime_setup(0);

		/* Unblock spawner. */
		uassert(stdsync_fence() == 0);
		uprintf("[nanvix][benchmark] server alive");

		__runtime_setup(3);

		/* Allocate many blocks.*/
		uprintf("[nanvix][benchmark] allocating pages: %d", NUM_PAGES);
		perf_start(0, PERF_CYCLES);
		for (int i = 0; i < NUM_PAGES; i++)
			uassert((pages[i] = nanvix_vmem_alloc(1)) != NULL);
		perf_stop(0);
		time_alloc = perf_read(0);

		/* Read and write. */
		uprintf("[nanvix][benchmark] read and writing");
		perf_start(0, PERF_CYCLES);
		for (int i = 0; i < workload_size; i++)
		{
			if (work[i].page >= NUM_PAGES)
			{
				skipped++;
				continue;
			}

			umemset(buffer1, i + 1, RMEM_BLOCK_SIZE);
			umemset(buffer2, 0, RMEM_BLOCK_SIZE);

			uassert(nanvix_vmem_write(pages[work[i].page], buffer1, RMEM_BLOCK_SIZE) == RMEM_BLOCK_SIZE);

			uassert(nanvix_vmem_read(buffer2, pages[work[i].page], RMEM_BLOCK_SIZE) == RMEM_BLOCK_SIZE);
			uassert(umemcmp(buffer1, buffer2, RMEM_BLOCK_SIZE) == 0);
		}
		perf_stop(0);
		time_rw = perf_read(0);
		uprintf("[nanvix][benchmark] %d lines skipped", skipped);

		/* Free all blocks. */
		uprintf("[nanvix][benchmark] freeing pages: %d", NUM_PAGES);
		perf_start(0, PERF_CYCLES);
		for (int i = NUM_PAGES - 1; i >= 0; i--)
			uassert(nanvix_vmem_free(pages[i]) == 0);
		perf_stop(0);
		time_free = perf_read(0);

		uprintf("[nanvix][benchmark] alloc %l rw %l free %l", time_alloc, time_rw, time_free);

		nanvix_shutdown();

	__runtime_cleanup();

	return (0);
}
