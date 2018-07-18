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

#include <nanvix/const.h>
#include <nanvix/syscalls.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/*============================================================================*
 * API Test: Barrier Mode                                                     *
 *============================================================================*/

/**
 * @brief API Test: Barrier
 */
static void test_sys_sync_barrier(void)
{
	int nodenum;
	int syncid;
	int syncid_local;
	int nodes[2];
	int nodes_local[2];

	nodenum = sys_get_node_num();

	nodes[0] = nodenum;
	nodes[1] = SPAWNER_SERVER_NODE;

	nodes_local[0] = SPAWNER_SERVER_NODE;
	nodes_local[1] = nodenum;

	/* Open syncrhonization points. */
	TEST_ASSERT((syncid_local = sys_sync_create(nodes_local, 2, SYNC_ONE_TO_ALL)) >= 0);
	TEST_ASSERT((syncid = sys_sync_open(nodes, 2, SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(sys_sync_wait(syncid_local) == 0);
	TEST_ASSERT(sys_sync_signal(syncid) == 0);

	/* House keeping. */
	TEST_ASSERT(sys_sync_unlink(syncid_local) == 0);
	TEST_ASSERT(sys_sync_close(syncid) == 0);
}

/*===================================================================*
 * Synchronization Point Test Driver                                 *
 *===================================================================*/

/**
 * @brief Synchronization Point Test Driver
 */
void test_kernel_sys_sync(void)
{
	test_sys_sync_barrier();
}
