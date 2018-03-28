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
#include <stdio.h>
#include <string.h>

/**
 * @brief My cluster ID.
 */
int clusterid;

char data[RMEM_BLOCK_SIZE];

/**
 * @brief Gets workload.
 */
static int kernel_workload(const char *workload)
{
	int wmode;

	wmode = clusterid%2;

	if (!strcmp(workload, "write"))
		wmode = 1;
	else if (!strcmp(workload, "read"))
		wmode = 0;

	return (wmode);
}

/**
 * @brief Regular pattern kernel.
 *
 * @param workload Workload type.
 *
 * @returns Kernel time.
 */
static void kernel_regular(const char *workload, int naccesses)
{
	long start, end;
	double total_time;

	k1_timer_init();

	/* Write. */
	if (kernel_workload(workload))
	{
		for (int i = 0; i < naccesses; i++)
		{
			start = k1_timer_get();

				memwrite(i, data, RMEM_BLOCK_SIZE);
			
			end = k1_timer_get();
			total_time = k1_timer_diff(start, end);

			if (clusterid != 0)
				continue;

			/* Warmup. */
			if (i == 0)
				continue;

			printf("%s;%d;%d;%.2lf\n",
					"write",
					1,
					RMEM_BLOCK_SIZE,
					total_time
			);
		}
	}

	/* Read. */
	else
	{
		for (int i = 0; i < naccesses; i++)
			memread(i, data, RMEM_BLOCK_SIZE);
	}
}

/**
 * @brief Irregular pattern kernel.
 *
 * @param workload  Workload type.
 * @param naccesses Number of accesses.
 *
 * @brief Kernel time.
 */
static void kernel_irregular(const char *workload, int naccesses)
{
	/* Write. */
	if (kernel_workload(workload))
	{
		for (int i = 0; i < naccesses; i++)
			memwrite(i%(RMEM_SIZE/RMEM_BLOCK_SIZE), data, RMEM_BLOCK_SIZE);
	}

	/* Read. */
	else
	{
		for (int i = 0; i < naccesses; i++)
			memread(rand()%(RMEM_SIZE/RMEM_BLOCK_SIZE), data, RMEM_BLOCK_SIZE);
	}
}

/**
 * @brief Remote memory unit test.
 */
int main(int argc, char **argv)
{
	int naccesses;
	const char *pattern;
	const char *workload;
	
	clusterid = k1_get_cluster_id();

#ifdef DEBUG
	printf("cluster %d: spawned!\n", clusterid);
#endif

	/* Invalid number of arguments. */
	if (argc != 4)
		return (-EINVAL);

	/* Retrieve paramenters. */
	pattern = argv[1];
	workload = argv[2];
	naccesses = atoi(argv[3]);

#ifdef DEBUG
	printf("ccluster %d: alive!\n", clusterid);
#endif

	/* Wait master IO cluster. */
	barrier_open(1);
	barrier_wait();

	if (!strcmp(pattern, "regular"))
		kernel_regular(workload, naccesses);
	else
		kernel_irregular(workload, naccesses);

	/* Wait master IO cluster. */
	barrier_wait();

#ifdef DEBUG
	printf("cluster %d: done!\n", clusterid);
#endif

	/* House keeping. */
	barrier_close();

	return (EXIT_SUCCESS);
}
