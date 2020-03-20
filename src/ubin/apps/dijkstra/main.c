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

#include <nanvix/runtime/rmem.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>
#include <posix/stddef.h>

/**
 * @brief Number of nodes.
 */
#define NUM_NODES 100

/* Import definitions. */
extern void *nanvix_malloc(size_t size);
extern void nanvix_free(void *ptr);
extern int nanvix_dijkstra(int*, int, int);

/**
 * @brief Syntetic Benchmark
 */
int __main2(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	int *adj_matrix;
	usrand(9876);
	uint64_t time_alloc, time_dijkstra, time_free;

	__runtime_setup(0);

		/* Unblock spawner. */
		uassert(stdsync_fence() == 0);
		uprintf("[nanvix][benchmark] server alive");

		__runtime_setup(3);

		/* Allocate many blocks.*/
#ifndef NDEBUG
		uprintf("[apps][dijkstra] allocating matrix...");
#endif
		perf_start(0, PERF_CYCLES);

			uassert((adj_matrix = nanvix_malloc((NUM_NODES*NUM_NODES)*sizeof(int))) != NULL);
			for(int i=0; i<NUM_NODES; i++)
				for(int j=0; j<NUM_NODES; j++)
					adj_matrix[i*NUM_NODES+j] = urand()%10;
		perf_stop(0);
		time_alloc = perf_read(0);

		/* Dijkstra algorithm. */
#ifndef NDEBUG
		uprintf("[apps][dijkstra] exec...");
#endif
		perf_start(0, PERF_CYCLES);
			uassert(nanvix_dijkstra(adj_matrix, 0, 4) == 0);
		perf_stop(0);
		time_dijkstra = perf_read(0);

		/* Free all blocks. */
#ifndef NDEBUG
		uprintf("[apps][dijkstra] freeing matrix...");
#endif
		perf_start(0, PERF_CYCLES);
			nanvix_free(adj_matrix);
		perf_stop(0);
		time_free = perf_read(0);


#ifndef NDEBUG
		uprintf("[apps][dijkstra] alloc %l sort %l free %l",
#else
		uprintf("[apps][dijkstra] %l %l %l",
#endif
			time_alloc,
			time_dijkstra,
			time_free
		);

		nanvix_shutdown();

	__runtime_cleanup();

	return (0);
}
