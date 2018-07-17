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
#include <pthread.h>

#include <nanvix/const.h>
#include <nanvix/syscalls.h>

#include "test.h"

/*============================================================================*
 * API Test: Query Core ID                                                    *
 *============================================================================*/

/**
 * @brief API Test: Query Core ID
 */
static void *test_thread_sys_get_core_id(void *args)
{
	int tid;

	sys_setup();
	pthread_barrier_wait(&core_barrier);

	tid = ((int *)args)[0];

	TEST_ASSERT(tid == sys_get_core_id());

	sys_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Query Core ID.
 */
static void test_sys_get_core_id(void)
{
	int args[core_ncores];
	pthread_t threads[core_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < core_ncores; i++)
	{
		args[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_thread_sys_get_core_id,
			&args[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < core_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Query Core Type                                                  *
 *============================================================================*/

/**
 * @brief API Test: Query Core Type
 */
static void *test_thread_sys_get_core_type(void *args)
{
	((void) args);

	sys_setup();
	pthread_barrier_wait(&core_barrier);

	TEST_ASSERT(sys_get_core_type() == HAL_CORE_SYSTEM);

	sys_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Query Core Type
 */
static void test_sys_get_core_type(void)
{
	pthread_t threads[core_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < core_ncores; i++)
	{
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_thread_sys_get_core_type,
			NULL)) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < core_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Query NoC Node ID                                                *
 *============================================================================*/

/**
 * @brief API Test: Query NoC Node ID
 */
static void test_sys_get_node_num(void)
{
	TEST_ASSERT(sys_get_node_num() == SPAWNER_SERVER_NODE);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test core_tests_api[] = {
	{ test_sys_get_core_id,    "Get Core ID"    },
	{ test_sys_get_core_type,  "Get Core Type"  },
	{ test_sys_get_node_num,   "Get Node Num"   },
	{ NULL,                    NULL             },
};
