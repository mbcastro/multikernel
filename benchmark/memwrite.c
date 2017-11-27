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
#include <omp.h>

#include <nanvix/vfs.h>
#include <nanvix/syscalls.h>
#include <nanvix/ramdisk.h>

/**
 * @brief Number of messages to exchange.
 */
#define NR_MESSAGES 128

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
 * @brief Memwrite benchmark.
 */
static void benchmark_memwrite(int nwrites)
{
	char buffer[BLOCK_SIZE]; /* Buffer.            */
	double t1, t2;

	srand(time(NULL));

	/* Write checksum. */
	for (int i = 0; i < BLOCK_SIZE; i++)
		buffer[i] = 1;

	/* Write blocks to remote memory. */
	t1 = tick();
	#pragma omp parallel for
	for (int k = 0; k < 1024; k++)
	{
		int j;

		j = rand()%nwrites;

		memwrite(buffer, j*(RAMDISK_SIZE/BLOCK_SIZE), BLOCK_SIZE);
	}
	t2 = tick();

	printf("[memwrite] write bandwidth: %lf MB/s\n", (BLOCK_SIZE*1024)/(1024*1024*(t2 - t1)));
}

/**
 * @brief IPC library unit test
 */
int main(int argc, char **argv)
{
	/* Invalid number of arguments. */
	if (argc != 2)
	{
		printf("missing number of writes\n");
		printf("Usage: memwrite <nwrites>\n");
		return (0);
	}

	/* Server */
	benchmark_memwrite(atoi(argv[1]));

	return (0);
}
