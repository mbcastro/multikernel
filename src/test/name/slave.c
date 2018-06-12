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

#include <nanvix/arch/mppa.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>
#include <nanvix/hal.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

// static int msg;

/*====================================================================*
 * main                                                               *
 *====================================================================*/

/**
 * @brief Remote name unit test.
 */
int main(int argc, char **argv)
{
	char pathname[PROC_NAME_MAX];
	// char out_pathname[PROC_NAME_MAX];
	int barrier;
	int clusterid;
	int nclusters;
	// int inbox;
	// int outbox;

	clusterid = hal_get_cluster_id();

	/* Retrieve parameters. */
	assert(argc == 2);
	assert((nclusters = atoi(argv[1])) > 0);

	barrier = barrier_open(nclusters);

	sprintf(pathname, "/cpu%d", clusterid);

	/* Primitives test. */

	/* Ask for an unregistered entry. */
	assert(name_lookup(pathname) == (-ENOENT));

	/* Register this cluster. */
	assert(name_link(clusterid, pathname) == 0);

	/* Ask for a registered entry. */
	assert(name_lookup(pathname) == clusterid);

	/* Remove the entry. */
	assert(name_unlink(pathname) == 0);

	/* Verify that the entry is removed. */
	assert(name_lookup(pathname) == (-ENOENT));

	// /* Register this cluster. */
	// assert(name_link(clusterid, pathname) == 0);
	//
	// /* Wait for others clusters. */
	// barrier_wait(barrier);
	//
	// /* Message exchange test using name resolution. */
	//
	// assert(nclusters > 1);
	//
	// inbox = mailbox_create(pathname);
	// sprintf(out_pathname, "/cpu%d", (clusterid + 1)%nclusters);
	//
	// outbox = mailbox_open(out_pathname);
	//
	// msg = clusterid;
	// assert(mailbox_write(outbox, &msg) == 0);
	//
	// msg = -1;
	// while(msg == -1){
	// 	assert(mailbox_read(inbox, &msg) == 0);
	// }
	//
	// if (clusterid - 1 < 0)
	// {
	// 	assert(msg == (clusterid + nclusters - 1));
	// }
	// else
	// {
	// 	assert(msg == (clusterid - 1)%nclusters);
	//
	// }
	//
	// /* House keeping. */
	// assert(mailbox_close(outbox) == 0);
	// assert(mailbox_close(inbox) == 0);
	barrier_close(barrier);

	return (EXIT_SUCCESS);
}
