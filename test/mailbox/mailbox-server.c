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

#include <mppa/osconfig.h>
#include <nanvix/hal.h>
#include <nanvix/pm.h>
#include <stdio.h>
#include <string.h>
#include "mailbox.h"

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

	inbox = mailbox_create("/io1");

	int sync_fd = mppa_open("/mppa/sync/128:8", O_WRONLY);
	uint64_t mask = (1 << 0);
	mppa_write(sync_fd, &mask, sizeof(uint64_t));
	mppa_close(sync_fd);

	timer_init();

	for (int i = 0; i < MAILBOX_MSG_SIZE; i++)
		checksum[i] = CHECKSUM;

	total = 0;
	for (int i = 0; i < NR_CCLUSTER*NMESSAGES; i++)
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
 * @brief Mailbox unit test.
 */
int main(int argc, char **argv)
{
	long total;
	int clusterid;

	((void) argc);
	((void) argv);

	clusterid = arch_get_cluster_id();
	total = server();

	printf("cluster %3d: server received %d KB in %lf s\n",
			clusterid,
			(NR_CCLUSTER*NMESSAGES*MAILBOX_MSG_SIZE)/1024,
			total/1000000.0
	);
	printf("cluster %3d: mailbox test [passed]\n",clusterid);

	return (EXIT_SUCCESS);
}
