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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/pm.h>

#define IO 192

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_ipc_mailbox_create_unlink_cc(void)
{
	int inbox;
	int nodenum;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = sys_get_node_num();

	sprintf(pathname, "inbox%d", nodenum);
	TEST_ASSERT((inbox = mailbox_create(pathname)) >= 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Open Close CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Open Close CC
 */
static void test_ipc_mailbox_open_close_cc(int nclusters)
{
	int outbox;
	int nodenum;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = sys_get_node_num();

	sprintf(pathname, "inbox%d", (nodenum + 1)%nclusters);
	TEST_ASSERT((outbox = mailbox_create(pathname)) >= 0);
	TEST_ASSERT(mailbox_unlink(outbox) == 0);
}

/*===================================================================*
 * API Test: Mailbox compute clusters test                           *
 *===================================================================*/

/**
 * @brief API Test: Mailbox compute clusters test
 */
static void test_ipc_mailbox_cc(int nclusters)
{
	char pathname_local[NANVIX_PROC_NAME_MAX];
	char pathname_remote[NANVIX_PROC_NAME_MAX];
	char buf[MAILBOX_MSG_SIZE];
	int nodenum;
	int inbox;
	int outbox;
	int barrier;
	int nodes[nclusters];

	nodenum = sys_get_node_num();

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	for (int i = 1; i < nclusters; i++)
		if (nodes[i] == 0)
		{
			nodes[i] = nodes[0];
			nodes[0] = 0;
		}

	TEST_ASSERT((barrier = barrier_create(nodes, nclusters)) >= 0);

	TEST_ASSERT(barrier_wait(barrier) == 0);

	sprintf(pathname_local, "cool-name%d", nodenum);

	TEST_ASSERT((inbox = mailbox_create(pathname_local)) >= 0);

	TEST_ASSERT(barrier_wait(barrier) == 0);

	sprintf(pathname_remote, "cool-name%d", (nodenum + 1)%nclusters);

	TEST_ASSERT((outbox = mailbox_open(pathname_remote)) >= 0);

	TEST_ASSERT(barrier_wait(barrier) == 0);

	memset(buf, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(mailbox_write(outbox, buf, sizeof(buf)) == 0);
	memset(buf, 0, MAILBOX_MSG_SIZE);
	TEST_ASSERT(mailbox_read(inbox, buf, sizeof(buf)) == 0);

	for (int i = 0; i < MAILBOX_MSG_SIZE; i++)
		TEST_ASSERT(buf[i] == 1);

	TEST_ASSERT(mailbox_close(outbox) == 0);

	TEST_ASSERT(mailbox_unlink(inbox) == 0);

	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/**
 * @brief API Test: Mailbox IO cluster -> Compute cluster test
 */
static void test_ipc_mailbox_io_cc(int nclusters)
{
	char pathname_local[NANVIX_PROC_NAME_MAX];
	char buf[MAILBOX_MSG_SIZE];
	int nodenum;
	int inbox;
	int syncid_local;
	int syncid;
	int nodes[(nclusters + 1)];

	nodenum = sys_get_node_num();

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	nodes[0] = 192;

	TEST_ASSERT((syncid_local = sys_sync_create(nodes, (nclusters + 1), SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT((syncid = sys_sync_open(nodes, (nclusters + 1), SYNC_ALL_TO_ONE)) >= 0);

	sprintf(pathname_local, "compute_cluster%d", nodenum);

	TEST_ASSERT((inbox = mailbox_create(pathname_local)) >= 0);

	/* Signal IO cluster. */
	TEST_ASSERT(sys_sync_signal(syncid) == 0);

	/* Wait for IO cluster. */
	TEST_ASSERT(sys_sync_wait(syncid_local) == 0);

	memset(buf, 0, MAILBOX_MSG_SIZE);
	TEST_ASSERT(mailbox_read(inbox, buf, sizeof(buf)) == 0);

	for (int i = 0; i < MAILBOX_MSG_SIZE; i++)
		TEST_ASSERT(buf[i] == 1);

	TEST_ASSERT(mailbox_unlink(inbox) == 0);

	/* House keeping. */
	TEST_ASSERT(sys_sync_unlink(syncid_local) == 0);
	TEST_ASSERT(sys_sync_close(syncid) == 0)
}

/**
 * @brief API Test: Compute cluster test -> Mailbox IO cluster
 */
static void test_ipc_mailbox_cc_io(int nclusters)
{
	char pathname_remote[NANVIX_PROC_NAME_MAX];
	char buf[MAILBOX_MSG_SIZE];
	int outbox;
	int syncid_local;
	int syncid;
	int nodes[(nclusters + 1)];

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	nodes[0] = 192;

	TEST_ASSERT((syncid_local = sys_sync_create(nodes, (nclusters + 1), SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT((syncid = sys_sync_open(nodes, (nclusters + 1), SYNC_ALL_TO_ONE)) >= 0);

	sprintf(pathname_remote, "IO1");

	/* Signal IO cluster. */
	TEST_ASSERT(sys_sync_signal(syncid) == 0);

	/* Wait for IO cluster. */
	TEST_ASSERT(sys_sync_wait(syncid_local) == 0);

	TEST_ASSERT((outbox = mailbox_open(pathname_remote)) >= 0);

	memset(buf, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(mailbox_write(outbox, buf, sizeof(buf)) == 0);

	TEST_ASSERT(mailbox_close(outbox) == 0);

	/* House keeping. */
	TEST_ASSERT(sys_sync_unlink(syncid_local) == 0);
	TEST_ASSERT(sys_sync_close(syncid) == 0)
}

/*====================================================================*
 * main                                                               *
 *====================================================================*/

/**
 * @brief Mailbox unit test.
 */
int main2(int argc, char **argv)
{
	int nclusters;
	int test;

	/* Retrieve kernel parameters. */
	TEST_ASSERT(argc == 4);
	nclusters = atoi(argv[2]);
	test = atoi(argv[3]);

	switch(test)
	{
		/* Create Unlink CC */
		case 0:
			test_ipc_mailbox_create_unlink_cc();
			break;

		/* Open Close CC. */
		case 1:
			test_ipc_mailbox_open_close_cc(nclusters);
			break;

		case 2:
			test_ipc_mailbox_cc(nclusters);
			break;

		/* IO cluster -> Compute cluster test. */
		case 3:
			test_ipc_mailbox_io_cc(nclusters);
			break;

		/* Compute cluster -> IO cluster test. */
		case 4:
			test_ipc_mailbox_cc_io(nclusters);
			break;

		/* Should not happen. */
		default:
			return (-EXIT_FAILURE);
			break;
	}

	return (EXIT_SUCCESS);
}
