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

#define __NEED_HAL_BARRIER_
#include <nanvix/const.h>
#include <nanvix/syscalls.h>
#include <nanvix/pm.h>
#include <nanvix/limits.h>

#include "test.h"

/*============================================================================*
 * API Test: Invalid Create                                                   *
 *============================================================================*/

/**
 * @brief API Test: Invalid Create
 */
static void test_nanvix_ipc_barrier_invalid_create(void)
{
	int nodes[NANVIX_PROC_MAX + 1];
	
	/* Build nodes list. */
	nodes[0] = sys_get_node_num();
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i] = i;

	TEST_ASSERT(barrier_create(NULL, NANVIX_PROC_MAX + 1) < 0);
	TEST_ASSERT(barrier_create(nodes, -1) < 0);
	TEST_ASSERT(barrier_create(nodes,  1000000) < 0);
}

/*============================================================================*
 * API Test: Bad Create                                                       *
 *============================================================================*/

/**
 * @brief API Test: Bad Create
 */
static void test_nanvix_ipc_barrier_bad_create(void)
{
	int nodes[NANVIX_PROC_MAX + 1];
	
	/* Build nodes list. */
	nodes[0] = sys_get_node_num();
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	TEST_ASSERT(barrier_create(&nodes[1], NANVIX_PROC_MAX) < 0);
	
	/* Build nodes list. */
	nodes[0] = sys_get_node_num();
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = 1000000;

	TEST_ASSERT(barrier_create(nodes, NANVIX_PROC_MAX + 1) < 0);
	
	/* Build nodes list. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = -1;

	TEST_ASSERT(barrier_create(nodes, NANVIX_PROC_MAX + 1) < 0);

	/* Build nodes list. */
	nodes[0] = 1000000;
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1 ] = i;

	TEST_ASSERT(barrier_create(nodes, NANVIX_PROC_MAX + 1) < 0);

	/* Build nodes list. */
	nodes[0] = -1;

	TEST_ASSERT(barrier_create(nodes, NANVIX_PROC_MAX + 1) < 0);
}

/*============================================================================*
 * API Test: Double Create                                                    *
 *============================================================================*/

/**
 * @brief API Test: Double Create
 */
static void test_nanvix_ipc_barrier_double_create(void)
{
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	
	/* Build nodes list. */
	nodes[0] = sys_get_node_num();
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);
	TEST_ASSERT(barrier_create(nodes, NANVIX_PROC_MAX + 1) < 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Invalid Unlink                                                   *
 *============================================================================*/

/**
 * @brief API Test: Invalid Unlink
 */
static void test_nanvix_ipc_barrier_invalid_unlink(void)
{
	TEST_ASSERT(barrier_unlink(-1) < 0);
	TEST_ASSERT(barrier_unlink(1000000) < 0);
}

/*============================================================================*
 * API Test: Bad Unlink                                                       *
 *============================================================================*/

/**
 * @brief API Test: Bad Unlink
 */
static void test_nanvix_ipc_barrier_bad_unlink(void)
{
	TEST_ASSERT(barrier_unlink(0) < 0);
}

/*============================================================================*
 * API Test: Double Unlink                                                    *
 *============================================================================*/

/**
 * @brief API Test: Double Unlink
 */
static void test_nanvix_ipc_barrier_double_unlink(void)
{
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	
	/* Build nodes list. */
	nodes[0] = sys_get_node_num();
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) < 0);
}

/*============================================================================*
 * API Test: Invalid Wait                                                     *
 *============================================================================*/

/**
 * @brief API Test: Invalid Wait
 */
static void test_nanvix_ipc_barrier_invalid_wait(void)
{
	TEST_ASSERT(barrier_wait(-1) < 0);
	TEST_ASSERT(barrier_wait(1000000) < 0);
}

/*============================================================================*
 * API Test: Bad Wait                                                         *
 *============================================================================*/

/**
 * @brief API Test: Bad Wait
 */
static void test_nanvix_ipc_barrier_bad_wait(void)
{
	TEST_ASSERT(barrier_wait(0) < 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test nanvix_ipc_barrier_tests_fault[] = {
	{ test_nanvix_ipc_barrier_invalid_create, "Invalid Create" },
	{ test_nanvix_ipc_barrier_bad_create,     "Bad Create"     },
	{ test_nanvix_ipc_barrier_double_create,  "Double Create"  },
	{ test_nanvix_ipc_barrier_invalid_unlink, "Invalid Unlink" },
	{ test_nanvix_ipc_barrier_bad_unlink,     "Bad Unlink"     },
	{ test_nanvix_ipc_barrier_double_unlink,  "Double Unlink"  },
	{ test_nanvix_ipc_barrier_invalid_wait,   "Invalid Wait"   },
	{ test_nanvix_ipc_barrier_bad_wait,       "Bad Wait"       },
	{ NULL,                                   NULL             },
};
