/*
 * Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <mpi.h>

#include <nanvix/vfs.h>
#include <nanvix/syscalls.h>
#include <nanvix/ramdisk.h>

#define NR_RAMDISKS 4

/**
 * @brief Gets wall clock (in seconds).
 *
 * @returns Wall clock value.
 */
static double tick(void)
{
	struct timeval tp;

	gettimeofday(&tp, NULL);

	return ((double) tp.tv_sec + (double)tp.tv_usec*1.e-6 );
}

/**
 * @brief Memread benchmark.
 */
static void benchmark_memread(int nreads, int wset)
{
	int myid;
	int nprocs;
	double t1, t2;
	char block[BLOCK_SIZE];

	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	/* Invalid arguments. */
	if ((nprocs*wset) > (NR_RAMDISKS*(RAMDISK_SIZE/BLOCK_SIZE)))
		exit(-1);

	srand(time(NULL));

	MPI_Barrier(MPI_COMM_WORLD);

	/* Write blocks to remote memory. */
	t1 = tick();
	for (int i = 0; i < nreads; i++)
		memread(block, myid*wset + (rand()%wset), BLOCK_SIZE);
	t2 = tick();

	printf("[memread] read bandwidth: %lf MB/s\n", (nreads*BLOCK_SIZE)/(1024*1024*(t2 - t1)));
}

/**
 * @brief IPC library unit test
 */
int main(int argc, char **argv)
{
	/* Invalid number of arguments. */
	if (argc != 3)
	{
		printf("missing number of reads\n");
		printf("Usage: memread <nreads> <wset>\n");
		return (0);
	}

    MPI_Init(&argc, &argv);

	benchmark_memread(atoi(argv[1]), atoi(argv[2]));

    MPI_Finalize();

	return (0);
}
