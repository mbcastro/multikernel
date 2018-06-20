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

#include <nanvix/hal.h>

#include "../kernel.h"

#define MASTER_NODE 128

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

/**
 * @brief HAL Mailbox Microbenchmark Kernel
 */
static void kernel(int inbox, int outbox)
{
	char buffer[HAL_MAILBOX_MSG_SIZE];

	for (int i = 0; i < NITERATIONS; i++)
	{
		assert(hal_mailbox_read(inbox, buffer, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);
		assert(hal_mailbox_write(outbox, buffer, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);
	}
}

/*============================================================================*
 * HAL Mailbox Microbenchmark Driver                                          *
 *============================================================================*/

/**
 * @brief Syncs with remote master.
 */
static void sync_master(int nremotes)
{
	int syncid;
	int nodes[nremotes + 1];

	/* Build nodes list. */
	nodes[0] = MASTER_NODE;
	for (int i = 0; i < nremotes; i++)
		nodes[i + 1] = i;

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
	int inbox;
	int outbox;
	int nremotes;

	hal_setup();

	/* Retrieve kernel parameters. */
	assert(argc == 2);
	nremotes = atoi(argv[1]);

	int nodeid = hal_get_node_id();

		printf("node %d: message sent %d\n", nodeid, HAL_MAILBOX_MSG_SIZE);


	/* Open mailboxes. */
	assert((inbox = hal_mailbox_create(nodeid)) >= 0);
	assert((outbox = hal_mailbox_open(128)) >= 0);

	sync_master(nremotes);

	/* Run kernel. */
	kernel(inbox, outbox);

	/* House keeping. */
	assert(hal_mailbox_close(outbox) == 0);
	assert((hal_mailbox_unlink(inbox)) == 0);

	hal_cleanup();
	return (EXIT_SUCCESS);
}
