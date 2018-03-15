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
#include <nanvix/hal.h>
#include <nanvix/pm.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Number of iterations.
 */
#define NITERATIONS (8*1024)

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
static long server(void)
{
	int inbox;
	int score = 0;
	long start, end, total;
	char msg[MAILBOX_MSG_SIZE];
	char checksum[MAILBOX_MSG_SIZE];

	for (int i = 0; i < MAILBOX_MSG_SIZE; i++)
		checksum[i] = 5;

	timer_init();

	inbox = mailbox_create("/cpu0");

	int sync_fd = mppa_open("/mppa/sync/128:8", O_WRONLY);
	uint64_t mask = (1 << 0);
	mppa_write(sync_fd, &mask, sizeof(uint64_t));
	mppa_close(sync_fd);

	total = 0;
	for (int i = 0; i < (NR_CCLUSTER - 1)*NITERATIONS; i++)
	{
		start = timer_get();
			mailbox_read(inbox, msg);
		end = timer_get();
		total += timer_diff(start, end);

		if (!memcmp(msg, checksum, MAILBOX_MSG_SIZE))
			score++;
	}

	return (total);
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
	char msg[MAILBOX_MSG_SIZE];

	for (int i = 0; i < MAILBOX_MSG_SIZE; i++)
		msg[i] = 5;

	outbox = mailbox_open("/cpu0");

	for (int i = 0; i < NITERATIONS; i++)
		mailbox_write(outbox, msg);

	return (1);
}

/**
 * @brief Mailbox unit test.
 */
int main(int argc, char **argv)
{
	/* Missing parameters. */
	if (argc < 2)
	{
		printf("missing parameters\n");
		printf("usage: mailbox.test <client | server>\n");

		return (0);
	}

	/* Server */
	if (!strcmp(argv[1], "client"))
	{
		int ret = client();
		printf("cluster %2d: mailbox test [%s]\n", arch_get_cluster_id(), (ret) ? "passed" : "FAILED");
	}
	else
	{
		long total = server();
		printf("cluster %2d: mailbox test [passed]\n", arch_get_cluster_id() );
		printf("cluster %2d: server received %d KB in %lf s\n",
				arch_get_cluster_id(),
				((NR_CCLUSTER - 1)*NITERATIONS*MAILBOX_MSG_SIZE)/1024,
				total/1000000.0
		);
	}

	return (EXIT_SUCCESS);
}
