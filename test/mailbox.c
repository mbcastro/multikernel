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
#include <nanvix/ipc.h>

/**
 * @brief Number of iterations.
 */
#define NITERATIONS 10

/**
 * @brief Magic number used for checksum.
 */
#define MAGIC 0xdeadbeef

/**
 * @brief Unit test server.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static int server(void)
{
	int output;
	int score = 0;
	unsigned msg1 = MAGIC;
	unsigned msg2 = ~MAGIC;

	output = nanvix_mailbox_open("/cpu1");

	for (int i = 0; i < NITERATIONS; i++)
	{
		nanvix_mailbox_receive(&msg2, sizeof(unsigned));
		nanvix_mailbox_send(output, &msg2, sizeof(unsigned));

		if (msg1 == msg2)
			score++;
	}

	return (score == NITERATIONS);
}

/**
 * @brief Unit test client.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static int client(void)
{
	int output;
	int score = 0;
	unsigned msg1 = MAGIC;
	unsigned msg2 = ~MAGIC;

	output = nanvix_mailbox_open("/cpu0");

	for (int i = 0; i < NITERATIONS; i++)
	{
		nanvix_mailbox_send(output, &msg1, sizeof(unsigned));
		nanvix_mailbox_receive(&msg2, sizeof(unsigned));

		if (msg1 == msg2)
			score++;
	}

	return (score == NITERATIONS);
}

/**
 * @brief Mailbox unit test.
 */
int main(int argc, char **argv)
{
	int ret;

	/* Missing parameters. */
	if (argc < 2)
	{
		printf("missing parameters");
		printf("usage: noc.test <mode>");
		printf("  --client Client mode.");
		printf("  --server Server mode.");

		return (0);
	}

	nanvix_noc_init(2);

	/* Server */
	ret = (!strcmp(argv[1], "--server")) ? 
		server() : client();

	printf("mailbox test [%s]\n", (ret) ? "passed" : "FAILED");

	return (EXIT_SUCCESS);
}
