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
#include <errno.h>
#include <unistd.h>

#include <nanvix/syscalls.h>
#include <nanvix/limits.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Nodes list.
 */
static int nodes[NANVIX_PROC_MAX + 1];

/**
 * @brief ID of master node.
 */
static int masternode;

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_sys_sync_create_unlink(int nclusters)
{
	int syncid;

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	/* Fix nodes list. */
	if (nodes[0] == sys_get_node_num())
	{
		nodes[0] += nodes[1];
		nodes[1] -= nodes[1];
		nodes[0] -= nodes[1];
	}

	TEST_ASSERT((syncid = sys_sync_create(nodes,
		nclusters,
		SYNC_ONE_TO_ALL)) >= 0
	);

	TEST_ASSERT(sys_sync_unlink(syncid) == 0);
}

/*============================================================================*
 * API Test: Open Close CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Open Close CC
 */
static void test_sys_sync_master_open_close(int nclusters)
{
	int nodenum;
	int syncid;
	int syncid_local;
	int nodes_local[nclusters];

	/* Build local nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes_local[i] = i;

	nodenum = sys_get_node_num();

	if (nodes_local[0] == nodenum)
	{
		nodes_local[0] += nodes_local[1];
		nodes_local[1] -= nodes_local[1];
		nodes_local[0] -= nodes_local[1];
	}

	TEST_ASSERT((syncid_local = sys_sync_create(
		nodes_local,
		nclusters,
		SYNC_ONE_TO_ALL)) == 0
	);

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	/* Fix nodes list. */
	if (nodes[0] != nodenum)
	{
		for (int i = 1; i < nclusters; i++)
		{
			if (nodes[i] == nodenum)
			{
				nodes[i] = nodes[0];
				nodes[0] = nodenum;
			}
		}
	}

	TEST_ASSERT((syncid = sys_sync_open(
		nodes,
		nclusters,
		SYNC_ONE_TO_ALL)) >= 0
	);

	TEST_ASSERT(sys_sync_close(syncid) == 0);

	TEST_ASSERT(sys_sync_unlink(syncid_local) == 0);
}

/*============================================================================*
 * API Test: Wait Signal CC                                                   *
 *============================================================================*/

/**
 * @brief API Test: Wait Signal CC
 */
static void test_sys_sync_wait_signal(int nclusters)
{
	int syncid;

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	TEST_ASSERT((syncid = sys_sync_create(
		nodes,
		nclusters + 1,
		SYNC_ONE_TO_ALL)) >= 0
	);

	TEST_ASSERT(sys_sync_wait(syncid) == 0);

	TEST_ASSERT(sys_sync_unlink(syncid) == 0);
}

/*============================================================================*
 * API Test: Signal Wait CC                                                   *
 *============================================================================*/

/**
 * @brief API Test: Signal Wait CC
 */
static void test_sys_sync_signal_wait(int nclusters)
{
	int syncid;

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	TEST_ASSERT((syncid = sys_sync_open(nodes,
		nclusters + 1,
		SYNC_ALL_TO_ONE)) >= 0
	);

	TEST_ASSERT(sys_sync_signal(syncid) == 0);

	TEST_ASSERT(sys_sync_close(syncid) == 0);
}

/*============================================================================*
 * API Test: Barrier CC                                                       *
 *============================================================================*/

/**
 * @brief API Test: Barrier CC
 */
static void test_sys_sync_barrier(int nclusters)
{
	int syncid1, syncid2;

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Open synchronization points. */
	TEST_ASSERT((syncid2 = sys_sync_create(nodes,
		nclusters + 1,
		SYNC_ONE_TO_ALL)) >= 0
	);
	TEST_ASSERT((syncid1 = sys_sync_open(nodes,
		nclusters + 1,
		SYNC_ALL_TO_ONE)) >= 0
	);

	TEST_ASSERT(sys_sync_signal(syncid1) == 0);
	TEST_ASSERT(sys_sync_wait(syncid2) == 0);

	/* House keeping. */
	TEST_ASSERT(sys_sync_close(syncid1) == 0);
	TEST_ASSERT(sys_sync_unlink(syncid2) == 0);
}

/*============================================================================*
 * API Test: Barrier 2 CC                                                     *
 *============================================================================*/

/**
 * @brief API Test: Barrier 2 CC
 */
static void test_sys_sync_barrier2(int nclusters)
{
	int nodenum;
	int syncid1, syncid2;

	nodenum = sys_get_node_num();

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	/* Open synchronization points. */
	if (nodenum == 0)
	{
		TEST_ASSERT((syncid1 = sys_sync_create(nodes,
			nclusters,
			SYNC_ALL_TO_ONE)) >= 0
		);
		TEST_ASSERT((syncid2 = sys_sync_open(nodes,
			nclusters,
			SYNC_ONE_TO_ALL)) >= 0
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
			SYNC_ONE_TO_ALL)) >= 0
		);
		TEST_ASSERT((syncid1 = sys_sync_open(nodes,
			nclusters,
			SYNC_ALL_TO_ONE)) >= 0
		);

		TEST_ASSERT(sys_sync_signal(syncid1) == 0);
		TEST_ASSERT(sys_sync_wait(syncid2) == 0);

		/* House keeping. */
		TEST_ASSERT(sys_sync_unlink(syncid2) == 0);
		TEST_ASSERT(sys_sync_close(syncid1) == 0);
	}
}

/*============================================================================*/

/**
 * @brief HAL Sync Test Driver
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
			test_sys_sync_create_unlink(nclusters);
			break;
		case 1:
			test_sys_sync_master_open_close(nclusters);
			break;
		case 2:
			test_sys_sync_wait_signal(nclusters);
			break;
		case 3:
			test_sys_sync_signal_wait(nclusters);
			break;
		case 4:
			test_sys_sync_barrier(nclusters);
			break;
		case 5:
			test_sys_sync_barrier2(nclusters);
			break;
		default:
			exit(EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
