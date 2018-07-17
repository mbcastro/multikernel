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

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <mppaipc.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SYNC_
#include <nanvix/const.h>
#include <nanvix/syscalls.h>

#include "test.h"

/*============================================================================*
 * Fault Injection Test: Invalid Create                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Invalid Create
 */
static void test_sys_sync_invalid_create(void)
{
	/* Build nodes list. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = sys_get_node_num() + i;

	TEST_ASSERT((sys_sync_create(NULL, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((sys_sync_create(nodes, -1, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((sys_sync_create(nodes, 0, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((sys_sync_create(nodes, 1, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((sys_sync_create(nodes, HAL_NR_NOC_NODES + 1, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((sys_sync_create(nodes, ncores, -1)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Create                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Bad Create
 */
static void test_sys_sync_bad_create1(void)
{
	/* Invalid list of NoC nodes. */
	for (int i = ncores - 1; i >= 0; i--)
		nodes[i] = -1;
	TEST_ASSERT((sys_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);

	/* Underlying NoC node is the sender. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = sys_get_node_num() + i;
	TEST_ASSERT((sys_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);

	/* Underlying NoC node is not listed. */
	for (int i = ncores - 1; i >=0; i--)
		nodes[i] = sys_get_node_num() + i - ncores + 1;
	TEST_ASSERT((sys_sync_create(nodes, ncores - 1, HAL_SYNC_ONE_TO_ALL)) < 0);

	/* Underlying NoC node appears twice in the list. */
	nodes[ncores - 1] = sys_get_node_num();
	nodes[ncores - 2] = sys_get_node_num();
	TEST_ASSERT((sys_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);
}

/**
 * @brief Fault Injection Test: Synchronization Point Bad Create
 */
static void test_sys_sync_bad_create2(void)
{
	/* Invalid list of NoC nodes. */
	for (int i = ncores - 1; i >= 0; i--)
		nodes[i] = -1;
	TEST_ASSERT((sys_sync_create(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) < 0);

	/* Underlying NoC node is not the receiver. */
	for (int i = ncores - 1; i >=0; i--)
		nodes[i] = sys_get_node_num() + i - ncores + 1;
	TEST_ASSERT((sys_sync_create(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) < 0);

	/* Underlying NoC node is not listed. */
	TEST_ASSERT((sys_sync_create(nodes, ncores - 1, HAL_SYNC_ALL_TO_ONE)) < 0);

	/* Underlying NoC node appears twice in the list. */
	nodes[ncores - 1] = sys_get_node_num();
	nodes[ncores - 2] = sys_get_node_num();
	TEST_ASSERT((sys_sync_create(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) < 0);
}

/**
 * @brief Fault Injection Test: Synchronization Point Bad Create
 */
static void test_sys_sync_bad_create(void)
{
	test_sys_sync_bad_create1();
	test_sys_sync_bad_create2();
}

/*============================================================================*
 * Fault Injection Test: Invalid Open                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Invalid Open
 */
static void test_sys_sync_invalid_open(void)
{
	/* Build nodes list. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = sys_get_node_num() + i;

	TEST_ASSERT((sys_sync_open(NULL, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((sys_sync_open(nodes, -1, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((sys_sync_open(nodes, 0, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((sys_sync_open(nodes, 1, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((sys_sync_open(nodes, HAL_NR_NOC_NODES + 1, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((sys_sync_open(nodes, ncores, -1)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Open                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Bad Open
 */
static void test_sys_sync_bad_open1(void)
{

	/* Invalid list of NoC nodes. */
	for (int i = ncores - 1; i >= 0; i--)
		nodes[i] = -1;
	TEST_ASSERT((sys_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);

	/* Underlying NoC node is not the sender. */
	for (int i = ncores - 1; i >=0; i--)
		nodes[i] = sys_get_node_num() + i - ncores + 1;
	TEST_ASSERT((sys_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);

	/* Underlying NoC node is not listed. */
	TEST_ASSERT((sys_sync_open(nodes, ncores - 1, HAL_SYNC_ONE_TO_ALL)) < 0);

	/* Underlying NoC node appears twice in the list. */
	nodes[ncores - 1] = sys_get_node_num();
	nodes[ncores - 2] = sys_get_node_num();
	TEST_ASSERT((sys_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);
}

/**
 * @brief Fault Injection Test: Synchronization Point Bad Open
 */
static void test_sys_sync_bad_open2(void)
{
	/* Invalid list of NoC nodes. */
	for (int i = ncores - 1; i >= 0; i--)
		nodes[i] = -1;
	TEST_ASSERT((sys_sync_open(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) < 0);

	/* Underlying NoC node is not the sender. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = sys_get_node_num() + i;
	TEST_ASSERT((sys_sync_open(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) < 0);

	/* Underlying NoC node is not listed. */
	TEST_ASSERT((sys_sync_open(&nodes[1], ncores - 1, HAL_SYNC_ALL_TO_ONE)) < 0);

	/* Underlying NoC node appears twice in the list. */
	nodes[ncores - 1] = sys_get_node_num();
	nodes[ncores - 2] = sys_get_node_num();
	TEST_ASSERT((sys_sync_open(&nodes[1], ncores, HAL_SYNC_ALL_TO_ONE)) < 0);
}

/**
 * @brief Fault Injection Test: Synchronization Point Bad Open
 */
static void test_sys_sync_bad_open(void)
{
	test_sys_sync_bad_open1();
	test_sys_sync_bad_open2();
}

/*============================================================================*
 * Fault Injection Test: Invalid Unlink                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Invalid Unlink
 */
static void test_sys_sync_invalid_unlink(void)
{
	TEST_ASSERT(sys_sync_unlink(-1) < 0);
	TEST_ASSERT(sys_sync_unlink(1) < 0);
	TEST_ASSERT(sys_sync_unlink(HAL_NR_SYNC) < 0);
	TEST_ASSERT(sys_sync_unlink(HAL_NR_SYNC + 1) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Unlink                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Bad Unlink
 */
static void test_sys_sync_bad_unlink(void)
{
	int syncid;

	for (int i = 0; i < ncores; i++)
		nodes[i] = sys_get_node_num() + i;

	TEST_ASSERT((syncid = sys_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(sys_sync_unlink(syncid) < 0);
	TEST_ASSERT(sys_sync_close(syncid) == 0);
}

/*============================================================================*
 * Fault Injection Test: Double Unlink                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Double Unlink
 */
static void test_sys_sync_double_unlink(void)
{
	int syncid;

	for (int i = 0; i < ncores; i++)
		nodes[i] = sys_get_node_num() + i;

	TEST_ASSERT((syncid = sys_sync_create(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) >= 0);
	TEST_ASSERT(sys_sync_unlink(syncid) == 0);
	TEST_ASSERT(sys_sync_unlink(syncid) < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Close                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Invalid Close
 */
static void test_sys_sync_invalid_close(void)
{
	TEST_ASSERT(sys_sync_close(-1) < 0);
	TEST_ASSERT(sys_sync_close(1) < 0);
	TEST_ASSERT(sys_sync_close(HAL_NR_SYNC) < 0);
	TEST_ASSERT(sys_sync_close(HAL_NR_SYNC + 1) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Close                                            *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Bad Close
 */
static void test_sys_sync_bad_close(void)
{
	int syncid;

	for (int i = 0; i < ncores; i++)
		nodes[i] = sys_get_node_num() + i;

	TEST_ASSERT((syncid = sys_sync_create(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) >= 0);

	TEST_ASSERT(sys_sync_close(syncid) < 0);
	TEST_ASSERT(sys_sync_unlink(syncid) == 0);
}

/*============================================================================*
 * Fault Injection Test: Double Close                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Double Close
 */
static void test_sys_sync_double_close(void)
{
	int syncid;

	for (int i = 0; i < ncores; i++)
		nodes[i] = sys_get_node_num() + i;

	TEST_ASSERT((syncid = sys_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);
	TEST_ASSERT(sys_sync_close(syncid) == 0);
	TEST_ASSERT(sys_sync_close(syncid) < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Signal                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Invalid Signal
 */
static void test_sys_sync_invalid_signal(void)
{
	TEST_ASSERT(sys_sync_signal(-1) < 0);
	TEST_ASSERT(sys_sync_signal(1) < 0);
	TEST_ASSERT(sys_sync_signal(HAL_NR_SYNC) < 0);
	TEST_ASSERT(sys_sync_signal(HAL_NR_SYNC + 1) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Signal                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Bad Signal
 */
static void test_sys_sync_bad_signal(void)
{
	int syncid;

	for (int i = 0; i < ncores; i++)
		nodes[i] = sys_get_node_num() + i;

	TEST_ASSERT((syncid = sys_sync_create(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) >= 0);

	TEST_ASSERT(sys_sync_signal(syncid) < 0);
	TEST_ASSERT(sys_sync_unlink(syncid) == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Wait                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Invalid Wait
 */
static void test_sys_sync_invalid_wait(void)
{
	TEST_ASSERT(sys_sync_wait(-1) < 0);
	TEST_ASSERT(sys_sync_wait(1) < 0);
	TEST_ASSERT(sys_sync_wait(HAL_NR_SYNC) < 0);
	TEST_ASSERT(sys_sync_wait(HAL_NR_SYNC + 1) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Wait                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Bad Wait
 */
static void test_sys_sync_bad_wait(void)
{
	int syncid;

	for (int i = 0; i < ncores; i++)
		nodes[i] = sys_get_node_num() + i;

	TEST_ASSERT((syncid = sys_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(sys_sync_wait(syncid) < 0);
	TEST_ASSERT(sys_sync_close(syncid) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_fault[] = {
	{ test_sys_sync_invalid_create, "Invalid Create" },
	{ test_sys_sync_bad_create,     "Bad Create"     },
	{ test_sys_sync_invalid_open,   "Invalid Open"   },
	{ test_sys_sync_bad_open,       "Bad Open"       },
	{ test_sys_sync_invalid_unlink, "Invalid Unlink" },
	{ test_sys_sync_bad_unlink,     "Bad Unlink"     },
	{ test_sys_sync_double_unlink,  "Double Unlink"  },
	{ test_sys_sync_invalid_close,  "Invalid Close"  },
	{ test_sys_sync_bad_close,      "Bad Close"      },
	{ test_sys_sync_double_close,   "Double Close"   },
	{ test_sys_sync_invalid_signal, "Invalid Signal" },
	{ test_sys_sync_bad_signal,     "Bad Signal"     },
	{ test_sys_sync_invalid_wait,   "Invalid Wait"   },
	{ test_sys_sync_bad_wait,       "Bad Wait"       },
	{ NULL,                         NULL             },
};
