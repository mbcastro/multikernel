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
	int inportal;
	long start, end, total;
	struct message msg;

	inbox = mailbox_create("/io1");
	inportal = portal_create("/io1");

	int sync_fd = mppa_open("/mppa/sync/128:8", O_WRONLY);
	uint64_t mask = (1 << 0);
	mppa_write(sync_fd, &mask, sizeof(uint64_t));
	mppa_close(sync_fd);

	timer_init();

	total = 0;
	for (int i = 0; i < NR_CCLUSTER*NMESSAGES; i++)
	{
		char data[BLOCKSIZE];

		mailbox_read(inbox, &msg);
	
		portal_allow(inportal, msg.source);

		start = timer_get();
			portal_read(inportal, data, msg.arg0);
		end = timer_get();
		total += timer_diff(start, end);
	}

	portal_unlink(inportal);
	mailbox_unlink(inbox);

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

	printf("cluster %3d: server received %d MB in %lf s\n",
			clusterid,
			(NR_CCLUSTER*NMESSAGES*BLOCKSIZE)/(1024*1204),
			total/1000000.0
	);
	printf("cluster %3d: mailbox test [passed]\n",clusterid);

	return (EXIT_SUCCESS);
}
