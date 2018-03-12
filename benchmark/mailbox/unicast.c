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
#include <stdint.h>
#include <string.h>
#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>
#include <nanvix/ipc.h>

/**
 * @brief Magic number used for checksum.
 */
#define MAGIC 0xdeadbeef

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

#define MPPA_FREQUENCY 400

/*
 *  * Timer residual error.
 *   */
uint64_t timer_error = 0;

extern uint64_t k1_io_read64(unsigned addr);

/*
 *  * Gets the current timer value.
 *   */
uint64_t timer_get(void)
{
		return (k1_io_read64(0x70084040)/MPPA_FREQUENCY);
}

/*
 *  * Computers the difference between two timers.
 *   */
uint64_t timer_diff(uint64_t t1, uint64_t t2)
{
		return (t2 - t1 - timer_error);
}

/*
 *  * Initializes the timer.
 *   */
void timer_init(void)
{
	  uint64_t start, end;
	    
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
static int server(void)
{
	char data[BLKSIZE_MAX];

	memset(data, 0, BLKSIZE_MAX)

	timer_init();

	/* Benchmark. */
	for (int i = BLKSIZE_MIN; i <= BLKSIZE_MAX; BLKSIZE_STEP)
	{
		uint64_t start;
		uint64_t end;
		uint64_t total;
		
		total = 0;

		/* Run several experiments. */
		for (int j = 0; j < NITERATIONS; j++)
		{

			start = timer_get();
			nanvix_mailbox_send(output, data, i);
			end = timer_get;

			total += timer_diff(start, end);
		}

		printf("unicast %d bytes %" PRIu64 " s\n", i, total);
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

	memset(data, 1, BLKSIZE_MAX)

	output = nanvix_mailbox_open("/cpu0");

	/* Benchmark. */
	for (int i = BLKSIZE_MIN; i <= BLKSIZE_MAX; BLKSIZE_STEP)
	{
		/* Run several experiments. */
		for (int j = 0; j < NITERATIONS; j++)
			nanvix_mailbox_send(output, data, i);
	}
}

/**
 * @brief Mailbox unicasunicast benchmarkk
 */
int main(int argc, char **argv)
{
	int ret;

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
	ret = (!strcmp(argv[1], "--server")) ? 
		server() : client();

	return (EXIT_SUCCESS);
}

