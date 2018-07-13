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
#include <stdlib.h>
#include <errno.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_MAILBOX_
#include <nanvix/hal.h>
#include <nanvix/limits.h>

/**
 * @brief ID of master node.
 */
static int masternode;
/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

/**
 * @brief Sync slaves.
 *
 * @param nclusters Number of slaves.
 */
static void sync_slaves(int nclusters)
{
	int nodeid;
	int syncid1, syncid2;
	static int nodes[NANVIX_PROC_MAX];

	nodeid = hal_get_node_id();

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	/* Open synchronization points. */
	if (nodeid == 0)
	{
		assert((syncid2 = hal_sync_create(nodes,
			nclusters,
			HAL_SYNC_ONE_TO_ALL)) >= 0
		);
		assert((syncid1 = hal_sync_open(nodes,
			nclusters,
			HAL_SYNC_ALL_TO_ONE)) >= 0
		);

		assert(hal_sync_signal(syncid1) == 0);
		assert(hal_sync_wait(syncid2) == 0);

		/* House keeping. */
		assert(hal_sync_close(syncid1) == 0);
		assert(hal_sync_unlink(syncid2) == 0);
	}
	else
	{
		assert((syncid2 = hal_sync_open(nodes,
			nclusters,
			HAL_SYNC_ONE_TO_ALL)) >= 0
		);
		assert((syncid1 = hal_sync_create(nodes,
			nclusters,
			HAL_SYNC_ALL_TO_ONE)) >= 0
		);

		assert(hal_sync_signal(syncid2) == 0);
		assert(hal_sync_wait(syncid1) == 0);

		/* House keeping. */
		assert(hal_sync_unlink(syncid1) == 0);
		assert(hal_sync_close(syncid2) == 0);
	}
}

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_hal_mailbox_create_unlink(void)
{
	int inbox;
	int nodeid;

	nodeid = hal_get_node_id();

	assert((inbox = hal_mailbox_create(nodeid)) >= 0);

	assert(hal_mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Open Close CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Open Close CC
 */
static void test_hal_mailbox_open_close(void)
{
	int outbox;

	assert((outbox = hal_mailbox_open(masternode)) >= 0);

	assert(hal_mailbox_close(outbox) == 0);
}

/*============================================================================*
 * API Test: Read Write CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Read Write CC
 */
static void test_hal_mailbox_read_write(int nclusters)
{
	int inbox;
	int outbox;
	int nodeid;
	char msg[HAL_MAILBOX_MSG_SIZE];

	nodeid = hal_get_node_id();

	assert((inbox = hal_mailbox_create(nodeid)) >= 0);

	sync_slaves(nclusters);

	assert((outbox = hal_mailbox_open((nodeid + 1)%nclusters)) >= 0);
	assert((hal_mailbox_write(
		outbox,
		msg,
		HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE)
	);
	assert((hal_mailbox_read(
		inbox,
		msg,
		HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE)
	);

	assert(hal_mailbox_close(outbox) == 0);
	assert(hal_mailbox_unlink(inbox) == 0);
}

/*============================================================================*/

/**
 * @brief HAL Mailbox Test Driver
 */
int main2(int argc, char **argv)
{
	int test;
	int nclusters;

	/* Retrieve kernel parameters. */
	assert(argc == 4);
	masternode = atoi(argv[1]);
	nclusters = atoi(argv[2]);
	test = atoi(argv[3]);

	((void) nclusters);

	switch (test)
	{
		case 0:
			test_hal_mailbox_create_unlink();
			break;
		case 1:
			test_hal_mailbox_open_close();
			break;
		case 2:
			test_hal_mailbox_read_write(nclusters);
			break;
		default:
			exit(EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
