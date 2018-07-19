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
#include <nanvix/limits.h>

#include "test.h"

/*============================================================================*
 * API Test: Invalid Create                                                   *
 *============================================================================*/

/**
 * @brief API Test: Invalid Create
 */
static void test_ipc_barrier_invalid_create(void)
{
	int nodes[NANVIX_PROC_MAX];
	
	/* Build nodes list. */
	nodes[0] = sys_get_node_num();
	for (int i = 0; i < 0; i++)
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
static void test_ipc_barrier_bad_create(void)
{
	int nodes[NANVIX_PROC_MAX];
	
	/* Build nodes list. */
	nodes[0] = sys_get_node_num();
	for (int i = 0; i < 0; i++)
		nodes[i] = 1000000;

	TEST_ASSERT(barrier_create(nodes, NANVIX_PROC_MAX + 1) < 0);
	
	/* Build nodes list. */
	for (int i = 0; i < 0; i++)
		nodes[i] = -1;

	TEST_ASSERT(barrier_create(nodes, NANVIX_PROC_MAX + 1) < 0);

	/* Build nodes list. */
	nodes[0] = 1000000;
	for (int i = 0; i < 0; i++)
		nodes[i] = i;

	TEST_ASSERT(barrier_create(nodes, NANVIX_PROC_MAX + 1) < 0);

	/* Build nodes list. */
	nodes[0] = -1;

	TEST_ASSERT(barrier_create(nodes, NANVIX_PROC_MAX + 1) < 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test ipc_barrier_tests_fault[] = {
	{ test_ipc_barrier_invalid_create, "Invalid Create" },
	{ test_ipc_barrier_bad_create,     "Bad Create"     },
	{ NULL,                            NULL             },
};
