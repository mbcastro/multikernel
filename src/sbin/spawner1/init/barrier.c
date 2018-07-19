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

#include <mppaipc.h>

#include <nanvix/const.h>
#include <nanvix/syscalls.h>
#include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Number of cores in the underlying cluster.
 */
static int ncores = 0;

/*============================================================================*
 * API Test: Wait                                                             *
 *============================================================================*/

/**
 * @brief API Test: Wait
 */
static void test_barrier_io(void)
{
	int barrier;
	int nodenum;
	int nodes[2];

	nodenum = sys_get_node_num();

	nodes[0] = nodenum;
	nodes[1] = SPAWNER_SERVER_NODE;

	/* Wait on barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, 2)) >= 0);
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*===================================================================*
 * API Test: Compute Cluster - IO Cluster tests                      *
 *===================================================================*/

/**
* @brief API Test: Compute Cluster - IO Cluster tests.
*/
static void test_barrier_master_cc_io(int barrier)
{
	int nodenum;

	nodenum = sys_get_node_num();

	printf("%d waits...\n", nodenum);

	TEST_ASSERT(barrier_wait(barrier) == 0);

	printf("%d passed the barrier.\n", nodenum);

	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/**
 * @brief API Test: Compute Clusters - IO Clusters tests
 */
static void test_barrier_cc_io(int nclusters)
{
	int status;
	int pids[nclusters];
	int barrier;
	int nodes[(nclusters + 2)];

	char nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/barrier-slave",
		nclusters_str,
		test_str,
		NULL
	};

	printf("[test][api] Barrier Compute Clusters - IO Clusters 1\n");

	/* Build node list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i + 2] = i;

	nodes[0] = sys_get_node_num();
	nodes[1] = SPAWNER_SERVER_NODE;

	TEST_ASSERT((barrier = barrier_create(nodes, (nclusters + 2))) >= 0);

	sprintf(nclusters_str, "%d", nclusters);
	sprintf(test_str, "%d", 1);

	for (int i = 0; i < nclusters; i++)
		TEST_ASSERT((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);

	test_barrier_master_cc_io(barrier);

	for (int i = 0; i < nclusters; i++)
	{
		TEST_ASSERT(mppa_waitpid(pids[i], &status, 0) != -1);
		TEST_ASSERT(status == EXIT_SUCCESS);
	}
}

/*===================================================================*
 * API Test: Barrier Driver                                          *
 *===================================================================*/

/**
 * @brief Barrier test driver.
 */
void test_kernel_barrier(void)
{
	ncores = sys_get_num_cores();

	/* API tests. */
	test_barrier_io();
	if (0)
	{
		test_barrier_cc_io(16);
	}
}
