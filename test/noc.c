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

#include <stdio.h>
#include <string.h>
#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>
#include <nanvix/perf.h>

/**
 * @brief Number of iterations.
 */
#define NITERATIONS 1024

/**
 * @brief Magic number used for checksum.
 */
#define MAGIC 0xdeadbeef

/**
 * @brief Unicats unit test server.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static int unicast_server(void)
{
	int score = 0;
	long start, end, total;

	timer_init();

	/* Test. */
	total = 0;
	for (int i = 0; i < (NR_CCLUSTER/2)*NITERATIONS; i++)
	{
		int remote;
		unsigned ack = MAGIC;
		unsigned msg = ~(MAGIC + 1);

		start = timer_get();
			remote = nanvix_noc_receive(&msg);
			nanvix_noc_send(remote, &ack);
		end = timer_get();
		total += timer_diff(start, end);

		if ((remote > 0)  && (remote <= NR_CCLUSTER/2) && (msg == (MAGIC + 1)))
			score++;
	}

	printf("noc unicast   server    %d bytes %ld s\n", (NR_CCLUSTER/2)*NITERATIONS*sizeof(unsigned), total);

	return (score == (NR_CCLUSTER/2)*NITERATIONS);
}

/**
 * @brief Unicats unit test client.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static int unicast_client(void)
{
	int score = 0;

	/* Test. */
	for (int i = 0; i < NITERATIONS; i++)
	{
		int remote;
		unsigned msg = (MAGIC + 1);
		unsigned ack = ~MAGIC;

		nanvix_noc_send(CCLUSTER0, &msg);
		remote = nanvix_noc_receive(&ack);

		if ((remote == CCLUSTER0) && (ack == MAGIC))
			score++;
	}

	return (score == NITERATIONS);
}


/**
 * @brief NoC library unit test.
 */
int main(int argc, char **argv)
{
	int ret;

	/* Missing parameters. */
	if (argc < 3)
	{
		printf("missing parameters");
		printf("usage: noc.test <unicast | broadcast> <client | server>");

		return (0);
	}

	nanvix_noc_init(1 + NR_CCLUSTER/2);

	/* Unicast. */
	if (!strcmp(argv[1], "unicast"))
	{
		if (!strcmp(argv[2], "client"))
		{
			ret = unicast_client();
			printf("noc unicast   client %2d test [%s]\n", mppa_getpid(), (ret) ? "passed" : "FAILED");
		}
		else
		{
			ret = unicast_server();
			printf("noc unicast   server    test [%s]\n", (ret) ? "passed" : "FAILED");
		}
	}

	return (EXIT_SUCCESS);
}
