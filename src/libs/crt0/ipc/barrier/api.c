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

#include <mppaipc.h>

#include <nanvix/const.h>
#include <nanvix/syscalls.h>
#include <nanvix/pm.h>

#include "test.h"

/*============================================================================*
 * API Test: Barrier Wait IO Clusters                                         *
 *============================================================================*/

/**
 * @brief API Test: Barrier IO Clusters
 */
static void test_barrier_wait(void)
{
	int barrier;
	int nodenum;
	int nodes[2];

	nodenum = sys_get_node_num();
   
	nodes[0] = SPAWNER1_SERVER_NODE;
	nodes[1] = nodenum;

	/* Wait on barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, 2)) >= 0);
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Compute Cluster - IO Cluster tests                               *
 *============================================================================*/

/**
 * @brief API Test: Barrier Compute Cluster - IO Cluster tests.
 */
static void test_barrier_cc()
{
	int barrier;
	int nodenum;
	int nodes[(16 + 2)];

	nodenum = sys_get_node_num();

	for (int i = 0; i < 16; i++)
		nodes[i + 2] = i;

	nodes[0] = SPAWNER1_SERVER_NODE;
	nodes[1] = nodenum;

	TEST_ASSERT((barrier = barrier_create(nodes, (16 + 2))) >= 0);
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test ipc_barrier_tests_api[] = {
	{ test_barrier_wait, "Wait"         },
	{ NULL,              NULL           },
	{ test_barrier_cc,   "Slaves Tests" },
	{ NULL,              NULL           },
};
