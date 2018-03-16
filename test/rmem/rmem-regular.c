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

char data[RMEM_BLOCK_SIZE];

/**
 * @brief Remote memory unit test.
 */
int main(int argc, char **argv)
{
	int wmode = 0;
	int clusterid;
	long start, end, total;

	/* Invalid number of arguments. */
	if (argc != 2)
		return (-EINVAL);
	
	clusterid = arch_get_cluster_id();

	if (!strcmp(argv[1], "write"))
		wmode = 1;
	else if (!strcmp(argv[1], "read"))
		wmode = 0;
	else 
		wmode = clusterid%2;

	timer_init();

	start = timer_get();
	for (int i = 0; i < NACCESSES; i++)
	{
		if (wmode)
			memwrite(i, data, RMEM_BLOCK_SIZE);
		else
			memread(i, data, RMEM_BLOCK_SIZE);
	}
	end = timer_get();

	total = timer_diff(start, end);

	printf("cluster %3d: %.2lf MB/s (%d MB %.2lf s)\n", 
			clusterid,
			(NACCESSES*RMEM_BLOCK_SIZE/(1024*1024))/(total/1000000.0),
			(NACCESSES*RMEM_BLOCK_SIZE)/(1024*1024),
			total/1000000.0
	);

	return (EXIT_SUCCESS);
}
