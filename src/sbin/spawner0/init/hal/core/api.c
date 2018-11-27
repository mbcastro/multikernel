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
#include <pthread.h>

#include <nanvix/const.h>
#include <nanvix/syscalls.h>

#include "test.h"

/*============================================================================*
 * API Test: Query Core ID                                                    *
 *============================================================================*/

/**
 * @brief Local lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief API Test: Query Core ID
 */
static void *test_thread_sys_get_core_id(void *args)
{
	int coreid;
	int *cores;

	cores = args;

	kernel_setup();
	pthread_barrier_wait(&core_barrier);

	coreid = sys_get_core_id();

	pthread_mutex_lock(&lock);
		TEST_ASSERT(!cores[coreid]);
		cores[coreid] = 1;
	pthread_mutex_unlock(&lock);

	kernel_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Query Core ID.
 */
static void test_sys_get_core_id(void)
{
	int cores[core_ncores];
	pthread_t threads[core_ncores];

	/* Spawn driver threads. */
	memset(cores, 0, core_ncores*sizeof(int));
	cores[0] = 1;
	for (int i = 1; i < core_ncores; i++)
	{
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_thread_sys_get_core_id,
			cores)) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < core_ncores; i++)
		pthread_join(threads[i], NULL);

	/* Check result. */
	for (int i = 0; i < core_ncores; i++)
		TEST_ASSERT(cores[i]);
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

	kernel_setup();
	pthread_barrier_wait(&core_barrier);

	TEST_ASSERT(sys_get_core_type() == CORE_SYSTEM);

	kernel_cleanup();
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
