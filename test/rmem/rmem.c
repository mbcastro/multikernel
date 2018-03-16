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
#include <stdio.h>
#include <string.h>

/**
 * @brief Number of access to remote memory.
 */
#define NACCESSES 30

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
static void kernel_regular(const char *workload)
{
	/* Write. */
	if (kernel_workload(workload))
	{
		for (int i = 0; i < NACCESSES; i++)
			memwrite(i, data, RMEM_BLOCK_SIZE);
	}

	/* Read. */
	else
	{
		for (int i = 0; i < NACCESSES; i++)
			memread(i, data, RMEM_BLOCK_SIZE);
	}
}

/**
 * @brief Irregular pattern kernel.
 *
 * @param workload Workload type.
 *
 * @brief Kernel time.
 */
static void kernel_irregular(const char *workload)
{
	/* Write. */
	if (kernel_workload(workload))
	{
		for (int i = 0; i < NACCESSES; i++)
			memwrite(i%(RMEM_SIZE/RMEM_BLOCK_SIZE), data, RMEM_BLOCK_SIZE);
	}

	/* Read. */
	else
	{
		for (int i = 0; i < NACCESSES; i++)
			memread(rand()%(RMEM_SIZE/RMEM_BLOCK_SIZE), data, RMEM_BLOCK_SIZE);
	}
}

/**
 * @brief Remote memory unit test.
 */
int main(int argc, char **argv)
{
	const char *pattern;
	const char *workload;

	/* Invalid number of arguments. */
	if (argc != 3)
		return (-EINVAL);
	
	clusterid = arch_get_cluster_id();

	/* Retrieve paramenters. */
	pattern = argv[1];
	workload = argv[2];

	/* Wait master IO cluster. */
	barrier_open();
	barrier_wait();

	if (!strcmp(pattern, "regular"))
		kernel_regular(workload);
	else
		kernel_irregular(workload);

	/* Wait master IO cluster. */
	barrier_wait();

	/* House keeping. */
	barrier_close();

	return (EXIT_SUCCESS);
}
