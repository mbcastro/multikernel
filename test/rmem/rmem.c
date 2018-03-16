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
 * @brief Number of access to remote memory.
 */
#define NACCESSES RMEM_SIZE/RMEM_BLOCK_SIZE

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
static long kernel_regular(const char *workload)
{
	long start, end;

	/* Write. */
	if (kernel_workload(workload))
	{
		start = timer_get();
			for (int i = 0; i < NACCESSES; i++)
				memwrite(i, data, RMEM_BLOCK_SIZE);
		end = timer_get();
	}

	/* Read. */
	else
	{
		start = timer_get();
			for (int i = 0; i < NACCESSES; i++)
				memread(i, data, RMEM_BLOCK_SIZE);
		end = timer_get();
	}

	return (timer_diff(start, end));
}

/**
 * @brief Irregular pattern kernel.
 *
 * @param workload Workload type.
 *
 * @brief Kernel time.
 */
static long kernel_irregular(const char *workload)
{
	long start, end;

	/* Write. */
	if (kernel_workload(workload))
	{
		start = timer_get();
			for (int i = 0; i < NACCESSES; i++)
				memwrite(i%(RMEM_SIZE/RMEM_BLOCK_SIZE), data, RMEM_BLOCK_SIZE);
		end = timer_get();
	}

	/* Read. */
	else
	{
		start = timer_get();
			for (int i = 0; i < NACCESSES; i++)
				memread(rand()%(RMEM_SIZE/RMEM_BLOCK_SIZE), data, RMEM_BLOCK_SIZE);
		end = timer_get();
	}

	return (timer_diff(start, end));
}

/**
 * @brief Remote memory unit test.
 */
int main(int argc, char **argv)
{
	long total;
	const char *pattern;
	const char *workload;

	/* Invalid number of arguments. */
	if (argc != 3)
		return (-EINVAL);
	
	clusterid = arch_get_cluster_id();

	/* Retrieve paramenters. */
	pattern = argv[1];
	workload = argv[2];

	timer_init();

	total = (!strcmp(pattern, "regular")) ? 
		kernel_regular(workload) : kernel_irregular(workload);

	printf("cluster %3d: %.2lf MB/s (%d MB %.2lf s)\n", 
			clusterid,
			(NACCESSES*RMEM_BLOCK_SIZE/(1024*1024))/(total/1000000.0),
			(NACCESSES*RMEM_BLOCK_SIZE)/(1024*1024),
			total/1000000.0
	);

	return (EXIT_SUCCESS);
}
