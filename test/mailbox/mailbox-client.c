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

	outbox = mailbox_open("/io1");

	for (int i = 0; i < NITERATIONS; i++)
		mailbox_write(outbox, msg);

	return (1);
}

/**
 * @brief Mailbox unit test.
 */
int main(int argc, char **argv)
{
	int ret;
	int clusterid;

	((void) argc);
	((void) argv);
	
	clusterid = arch_get_cluster_id();
	ret = client();

	printf("cluster %2d: mailbox test [%s]\n", 
			clusterid,
			(ret) ? "passed" : "FAILED"
	);

	return (EXIT_SUCCESS);
}
