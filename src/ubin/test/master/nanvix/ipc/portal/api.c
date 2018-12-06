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
 * @brief API Test: Portal Create Unlink
 */
static void test_nanvix_ipc_portal_create_unlink(void)
{
	int inportal;
	char pathname[NANVIX_PROC_NAME_MAX];

	sprintf(pathname, "cool-name");
	TEST_ASSERT((inportal = portal_create(pathname)) >= 0);
	TEST_ASSERT(portal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_nanvix_ipc_portal_create_unlink_cc(void)
{
	char masternode_str[4];
	char portal_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/ipc-portal-slave",
		masternode_str,
		portal_nclusters_str,
		test_str,
		NULL
	};

	/* Build arguments. */
	sprintf(masternode_str, "%d", sys_get_node_num());
	sprintf(portal_nclusters_str, "%d", NANVIX_PROC_MAX);
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
static void test_nanvix_ipc_portal_open_close_cc(void)
{
	char masternode_str[4];
	char portal_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/ipc-portal-slave",
		masternode_str,
		portal_nclusters_str,
		test_str,
		NULL
	};

	/* Build arguments. */
	sprintf(masternode_str, "%d", sys_get_node_num());
	sprintf(portal_nclusters_str, "%d", NANVIX_PROC_MAX);
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
static void test_nanvix_ipc_portal_read_write1_cc(void)
{
	char masternode_str[4];
	char portal_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/ipc-portal-slave",
		masternode_str,
		portal_nclusters_str,
		test_str,
		NULL
	};

	/* Build arguments. */
	sprintf(masternode_str, "%d", sys_get_node_num());
	sprintf(portal_nclusters_str, "%d", NANVIX_PROC_MAX);
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
static void test_nanvix_ipc_portal_read_write2_cc(void)
{
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char portal_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/ipc-portal-slave",
		masternode_str,
		portal_nclusters_str,
		test_str,
		NULL
	};

	/* Build arguments. */
	sprintf(masternode_str, "%d", sys_get_node_num());
	sprintf(portal_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 3);

	/* Build nodes list. */
	nodes[0] = sys_get_node_num();
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);

	spawn_slaves(args);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	/* Send data. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		int outportal;
		char buffer[DATA_SIZE];
		char pathname[NANVIX_PROC_NAME_MAX];

		sprintf(pathname, "ccluster%d", i);
		TEST_ASSERT((outportal = portal_open(pathname)) >= 0);
		TEST_ASSERT((portal_write(
			outportal,
			buffer,
			DATA_SIZE) == DATA_SIZE)
		);
		TEST_ASSERT(portal_close(outportal) == 0);
	}

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Read Write 3 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Read Write 2 CC
 */
static void test_nanvix_ipc_portal_read_write3_cc(void)
{
	int nodenum;
	int inportal;
	char masternode_str[4];
	char portal_nclusters_str[4];
	char test_str[4];
	char pathname[NANVIX_PROC_NAME_MAX];
	const char *args[] = {
		"/test/ipc-portal-slave",
		masternode_str,
		portal_nclusters_str,
		test_str,
		NULL
	};

	nodenum = sys_get_node_num();

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodenum);
	sprintf(portal_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 4);

	sprintf(pathname, "iocluster%d", nodenum);
	TEST_ASSERT((inportal = portal_create(pathname)) >= 0);

	spawn_slaves(args);

	/* Receive data. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		char buffer[DATA_SIZE];

		TEST_ASSERT((portal_allow(
			inportal,
			i) == 0)
		);

		TEST_ASSERT((portal_read(
			inportal,
			buffer,
			DATA_SIZE) == DATA_SIZE)
		);
	}

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(portal_unlink(inportal) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test nanvix_ipc_portal_tests_api[] = {
	{ test_nanvix_ipc_portal_create_unlink,    "Create Unlink"    },
	{ test_nanvix_ipc_portal_create_unlink_cc, "Create Unlink CC" },
	{ test_nanvix_ipc_portal_open_close_cc,    "Open Close CC"    },
	{ test_nanvix_ipc_portal_read_write1_cc,   "Read Write 1 CC"  },
	{ test_nanvix_ipc_portal_read_write2_cc,   "Read Write 2 CC"  },
	{ test_nanvix_ipc_portal_read_write3_cc,   "Read Write 3 CC"  },
	{ NULL,                                     NULL              },
};
