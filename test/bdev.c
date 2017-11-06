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

#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>

#include <nanvix/klib.h>
#include <nanvix/ipc.h>
#include <nanvix/vfs.h>
#include <nanvix/syscalls.h>

/**
 * @brief Number of messages to exchange.
 */
#define NR_MESSAGES 128

double mysecond()
{
        struct timeval tp;

        gettimeofday(&tp, NULL);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

/**
 * @brief Unit test client.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static int client(void)
{
	double max;
	char buffer[BLOCK_SIZE];

	/* Write checksum. */
	for (int i = 0; i < BLOCK_SIZE; i++)
		buffer[i] = 1;

	memopen();

	for (int k = 0; k < 1024; k++)
	{
		double t1, t2, bandwidth;

		t1 = mysecond();
		
		memwrite(buffer, k*BLOCK_SIZE, BLOCK_SIZE);
		
		t2 = mysecond();

		bandwidth = BLOCK_SIZE/(1024*(t2-t1));
		if (bandwidth > max)
			max = bandwidth;
	}

	memclose();

	fprintf(stdout, "[info] [bdev.test] max bandwidth: %lf MB/s\n", max);

	return (NANVIX_SUCCESS);
}

/**
 * @brief IPC library unit test
 */
int main(int argc, char **argv)
{
	int ret;

	((void) argc);
	((void) argv);

	/* Server */
	ret = client();

	if (ret == NANVIX_SUCCESS)
		kprintf("bdev test passed");
	else
		kprintf("bdev test FAILED");

	return (NANVIX_SUCCESS);
}
