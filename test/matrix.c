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

double data[RMEM_BLOCK_SIZE/sizeof(double)];

/**
 * @brief Matrix multiplication kernel.
 *
 */
static void kernel(int nclusters, int msize)
{
	for (int i = 0; i < msize; i++)
	{
		for (int j = 0; j < msize; j++)
		{
			memread(0, data, RMEM_BLOCK_SIZE);

			for (int k = 0; k < (RMEM_BLOCK_SIZE/sizeof(double)); k++)
				data[k] *= 1.321312523;

			memwrite(clusterid, data, RMEM_BLOCK_SIZE);
		}
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

	/* Invalid number of arguments. */
	if (argc != 4)
		return (-EINVAL);
	
	clusterid = arch_get_cluster_id();

	/* Retrieve paramenters. */
	int nclusters = 16;
	int msize = 100;

	/* Wait master IO cluster. */
	barrier_open(1);
	barrier_wait();

	kernel(nclusters, msize);

	/* Wait master IO cluster. */
	barrier_wait();

	/* House keeping. */
	barrier_close();

	return (EXIT_SUCCESS);
}
