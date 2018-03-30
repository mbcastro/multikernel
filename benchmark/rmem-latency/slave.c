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
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "kernel.h"

/**
 * @brief Local data block.
 */
static char data[RMEM_BLOCK_SIZE];

/*====================================================================*
 * Write Kernel                                                       *
 *====================================================================*/

/**
 * @brief Remote memory write unit test.
 */
static void kernel_write(int size, int nclusters, int clusterid)
{
	long start, end;
	long total_time;

	/* Benchmark. */
	for (int i = 0; i < NITERATIONS; i++)
	{
		start = k1_timer_get();

			memwrite(i, data, size);
		
		end = k1_timer_get();

		/* Do not profile. */
		if (clusterid != 0)
			continue;

		/* Warmup. */
		if (i == 0)
			continue;

		total_time = k1_timer_diff(start, end);

		printf("%s;%d;%d;%ld\n",
				"write",
				nclusters,
				size,
				total_time
		);
	}
}

/*====================================================================*
 * Read Kernel                                                        *
 *====================================================================*/

/**
 * @brief Remote memory read unit test.
 */
static void kernel_read(int size, int nclusters, int clusterid)
{
	long start, end;
	long total_time;

	/* Benchmark. */
	k1_timer_init();
	for (int i = 0; i < NITERATIONS; i++)
	{
		start = k1_timer_get();

			memread(i, data, size);
		
		end = k1_timer_get();

		/* Do not profile. */
		if (clusterid != 0)
			continue;

		/* Warmup. */
		if (i == 0)
			continue;

		total_time = k1_timer_diff(start, end);

		printf("%s;%d;%d;%ld\n",
				"read",
				nclusters,
				size,
				total_time
		);
	}
}

/*====================================================================*
 * main                                                               *
 *====================================================================*/

/**
 * @brief Remote memory unit test.
 */
int main(int argc, char **argv)
{
	int size;
	int nclusters;
	int clusterid;
	
	clusterid = k1_get_cluster_id();

	/* Retrieve parameters. */
	assert(argc == 4);
	assert((nclusters = atoi(argv[2])) > 0);
	assert((size = atoi(argv[3])) <= RMEM_BLOCK_SIZE);

	/*
	 * Touch data to initialize all pages
	 * and warmup D-cache.
	 */
	memset(data, clusterid, size);

	k1_timer_init();

	/* Wait master IO cluster. */
	barrier_open(1);
	barrier_wait();

	if (!strcmp(argv[1], "write"))
		kernel_write(size, nclusters, clusterid);
	else
		kernel_read(size, nclusters, clusterid);

	/* Wait master IO cluster. */
	barrier_wait();

	/* House keeping. */
	barrier_close();

	return (EXIT_SUCCESS);
}
