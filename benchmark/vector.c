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
 * @brief Scalar vector multiplication.
 */
static void benchmark_vector(int nprocs, int myrank)
{
	double sum;
	int i0, in;
	int nchunks;
	double t1, t2;
	int chunksize;

	chunksize = BLOCK_SIZE/sizeof(float);

	nchunks = NR_RAMDISKS*(RAMDISK_SIZE/BLOCK_SIZE);

	i0 = myrank*(nchunks/nprocs);
	in = (myrank + 1)*(nchunks/nprocs);

	sum = 0;

	/* Initialize vector. */
	for (int i = i0; i < in; i++)
	{
		double chunk[chunksize];

		for (int j = 0; j < chunksize; j++)
			chunk[j] = 1;

		t1 = tick();
			memwrite(chunk, i, BLOCK_SIZE);
		t2 = tick();

		sum += t2 - t1;
	}

	fprintf(stderr, "[vector] process %d: time %lf s %d bytes\n", myrank, sum, (in - i0)*BLOCK_SIZE);

	MPI_Barrier(MPI_COMM_WORLD);

	sum = 0;

	/* Multiply. */
	for (int i = i0; i < in; i++)
	{
		double chunk[chunksize];

		t1 = tick();
			memread(chunk, i, BLOCK_SIZE);
		t2 = tick();

		sum += t2 -t1;

		for (int j = 0; j < chunksize; j++)
			chunk[j] *= 2.31;
		
		t1 = tick();
			memwrite(chunk, i, BLOCK_SIZE);
		t2 = tick();

		sum += t2 -t1;
	}

	fprintf(stderr, "[vector] process %d: time %lf s %d bytes\n", myrank, sum, 2*(in - i0)*BLOCK_SIZE);
}

/**
 * @brief Scalar vector multiplication benchmark.
 */
int main(int argc, char** argv)
{
	int nprocs;
	int myrank;

    MPI_Init(&argc, &argv);

    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);


	fprintf(stderr, "hello from %d\n", myrank);

	benchmark_vector(nprocs, myrank);

    // Finalize the MPI environment.
    MPI_Finalize();
}
