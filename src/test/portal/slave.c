/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <string.h>
#include "kernel.h"

/**
 * @brief Cluster ID.
 */
static int clusterid;

/*===================================================================*
 * Write Kernel                                                      *
 *===================================================================*/

/**
 * @brief Buffer.
 */
static char buffer[MAX_BUFFER_SIZE];

/**
 * @brief Benchmarks write operations on a portal connector.
 */
static void kernel_write(int size, int nclusters)
{
	long t[2];
	long total_time;
	int barrier;
	int outportal;

	outportal = portal_open("/portal1");
	barrier = barrier_open(nclusters);

	/*
	 * Touch data to initialize all pages
	 * and warmup D-cache.
	 */
	memset(buffer, clusterid, size);

	/*
	 * Benchmark. First iteration is
	 * used to warmup resources.
	 */
	k1_timer_init();
	for (int i = 0; i <= NITERATIONS; i++)
	{
		/*
		 * Force cclusters to start
		 * all together.
		 */
		barrier_wait(barrier);

		t[0] = k1_timer_get();

		portal_write(outportal, buffer, size);

		/*
		 * Wait for other cclusters to
		 * complete the write operation.
		 */
		barrier_wait(barrier);
		t[1] = k1_timer_get();

		if (i == 0)
			continue;

		total_time = k1_timer_diff(t[0], t[1]);
		printf("%s;%d;%d;%ld\n",
			"write",
			clusterid,
			size,
			total_time
		);
	}

	/* House keeping. */
	barrier_close(barrier);
	portal_close(outportal);
}

/*===================================================================*
 * main                                                              *
 *===================================================================*/

/**
 * @brief Buffer.
 */
static char buffer[MAX_BUFFER_SIZE];

/**
 * @brief Benchmarks write operations on a portal connector.
 */
int main(int argc, char **argv)
{
	int size;        /* Write size.         */
	int nclusters;

	clusterid = hal_get_cluster_id();

	/* Retrieve parameters. */
	assert(argc == 4);
	assert((nclusters = atoi(argv[2])) > 0);
	assert((size = atoi(argv[3])) <= MAX_BUFFER_SIZE);

	kernel_write(size, nclusters);

	return (EXIT_SUCCESS);
}
