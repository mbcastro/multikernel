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

#include <nanvix/vfs.h>
#include <nanvix/syscalls.h>

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
static void benchmark_vector(int nchunks, int k)
{
	double t1, t2;
	int chunksize = (BLOCK_SIZE/sizeof(float));
	float chunk[chunksize];

	t1 = tick();

	/* Initialize vector. */
	for (int i = 0; i < 1024; i++)
	{
		for (int j = 0; j < chunksize; j++)
			chunk[j] = 1;
		
		memwrite(chunk, k*1024 + i, BLOCK_SIZE);
	}

	/* Multiply. */
	for (int i = 0; i < 1024; i++)
	{
		memread(chunk, k*1024 + i, BLOCK_SIZE);

		for (int j = 0; j < chunksize; j++)
			chunk[j] *= 2.31;
		
		memwrite(chunk, k*1024 + i, BLOCK_SIZE);
	}
		
	t2 = tick();

	printf("[vector] time: %lfs\n", t2 - t1);
}

/**
 * @brief Scalar vector multiplication benchmark.
 */
int main(int argc, char **argv)
{
	/* Invalid number of arguments. */
	if (argc != 3)
	{
		printf("missing number of chunks\n");
		printf("Usage: vector <nchunks> <offset>\n");
		return (0);
	}

	/* Server */
	benchmark_vector(atoi(argv[1]), atoi(argv[2]));

	return (0);
}
