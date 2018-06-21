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

#include <nanvix/arch/mppa.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/hal.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief API Test: Synchronization Point Create Unlink
 */
static void test_hal_sync_create_unlink(int nclusters)
{
	int nodes[nclusters];
	int syncid;

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	if (nodes[0] == hal_get_node_id())
	{
		nodes[0] += nodes[1];
		nodes[1] -= nodes[1];
		nodes[0] -= nodes[1];
	}

	TEST_ASSERT((syncid = hal_sync_create(nodes, nclusters, HAL_SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
}

/**
 * @brief API Test: Synchronization Point Open Close
 */
static void test_hal_sync_master_open_close(int nclusters)
{
	int nodes_local[nclusters];
	int nodes[nclusters];
	int syncid_local;
	int syncid;

	/* Build local nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes_local[i] = i;

	if (nodes_local[0] == hal_get_node_id())
	{
		nodes_local[0] += nodes_local[1];
		nodes_local[1] -= nodes_local[1];
		nodes_local[0] -= nodes_local[1];
	}

	TEST_ASSERT((syncid_local = hal_sync_create(nodes_local, nclusters,
											HAL_SYNC_ONE_TO_ALL)) == 0);

	/* Wait for other clusters. */
	sleep(10);

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	if(nodes[0] != hal_get_node_id())
	{
		for (int i = 1; i < nclusters; i++)
			if(nodes[i] == hal_get_node_id())
			{
				nodes[i] = nodes[0];
				nodes[0] = hal_get_node_id();
			}
	}

	TEST_ASSERT((syncid = hal_sync_open(nodes, nclusters,
							 HAL_SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(hal_sync_close(syncid) == 0);

	TEST_ASSERT(hal_sync_unlink(syncid_local) == 0);
}

/**
 * @brief API Test: Synchronization Point Wait Signal
 */
static void test_hal_sync_thread_wait_signal(int nclusters)
{
	int syncid;
	int nodes[nclusters + 1];

	/* Build nodes list. */
	nodes[0] = 128;

	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	TEST_ASSERT((syncid = hal_sync_create(nodes, nclusters + 1, HAL_SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(hal_sync_wait(syncid) == 0);

	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
}

/*====================================================================*
 * main                                                               *
 *====================================================================*/

/**
 * @brief Remote sync unit test.
 */
int main(int argc, char **argv)
{
	int nclusters;
	int test;

	hal_setup();

	TEST_ASSERT(argc == 3);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);
	test = atoi(argv[2]);

	if(test == 0)
	{
		test_hal_sync_create_unlink(nclusters);
		test_hal_sync_master_open_close(nclusters);
	}
	else if(test == 1)
	{
		test_hal_sync_thread_wait_signal(nclusters);
	}


	hal_cleanup();
	return (EXIT_SUCCESS);
}
