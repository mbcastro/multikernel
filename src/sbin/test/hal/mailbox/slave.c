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

#include <stdlib.h>
#include <errno.h>

#include <nanvix/syscalls.h>
#include <nanvix/pm.h>
#include <nanvix/limits.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

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
	int nodes[NANVIX_PROC_MAX];

	nodeid = sys_get_node_id();

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	/* Open synchronization points. */
	if (nodeid == 0)
	{
		TEST_ASSERT((syncid1 = sys_sync_create(nodes,
			nclusters,
			HAL_SYNC_ALL_TO_ONE)) >= 0
		);
		TEST_ASSERT((syncid2 = sys_sync_open(nodes,
			nclusters,
			HAL_SYNC_ONE_TO_ALL)) >= 0
		);

		TEST_ASSERT(sys_sync_wait(syncid1) == 0);
		TEST_ASSERT(sys_sync_signal(syncid2) == 0);

		/* House keeping. */
		TEST_ASSERT(sys_sync_close(syncid2) == 0);
		TEST_ASSERT(sys_sync_unlink(syncid1) == 0);
	}
	else
	{
		TEST_ASSERT((syncid2 = sys_sync_create(nodes,
			nclusters,
			HAL_SYNC_ONE_TO_ALL)) >= 0
		);
		TEST_ASSERT((syncid1 = sys_sync_open(nodes,
			nclusters,
			HAL_SYNC_ALL_TO_ONE)) >= 0
		);

		TEST_ASSERT(sys_sync_signal(syncid1) == 0);
		TEST_ASSERT(sys_sync_wait(syncid2) == 0);

		/* House keeping. */
		TEST_ASSERT(sys_sync_unlink(syncid2) == 0);
		TEST_ASSERT(sys_sync_close(syncid1) == 0);
	}
}

/**
 * @brief Sync with master.
 *
 * @poaram nclusters Number of slaves.
 */
static void sync_master(int nclusters)
{
	int syncid;
	int nodes[NANVIX_PROC_MAX + 1];

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	TEST_ASSERT((syncid = sys_sync_open(nodes,
		nclusters + 1,
		HAL_SYNC_ALL_TO_ONE)) >= 0
	);

	TEST_ASSERT(sys_sync_signal(syncid) == 0);

	TEST_ASSERT(sys_sync_close(syncid) == 0);
}

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_sys_mailbox_create_unlink(void)
{
	int inbox;

	TEST_ASSERT((inbox = get_inbox()) >= 0);
}

/*============================================================================*
 * API Test: Open Close CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Open Close CC
 */
static void test_sys_mailbox_open_close(void)
{
	int outbox;

	TEST_ASSERT((outbox = sys_mailbox_open(masternode)) >= 0);
	TEST_ASSERT(sys_mailbox_close(outbox) == 0);
}

/*============================================================================*
 * API Test: Read Write CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Read Write CC
 */
static void test_sys_mailbox_read_write(int nclusters)
{
	int inbox;
	int outbox;
	int nodeid;
	char msg[HAL_MAILBOX_MSG_SIZE];

	nodeid = sys_get_node_id();

	TEST_ASSERT((inbox = get_inbox()) >= 0);

	sync_slaves(nclusters);

	TEST_ASSERT((outbox = sys_mailbox_open((nodeid + 1)%nclusters)) >= 0);

	TEST_ASSERT((sys_mailbox_write(
		outbox,
		msg,
		HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE)
	);
	TEST_ASSERT((sys_mailbox_read(
		inbox,
		msg,
		HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE)
	);

	/* House keeping. */
	TEST_ASSERT(sys_mailbox_close(outbox) == 0);
}

/*============================================================================*
 * API Test: Read Write 2 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Read Write 2 CC
 */
static void test_sys_mailbox_read_write2(int nclusters)
{
	int inbox;
	char msg[HAL_MAILBOX_MSG_SIZE];

	TEST_ASSERT((inbox = get_inbox()) >= 0);

	sync_master(nclusters);

	TEST_ASSERT((sys_mailbox_read(
		inbox,
		msg,
		HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE)
	);
}

/*============================================================================*
 * API Test: Read Write 3 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Read Write 3 CC
 */
static void test_sys_mailbox_read_write3(void)
{
	int outbox;
	char msg[HAL_MAILBOX_MSG_SIZE];

	TEST_ASSERT((outbox = sys_mailbox_open(masternode)) >= 0);

	TEST_ASSERT((sys_mailbox_write(
		outbox,
		msg,
		HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE)
	);

	/* House keeping. */
	TEST_ASSERT(sys_mailbox_close(outbox) == 0);
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
	TEST_ASSERT(argc == 4);
	masternode = atoi(argv[1]);
	nclusters = atoi(argv[2]);
	test = atoi(argv[3]);

	switch (test)
	{
		case 0:
			test_sys_mailbox_create_unlink();
			break;
		case 1:
			test_sys_mailbox_open_close();
			break;
		case 2:
			test_sys_mailbox_read_write(nclusters);
			break;
		case 3:
			test_sys_mailbox_read_write2(nclusters);
			break;
		case 4:
			test_sys_mailbox_read_write3();
			break;
		default:
			exit(EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
