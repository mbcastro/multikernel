/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
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
