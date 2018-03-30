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
#include <nanvix/hal.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <stdio.h>
#include "kernel.h"

static char data[RMEM_BLOCK_SIZE];

/**
 * @brief Remote memory unit test.
 */
int main(int argc, char **argv)
{
	int size;
	int clusterid;
	long start, end;
	long total_time;
	
	clusterid = k1_get_cluster_id();

	/* Retrieve paramenters. */
	assert(argc == 3);
	assert((size = atoi(argv[2])) <= RMEM_BLOCK_SIZE);

	k1_timer_init();

	/* Wait master IO cluster. */
	barrier_open(1);
	barrier_wait();

	for (int i = 0; i < NITERATIONS; i++)
	{
		start = k1_timer_get();

			memwrite(i, data, size);
		
		end = k1_timer_get();
		total_time = k1_timer_diff(start, end);

		if (clusterid != 0)
			continue;

		/* Warmup. */
		if (i == 0)
			continue;

		printf("%s;%d;%d;%ld\n",
				"write",
				atoi(argv[1]),
				size,
				total_time
		);
	}

	/* Wait master IO cluster. */
	barrier_wait();

	/* House keeping. */
	barrier_close();

	return (EXIT_SUCCESS);
}
