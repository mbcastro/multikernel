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
#include <omp.h>

/**
 * @brief Magic number used for checksum.
 */
#define MAGIC 0xdeadbeef

/**
 * @brief Number of benchmark iterations.
 */
#define NITERATIONS 1

/**
 * Minimum block size (in bytes).
 */
#define BLKSIZE_MIN 1

/**
 * Maximum block size (in bytes).
 */
#define BLKSIZE_MAX 512

/**
 * Block size increment (in bytes).
 */
#define BLKSIZE_STEP 1

/*
 *  * Timer residual error.
    */
double timer_error = 0;

/*
 *  * Gets the current timer value.
 *   */
double timer_get(void)
{
		return (omp_get_wtime());
}

/*
 *  * Computers the difference between two timers.
 *   */
double timer_diff(double t1, double t2)
{
		return (t2 - t1 - timer_error);
}

/*
 *  * Initializes the timer.
 *   */
void timer_init(void)
{
	  double start, end;
	    
	    start = timer_get();
		  end = timer_get();
		    
		    timer_error = (end - start);
}



/**
 * @brief Unit test server.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static void server(void)
{
	char data[BLKSIZE_MAX];

	memset(data, 0, BLKSIZE_MAX);

	timer_init();

	/* Benchmark. */
	for (int i = BLKSIZE_MIN; i <= BLKSIZE_MAX; i += BLKSIZE_STEP)
	{
		double start, end, total;

		total = 0;
		/* Run several experiments. */
		for (int j = 0; j < NITERATIONS; j++)
		{
			start = timer_get();
			nanvix_mailbox_receive(data, i);
			end = timer_get();
			total += timer_diff(start, end);
		}

		printf("unicast benchmark %d %lf\n", i, total);
	}
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
	char data[BLKSIZE_MAX];

	memset(data, 1, BLKSIZE_MAX);

	output = nanvix_mailbox_open("/cpu0");

	/* Benchmark. */
	for (int i = BLKSIZE_MIN; i <= BLKSIZE_MAX; i += BLKSIZE_STEP)
	{
		/* Run several experiments. */
		for (int j = 0; j < NITERATIONS; j++)
			nanvix_mailbox_send(output, data, i);
	}

printf("CLIENT DONE\n");
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

	nanvix_connector_init();

	/* Server */
	(!strcmp(argv[1], "--server")) ? 
		server() : client();

	return (EXIT_SUCCESS);
}

