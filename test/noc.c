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

/**
 * @brief Number of iterations.
 */
#define NITERATIONS 10

/**
 * @brief Magic number used for checksum.
 */
#define MAGIC 0xdeadbeef

/**
 * @brief Ping Pong unit test server.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static int pingpong_server(void)
{
	int score = 0;
	unsigned msg1 = MAGIC;
	unsigned msg2 = ~MAGIC;

	/* Test. */
	for (int i = 0; i < NITERATIONS; i++)
	{
		nanvix_noc_receive(&msg2, sizeof(unsigned));
		nanvix_noc_send(CCLUSTER1, &msg2, sizeof(unsigned));

		if (msg1 == msg2)
			score++;
	}

	return (score == NITERATIONS);
}

/**
 * @brief Ping Pong unit test client.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static int pingpong_client(void)
{
	int score = 0;
	unsigned msg1 = MAGIC;
	unsigned msg2 = ~MAGIC;

	/* Test. */
	for (int i = 0; i < NITERATIONS; i++)
	{
		nanvix_noc_send(CCLUSTER0, &msg1, sizeof(unsigned));
		nanvix_noc_receive(&msg2, sizeof(unsigned));

		if (msg1 == msg2)
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
		printf("usage: noc.test <unicast | ping-pong> <client | server>");

		return (0);
	}

	nanvix_noc_init(2);

	/* Server */
	if (!strcmp(argv[2], "ping-ping"))
	{
		ret = (!strcmp(argv[3], "client")) ? 
			pingpong_server() : pingpong_client();

		printf("noc ping pong test [%s]\n", (ret) ? "passed" : "FAILED");
	}

	return (EXIT_SUCCESS);
}
