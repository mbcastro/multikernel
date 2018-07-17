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

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SYNC_

#include <nanvix/const.h>
#include <nanvix/syscalls.h>
#include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/*===================================================================*
 * API Test: Barrier Compute cluster tests                           *
 *===================================================================*/

/**
 * @brief API Test: Barrier compute clusters tests.
 */
static void test_barrier_cc(int nclusters)
{
	int barrier;
	int nodenum;
	int nodes[nclusters];

	nodenum = sys_get_node_num();

	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	TEST_ASSERT((barrier = barrier_create(nodes, nclusters)) >= 0);

	printf("%d waits...\n", nodenum);

	TEST_ASSERT(barrier_wait(barrier) == 0);

	printf("%d passed the barrier.\n", nodenum);

	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*===================================================================*
 * API Test: Compute Cluster - IO Cluster tests                      *
 *===================================================================*/

/**
 * @brief API Test: Barrier Compute cluster - IO cluster tests.
 */
static void test_barrier_cc_io(int nclusters)
{
	int barrier;
	int nodenum;
	int nodes[(nclusters + 2)];

	nodenum = sys_get_node_num();

	for (int i = 0; i < nclusters; i++)
		nodes[i + 2] = i;

	nodes[0] = SPAWNER1_SERVER_NODE;
	nodes[1] = SPAWNER_SERVER_NODE;

	TEST_ASSERT((barrier = barrier_create(nodes, (nclusters + 2))) >= 0);

	printf("%d waits...\n", nodenum);

	TEST_ASSERT(barrier_wait(barrier) == 0);

	printf("%d passed the barrier.\n", nodenum);

	TEST_ASSERT(barrier_unlink(barrier) == 0);
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
	TEST_ASSERT(argc == 3);
	nclusters = atoi(argv[1]);
	test = atoi(argv[2]);

	switch(test)
	{
		/* Compute clusters test. */
		case 0:
			test_barrier_cc(nclusters);

			break;

		/* IO clusters - Compute clusters test. */
		case 1:
			test_barrier_cc_io(nclusters);

			break;

		/* Should not happen. */
		default:
			break;
	}

	return (EXIT_SUCCESS);
}
