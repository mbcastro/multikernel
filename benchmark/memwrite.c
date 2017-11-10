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
	double total;              /* Maximum bandwidth. */
	char buffer[BLOCK_SIZE]; /* Buffer.            */

	srand(time(NULL));

	/* Write checksum. */
	for (int i = 0; i < BLOCK_SIZE; i++)
		buffer[i] = 1;

	total = 0;

	/* Write blocks to remote memory. */
	for (int k = 0; k < nwrites; k++)
	{
		int j;
		double t1, t2;

		j = rand()%nwrites;

		t1 = tick();
		
		memwrite(buffer, j*BLOCK_SIZE, BLOCK_SIZE);
		
		t2 = tick();

		total += t2 - t1;
	}

	printf("[memwrite] write bandwidth: %d bytes %lf seconds\n", (nwrites*BLOCK_SIZE), total);
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
