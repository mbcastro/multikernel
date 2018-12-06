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

#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/pm.h>

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

/*============================================================================*
 * API Test: Read Write CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Read Write CC
 */
static void test_ipc_mailbox_read_write_cc(int nclusters)
{
	int inbox;
	int outbox;
	int barrier;
	int nodenum;
	int nodes[nclusters];
	char buffer[MAILBOX_MSG_SIZE];
	char pathname1[NANVIX_PROC_NAME_MAX];
	char pathname2[NANVIX_PROC_NAME_MAX];

	nodenum = sys_get_node_num();

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, nclusters)) >= 0);

	/* Create input mailbox. */
	sprintf(pathname1, "inbox%d", nodenum);
	TEST_ASSERT((inbox = mailbox_create(pathname1)) >= 0);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	/* Open output mailbox. */
	sprintf(pathname2, "inbox%d", (nodenum + 1)%nclusters);
	TEST_ASSERT((outbox = mailbox_open(pathname2)) >= 0);

	/* Sync. */
#ifndef _TEST_MAN_IN_THE_MIDDLE
	TEST_ASSERT(barrier_wait(barrier) == 0);
#endif

	memset(buffer, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(mailbox_write(outbox, buffer, MAILBOX_MSG_SIZE) == 0);

	memset(buffer, 0, MAILBOX_MSG_SIZE);
	TEST_ASSERT(mailbox_read(inbox, buffer, MAILBOX_MSG_SIZE) == 0);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	/* Checksum. */
	for (int i = 0; i < MAILBOX_MSG_SIZE; i++)
		TEST_ASSERT(buffer[i] == 1);

	/* House keeping. */
	TEST_ASSERT(mailbox_close(outbox) == 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Read Write 2 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Read Write 2 CC
 */
static void test_ipc_mailbox_read_write2_cc(void)
{
	int outbox;
	char buffer[MAILBOX_MSG_SIZE];

	/* Open output mailbox. */
	TEST_ASSERT((outbox = mailbox_open("master")) >= 0);

	memset(buffer, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(mailbox_write(outbox, buffer, MAILBOX_MSG_SIZE) == 0);

	/* House keeping. */
	TEST_ASSERT(mailbox_close(outbox) == 0);
}

/*============================================================================*
 * API Test: Read Write 3 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Read Write 3 CC
 */
static void test_ipc_mailbox_read_write3_cc(int masternode, int nclusters)
{
	int inbox;
	int barrier;
	int nodenum;
	int nodes[nclusters + 1];
	char buffer[MAILBOX_MSG_SIZE];
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = sys_get_node_num();

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, nclusters + 1)) >= 0);

	/* Create input mailbox. */
	sprintf(pathname, "cluster%d", nodenum);
	TEST_ASSERT((inbox = mailbox_create(pathname)) >= 0);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	memset(buffer, 0, MAILBOX_MSG_SIZE);
	TEST_ASSERT(mailbox_read(inbox, buffer, MAILBOX_MSG_SIZE) == 0);

	/* Checksum. */
	for (int i = 0; i < MAILBOX_MSG_SIZE; i++)
		TEST_ASSERT(buffer[i] == 1);

	/* House keeping. */
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*/

/**
 * @brief Mailbox unit test.
 */
int main2(int argc, char **argv)
{
	int test;
	int masternode;
	int nclusters;

	/* Retrieve kernel parameters. */
	TEST_ASSERT(argc == 4);
	masternode = atoi(argv[1]);
	nclusters = atoi(argv[2]);
	test = atoi(argv[3]);

	switch(test)
	{
		/* Create Unlink CC */
		case 0:
			test_ipc_mailbox_create_unlink_cc();
			break;

		/* Open Close CC */
		case 1:
			test_ipc_mailbox_open_close_cc(nclusters);
			break;

		/* Read Write CC */
		case 2:
			test_ipc_mailbox_read_write_cc(nclusters);
			break;

		/* Read Write 2 CC */
		case 3:
			test_ipc_mailbox_read_write2_cc();
			break;

		/* Read Write 3 CC */
		case 4:
			test_ipc_mailbox_read_write3_cc(masternode, nclusters);
			break;

		/* Should not happen. */
		default:
			return (-EXIT_FAILURE);
			break;
	}

	return (EXIT_SUCCESS);
}
