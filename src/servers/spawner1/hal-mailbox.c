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
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_MAILBOX_
#include <nanvix/const.h>
#include <nanvix/hal.h>
#include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Number of cores in the underlying cluster.
 */
static int ncores = 0;

/**
 * @brief Synchronization point.
 */
static int syncid;
static int syncid_local;

static int nodes[2];
static int nodes_local[2];

/*===================================================================*
 * API Test: Open Close between IO Clusters                          *
 *===================================================================*/

/**
 * @brief API Test: Open Close between IO Clusters
 */
static void test_hal_mailbox_open_close_io(void)
{
	int inbox;
	int outbox;
	int nodeid;

	printf("[test][api] Mailbox Open Close IO Cluster 1\n");

	nodeid = hal_get_node_id();

	TEST_ASSERT((inbox = hal_mailbox_create(nodeid)) >= 0);

	TEST_ASSERT(hal_sync_wait(syncid_local) == 0);
	TEST_ASSERT(hal_sync_signal(syncid) == 0);

	TEST_ASSERT((outbox = hal_mailbox_open(hal_noc_nodes[SPAWNER_SERVER_NODE])) >= 0);

	TEST_ASSERT(hal_mailbox_close(outbox) == 0);

	TEST_ASSERT(hal_sync_wait(syncid_local) == 0);
	TEST_ASSERT(hal_sync_signal(syncid) == 0);

	TEST_ASSERT(hal_mailbox_unlink(inbox) == 0);
}

/*===================================================================*
 * Mailbox Test Driver                                               *
 *===================================================================*/

/**
 * @brief Mailbox Test Driver
 */
void test_kernel_hal_mailbox(void)
{
	ncores = hal_get_num_cores();

	/* Wait for other IO cluster. */
	nodes[0] = hal_get_node_id();
	nodes[1] = hal_noc_nodes[SPAWNER_SERVER_NODE];

	nodes_local[0] = hal_noc_nodes[SPAWNER_SERVER_NODE];
	nodes_local[1] = hal_get_node_id();

	TEST_ASSERT((syncid_local = hal_sync_create(nodes_local, 2, HAL_SYNC_ONE_TO_ALL)) >= 0);
	TEST_ASSERT((syncid = hal_sync_open(nodes, 2, HAL_SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(hal_sync_wait(syncid_local) == 0);
	TEST_ASSERT(hal_sync_signal(syncid) == 0);

	test_hal_mailbox_open_close_io();

	/* House keeping. */
	TEST_ASSERT(hal_sync_unlink(syncid_local) == 0);
	TEST_ASSERT(hal_sync_close(syncid) == 0);
}
