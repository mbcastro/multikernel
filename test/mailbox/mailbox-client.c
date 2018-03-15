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
 * @brief Cluster ID of the process.
 */
static int myclusterid;

/**
 * @brief Unit test client.
 *
 * @returns Upon successful non-zero is returned. Upon failure zero is
 * returned instead.
 */
static int client(void)
{
	int outbox;
	int outportal;
	struct message msg;
	char data[BLOCKSIZE];

	outbox = mailbox_open("/io1");
	outportal = portal_open("/io1");

	for (int i = 0; i < NMESSAGES; i++)
	{
		msg.source = myclusterid;
		msg.arg0 = BLOCKSIZE;

		mailbox_write(outbox, &msg);

		portal_write(outportal, data, BLOCKSIZE);
	}

	portal_close(outportal);
	mailbox_close(outbox);

	return (1);
}

/**
 * @brief Mailbox unit test.
 */
int main(int argc, char **argv)
{
	int ret;

	((void) argc);
	((void) argv);
	
	myclusterid = arch_get_cluster_id();
	ret = client();

	printf("cluster %3d: mailbox test [%s]\n", 
			myclusterid,
			(ret) ? "passed" : "FAILED"
	);

	return (EXIT_SUCCESS);
}
