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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <mppa/osconfig.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#include <nanvix/config.h>
#include <nanvix/hal.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }
/**
 * @brief Global barrier for synchronization.
 */
static pthread_barrier_t barrier;

/**
 * @brief Number of cores in the underlying cluster.
 */
static int ncores = 0;

/*===================================================================*
 * API Test: Query Cluster ID                                        *
 *===================================================================*/

/**
 * @brief API Test: Query Cluster ID
 */
static void *test_thread_hal_get_cluster_id(void *args)
{
	int arg;

	hal_setup();
	pthread_barrier_wait(&barrier);

	arg = ((int *)args)[0];

	TEST_ASSERT(arg == hal_get_cluster_id());

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Query Cluster ID.
 */
static void test_hal_get_cluster_id(void)
{
	int args[ncores];
	pthread_t threads[ncores];

	printf("[test][api] Query Cluster ID\n");

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		args[i] = hal_noc_nodes[SPAWNER_SERVER_NODE];
		assert((pthread_create(&threads[i],
			NULL,
			test_thread_hal_get_cluster_id,
			&args[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * API Test: Query Core ID                                           *
 *===================================================================*/

/**
 * @brief API Test: Query Core ID
 */
static void *test_thread_hal_get_core_id(void *args)
{
	int tid;

	hal_setup();
	pthread_barrier_wait(&barrier);

	tid = ((int *)args)[0];

	TEST_ASSERT(tid == hal_get_core_id());

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Query Core ID.
 */
static void test_hal_get_core_id(void)
{
	int args[ncores];
	pthread_t threads[ncores];

	printf("[test][api] Query Core ID\n");

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		args[i] = i;
		assert((pthread_create(&threads[i],
			NULL,
			test_thread_hal_get_core_id,
			&args[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * API Test: Query Core Type                                         *
 *===================================================================*/

/**
 * @brief API Test: Query Core Type
 */
static void *test_thread_hal_get_core_type(void *args)
{
	((void) args);

	hal_setup();
	pthread_barrier_wait(&barrier);

	TEST_ASSERT(hal_get_core_type() == HAL_CORE_SYSTEM);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Query Core Type
 */
static void test_hal_get_core_type(void)
{
	pthread_t threads[ncores];

	printf("[test][api] Query Core Type\n");

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		assert((pthread_create(&threads[i],
			NULL,
			test_thread_hal_get_core_type,
			NULL)) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * API Test: Query NoC Node ID                                         *
 *===================================================================*/

/**
 * @brief API Test: Query NoC Node ID
 */
static void *test_thread_hal_get_node_id(void *args)
{
	int tid;

	hal_setup();
	pthread_barrier_wait(&barrier);

	tid = ((int *)args)[0];

	TEST_ASSERT(hal_get_node_id() == hal_noc_nodes[SPAWNER_SERVER_NODE + tid]);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Query NoC Node ID
 */
static void test_hal_get_node_id(void)
{
	int args[ncores];
	pthread_t threads[ncores];

	printf("[test][api] Query NoC Node ID\n");

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		args[i] = i;
		assert((pthread_create(&threads[i],
			NULL,
			test_thread_hal_get_node_id,
			&args[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * HAL Test Driver                                                   *
 *===================================================================*/

/**
 * @brief HAL Test Driver
 */
int main(int argc, const char **argv)
{
	((void) argc);
	((void) argv);

	hal_setup();

	/*
	 * We hope that this is not buggy.
	 */
	ncores = hal_get_num_cores();
	printf("[test][api] Number of Cores = %d\n", ncores);

	pthread_barrier_init(&barrier, NULL, ncores - 1);

	/* Core Interface API tests. */
	test_hal_get_cluster_id();
	test_hal_get_core_id();
	test_hal_get_core_type();
	test_hal_get_node_id();

	hal_cleanup();
	return (EXIT_SUCCESS);
}
