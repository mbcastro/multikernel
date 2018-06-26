/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <mppa/osconfig.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_MAILBOX_
#include <nanvix/hal.h>

#include "../kernel.h"

/**
 * @brief Master node NoC ID.
 */
static int master_node;

/**
 * @brief Inbox for receiving messages.
 */
static int inbox;

/**
 * @brief Underlying NoC node ID.
 */
static int nodeid;

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

/**
 * @brief HAL Mailbox Microbenchmark Kernel
 */
static void kernel(void)
{
	int outbox;
	char buffer[HAL_MAILBOX_MSG_SIZE];

	/* Open outbox. */
	assert((outbox = hal_mailbox_open(master_node)) >= 0);

	/* Benchmark. */
	for (int k = 0; k < NITERATIONS; k++)
	{
		assert(hal_mailbox_read(inbox, buffer, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);
		assert(hal_mailbox_write(outbox, buffer, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);
	}

	/* House keeping. */
	assert(hal_mailbox_close(outbox) == 0);
}

/*============================================================================*
 * HAL Mailbox Microbenchmark Driver                                          *
 *============================================================================*/

/**
 * @brief Syncs with remote master.
 *
 * @param first_remote First remote in the nodes list.
 * @param last_remote  Last remote in the nodes list.
 */
static void sync_master(int first_remote, int last_remote)
{
	const int nremotes = last_remote - first_remote;
	int syncid;
	int nodes[nremotes + 1];

	/* Build nodes list. */
	nodes[0] = master_node;
	for (int i = 0; i < nremotes; i++)
		nodes[i + 1] = first_remote + i;

	/* Sync. */
	assert((syncid = hal_sync_open(nodes, nremotes + 1, HAL_SYNC_ALL_TO_ONE)) >= 0);
	assert(hal_sync_signal(syncid) == 0);
	assert(hal_sync_close(syncid) == 0);
}

/**
 * @brief HAL Mailbox Microbenchmark Driver
 */
int main(int argc, const char **argv)
{
	int first_remote;
	int last_remote;
	
	/* Initialization. */
	hal_setup();
	nodeid = hal_get_node_id();
	assert((inbox = hal_mailbox_create(nodeid)) >= 0);

	/* Retrieve kernel parameters. */
	assert(argc == 4);
	master_node = atoi(argv[1]);
	first_remote = atoi(argv[2]);
	last_remote = atoi(argv[3]);

	sync_master(first_remote, last_remote);

	/* Run kernel. */
	kernel();

	/* House keeping. */
	assert(hal_mailbox_unlink(inbox) == 0);
	hal_cleanup();

	return (EXIT_SUCCESS);
}
