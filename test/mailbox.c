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

#include <mppa/osconfig.h>
#include <nanvix/pm.h>
#include <stdio.h>
#include <string.h>

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
 * @returns Upon successful non-zero is returned. Upon failure zero is
 * returned instead.
 */
static int server(void)
{
	int inbox;
	int score = 0;
	char msg[MAILBOX_MSG_SIZE];
	char checksum[MAILBOX_MSG_SIZE];

	for (int i = 0; i < MAILBOX_MSG_SIZE; i++)
		checksum[i] = 5;

	inbox = nanvix_mailbox_create("/cpu1");

	for (int i = 0; i < NITERATIONS; i++)
	{
		mailbox_read(inbox, &msg);

		if (!memcmp(msg, checksum, MAILBOX_MSG_SIZE))
			score++;
	}

	return (score == NITERATIONS);
}

/**
 * @brief Unit test client.
 *
 * @returns Upon successful non-zero is returned. Upon failure zero is
 * returned instead.
 */
static int client(void)
{
	int outbox;

	output = nanvix_mailbox_open("/cpu0");

	for (int i = 0; i < NITERATIONS; i++)
	{
		mailbox_read(inbox, &msg);
	}

	return (1);
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
		printf("missing parameters\n");
		printf("usage: noc.test <client | server>\n";

		return (0);
	}

	/* Server */
	ret = (!strcmp(argv[1], "server")) ? 
		server() : client();

	printf("mailbox test [%s]\n", (ret) ? "passed" : "FAILED");

	return (EXIT_SUCCESS);
}
