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

#include <stdio.h>
#include <string.h>
#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>
#include <nanvix/ipc.h>
#include <nanvix/perf.h>

/**
 * @brief Magic number used for checksum.
 */
#define MAGIC 0xdeadbeef

/**
 * @brief Number of benchmark iterations.
 */
#define NITERATIONS 10

/**
 * Minimum block size (in bytes).
 */
#define BLKSIZE_MIN 4

/**
 * Maximum block size (in bytes).
 */
#define BLKSIZE_MAX 4

/**
 * Block size increment (in bytes).
 */
#define BLKSIZE_STEP 1

/**
 * @brief Unit test server.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static void server(void)
{
	int output;
	int data;

	output = nanvix_mailbox_open("/cpu1");

	timer_init();

		double start, end, total;

		total = 0;
		/* Run several experiments. */
		for (int j = 0; j < NITERATIONS; j++)
		{
			start = timer_get();
			nanvix_mailbox_receive(&data, sizeof(int));
			end = timer_get();
			total += timer_diff(start, end);
		}

	printf("unicast benchmark %d bytes %lf s", NITERATIONS*sizeof(int), total);
}

/**
 * @brief Unit test client.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static void client(void)
{
	int output;
	int data;

	data = 1;

	output = nanvix_mailbox_open("/cpu0");

	/* Benchmark. */
		/* Run several experiments. */
		for (int j = 0; j < NITERATIONS; j++)
		{
			nanvix_mailbox_send(output, &data, sizeof(int));

		}
}

/**
 * @brief Mailbox unicasunicast benchmarkk
 */
int main(int argc, char **argv)
{
	/* Missing parameters. */
	if (argc < 2)
	{
		printf("missing parameters");
		printf("usage: mailbox-unicast.benchmark <mode>");
		printf("  --client Client mode.");
		printf("  --server Server mode.");

		return (0);
	}

	nanvix_noc_init(2);

	/* Server */
	(!strcmp(argv[1], "--server")) ? 
		server() : client();

	return (EXIT_SUCCESS);
}

