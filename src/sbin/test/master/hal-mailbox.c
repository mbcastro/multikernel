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
#include <stdio.h>

#include <mppaipc.h>

#include <nanvix/syscalls.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

/**
 * @brief PID of slaves.
 */
static int pids[NANVIX_PROC_MAX];

/**
 * @brief Spawn slave processes.
 */
static void spawn_slaves(const char **args)
{
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		TEST_ASSERT((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);
}

/**
 * @brief Wait for slaves to terminate.
 */
static void join_slaves(void)
{
	int status;

	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		TEST_ASSERT(mppa_waitpid(pids[i], &status, 0) != -1);
		TEST_ASSERT(status == EXIT_SUCCESS);
	}
}

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_sys_mailbox_create_unlink_cc(void)
{
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-mailbox-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][mailbox] Create Unlink CC\n");

	/* Build arguments. */
	sprintf(masternode_str, "%d", sys_get_node_num());
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 0);

	spawn_slaves(args);
	join_slaves();
}

/*============================================================================*
 * API Test: Open Close CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Open Close CC
 */
static void test_sys_mailbox_open_close_cc(void)
{
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-mailbox-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][mailbox] Open Close CC\n");

	/* Build arguments. */
	sprintf(masternode_str, "%d", sys_get_node_num());
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 1);

	spawn_slaves(args);
	join_slaves();
}

/*============================================================================*
 * API Test: Read Write CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Read Write CC
 */
static void test_sys_mailbox_read_write_cc(void)
{
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-mailbox-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][mailbox] Read Write CC\n");

	/* Build arguments. */
	sprintf(masternode_str, "%d", sys_get_node_num());
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 2);

	spawn_slaves(args);
	join_slaves();
}

/*============================================================================*
 * API Test: Read Write 2 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Read Write 2 CC
 */
static void test_sys_mailbox_read_write2_cc(void)
{
	int syncid;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-mailbox-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][mailbox] Read Write 2 CC\n");

	/* Build arguments. */
	sprintf(masternode_str, "%d", sys_get_node_num());
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 3);

	/* Build nodes list. */
	nodes[0] = sys_get_node_num();
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	/* Create synchronization point. */
	TEST_ASSERT((syncid = sys_sync_create(
		nodes,
		NANVIX_PROC_MAX + 1,
		HAL_SYNC_ALL_TO_ONE)) >= 0
	);

	spawn_slaves(args);

	/* Wait. */
	TEST_ASSERT(sys_sync_wait(syncid) == 0);

	/* Send messages. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		int outbox;
		char msg[HAL_MAILBOX_MSG_SIZE];

		TEST_ASSERT((outbox = sys_mailbox_open(i)) >=0);
		TEST_ASSERT((sys_mailbox_write(
			outbox,
			msg,
			HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE)
		);
		TEST_ASSERT(sys_mailbox_close(outbox) == 0);
	}

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(sys_sync_unlink(syncid) == 0);
}

/*============================================================================*
 * API Test: Read Write 3 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Read Write 2 CC
 */
static void test_sys_mailbox_read_write3_cc(void)
{
	int inbox;
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-mailbox-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][mailbox] Read Write 3 CC\n");

	/* Build arguments. */
	sprintf(masternode_str, "%d", sys_get_node_num());
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 4);

	TEST_ASSERT((inbox = get_inbox()) >= 0);

	spawn_slaves(args);

	/* Receive messages. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		char msg[HAL_MAILBOX_MSG_SIZE];

		TEST_ASSERT((sys_mailbox_read(
			inbox,
			msg,
			HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE)
		);
	}

	join_slaves();
}

/*============================================================================*/

/**
 * @brief Automated HAL mailbox test driver.
 */
void test_sys_mailbox(void)
{
	test_sys_mailbox_create_unlink_cc();
	test_sys_mailbox_open_close_cc();
	test_sys_mailbox_read_write_cc();
	test_sys_mailbox_read_write2_cc();
	test_sys_mailbox_read_write3_cc();
}

