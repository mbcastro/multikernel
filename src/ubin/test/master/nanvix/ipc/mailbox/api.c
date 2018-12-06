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
#include <string.h>
#include <stdlib.h>

#ifdef _KALRAY_MPPA256

#include <mppaipc.h>

#else

#define UNUSED(x) ((void)(x))

static inline int mppa_spawn(int a, void *b, const void *c, void *d, void *e)
{
	UNUSED(a); UNUSED(b); UNUSED(c); UNUSED(d); UNUSED(e);
	return (0);
}

static inline int mppa_waitpid(int a, void *b, int c)
{
	UNUSED(a); UNUSED(b); UNUSED(c);

	return (0);
}

#endif

#define __NEED_HAL_BARRIER_
#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

#include "test.h"

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
 * API Test: Create Unlink                                                    *
 *============================================================================*/

/**
 * @brief API Test: Mailbox Create Unlink
 */
static void test_nanvix_ipc_mailbox_create_unlink(void)
{
	int inbox;
	char pathname[NANVIX_PROC_NAME_MAX];

	sprintf(pathname, "cool-name");
	TEST_ASSERT((inbox = mailbox_create(pathname)) >= 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_nanvix_ipc_mailbox_create_unlink_cc(void)
{
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/ipc-mailbox-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

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
static void test_nanvix_ipc_mailbox_open_close_cc(void)
{
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/ipc-mailbox-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

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
static void test_nanvix_ipc_mailbox_read_write1_cc(void)
{
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/ipc-mailbox-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

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
static void test_nanvix_ipc_mailbox_read_write2_cc(void)
{
	int inbox;
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	char buffer[MAILBOX_MSG_SIZE];
	const char *args[] = {
		"/test/ipc-mailbox-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	/* Build arguments. */
	sprintf(masternode_str, "%d", sys_get_node_num());
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 3);

	/* Create input mailbox. */
	TEST_ASSERT((inbox = mailbox_create("master")) >= 0);

	spawn_slaves(args);

	/* Receive messages. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		memset(buffer, 0, MAILBOX_MSG_SIZE);
		TEST_ASSERT(mailbox_read(inbox, buffer, MAILBOX_MSG_SIZE) == 0);

		/* Checksum. */
		for (int j = 0; j < MAILBOX_MSG_SIZE; j++)
			TEST_ASSERT(buffer[j] == 1);
	}

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Read Write 3 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Read Write 3 CC
 */
static void test_nanvix_ipc_mailbox_read_write3_cc(void)
{
	int nodenum;
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/ipc-mailbox-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	nodenum = sys_get_node_num();

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodenum);
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 4);

	/* Build nodes list. */
	nodes[0] = nodenum;
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);

	spawn_slaves(args);

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	/* Send messages. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		int outbox;
		char buffer[MAILBOX_MSG_SIZE];
		char pathname[NANVIX_PROC_NAME_MAX];

		/* Open output mailbox. */
		sprintf(pathname, "cluster%d", i);
		TEST_ASSERT((outbox = mailbox_open(pathname)) >= 0);

		/* Send messages. */
		memset(buffer, 1, MAILBOX_MSG_SIZE);
		TEST_ASSERT(mailbox_write(outbox, buffer, MAILBOX_MSG_SIZE) == 0);

		/* Close output mailbox. */
		TEST_ASSERT(mailbox_close(outbox) == 0);
	}

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test nanvix_ipc_mailbox_tests_api[] = {
	{ test_nanvix_ipc_mailbox_create_unlink,    "Create Unlink"    },
	{ test_nanvix_ipc_mailbox_create_unlink_cc, "Create Unlink CC" },
	{ test_nanvix_ipc_mailbox_open_close_cc,    "Open Close CC"    },
	{ test_nanvix_ipc_mailbox_read_write1_cc,   "Read Write 1 CC"  },
	{ test_nanvix_ipc_mailbox_read_write2_cc,   "Read Write 2 CC"  },
	{ test_nanvix_ipc_mailbox_read_write3_cc,   "Read Write 3 CC"  },
	{ NULL,                                      NULL              },
};
