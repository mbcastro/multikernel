/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

#include <mppa/osconfig.h>

#include <nanvix/config.h>
#include <nanvix/hal.h>

/**
 * @brief Number of cores in the underlying cluster.
 */
static int ncores = 0;

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Global barrier for synchronization.
 */
static pthread_barrier_t barrier;

/**
 * @brief Lock for critical sections.
 */
static pthread_mutex_t lock;

/*===================================================================*
 * API Test: Create Unlink                                           *
 *===================================================================*/

/**
 * @brief API Test: Synchronization Point Create Unlink
 */
static void *test_hal_sync_thread_create_unlink(void *args)
{
	int syncid;
	int *nodes;

	hal_setup();

	nodes = ((int *)args);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((syncid = hal_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Create Unlink
 */
static void test_hal_sync_create_unlink(void)
{
	int nodes[ncores];
	pthread_t tids[ncores];

	printf("[test][api] Create Unlink\n");

	/* Build nodes list. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		assert((pthread_create(&tids[i],
			NULL,
			test_hal_sync_thread_create_unlink,
			nodes)) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * API Test: Open Close                                              *
 *===================================================================*/

/**
 * @brief API Test: Synchronization Point Open Close
 */
static void *test_hal_sync_thread_open_close(void *args)
{
	int syncid;
	int *nodes;

	hal_setup();

	nodes = ((int *)args);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((syncid = hal_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Open Close
 */
static void test_hal_sync_master_open_close(const int *nodes)
{
	int syncid;

	pthread_mutex_lock(&lock);
	TEST_ASSERT((syncid = hal_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_close(syncid) == 0);
	pthread_mutex_unlock(&lock);
}

/**
 * @brief API Test: Synchronization Point Open Close
 */
static void test_hal_sync_open_close(void)
{
	int nodes[ncores];
	pthread_t tids[ncores];

	printf("[test][api] Open Close\n");

	/* Build nodes list. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		assert((pthread_create(&tids[i],
			NULL,
			test_hal_sync_thread_open_close,
			nodes)) == 0
		);
	}

	test_hal_sync_master_open_close(nodes);

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * API Test: Wait Signal                                             *
 *===================================================================*/

/**
 * @brief API Test: Synchronization Point Wait Signal
 */
static void *test_hal_sync_thread_wait_signal(void *args)
{
	int syncid;
	int *nodes;

	hal_setup();

	nodes = ((int *)args);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((syncid = hal_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	TEST_ASSERT(hal_sync_wait(syncid) == 0);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Wait Signal
 */
static void test_hal_sync_master_wait_signal(const int *nodes)
{
	int syncid;

	pthread_mutex_lock(&lock);
	TEST_ASSERT((syncid = hal_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);
	pthread_mutex_unlock(&lock);

	TEST_ASSERT(hal_sync_signal(syncid) == 0);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_close(syncid) == 0);
	pthread_mutex_unlock(&lock);
}

/**
 * @brief API Test: Synchronization Point Wait Signal
 */
static void test_hal_sync_wait_signal(void)
{
	int nodes[ncores];
	pthread_t tids[ncores];

	printf("[test][api] Wait Signal\n");

	/* Build nodes list. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		assert((pthread_create(&tids[i],
			NULL,
			test_hal_sync_thread_wait_signal,
			nodes)) == 0
		);
	}

	test_hal_sync_master_wait_signal(nodes);

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * API Test: Signal Wait                                             *
 *===================================================================*/

/**
 * @brief API Test: Synchronization Point Signal Wait
 */
static void *test_hal_sync_thread_signal_wait(void *args)
{
	int syncid;
	int *nodes;

	hal_setup();

	nodes = ((int *)args);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((syncid = hal_sync_open(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	TEST_ASSERT(hal_sync_signal(syncid) == 0);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_close(syncid) == 0);
	pthread_mutex_unlock(&lock);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Signal Wait
 */
static void test_hal_sync_master_signal_wait(const int *nodes)
{
	int syncid;

	pthread_mutex_lock(&lock);
	TEST_ASSERT((syncid = hal_sync_create(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) >= 0);
	pthread_mutex_unlock(&lock);

	TEST_ASSERT(hal_sync_wait(syncid) == 0);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
	pthread_mutex_unlock(&lock);
}

/**
 * @brief API Test: Synchronization Point Signal Wait
 */
static void test_hal_sync_signal_wait(void)
{
	int nodes[ncores];
	pthread_t tids[ncores];

	printf("[test][api] Signal Wait\n");

	/* Build nodes list. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		assert((pthread_create(&tids[i],
			NULL,
			test_hal_sync_thread_signal_wait,
			nodes)) == 0
		);
	}

	test_hal_sync_master_signal_wait(nodes);

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * Fault Injection Test: Invalid Create                              *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Invalid Create 
 */
static void test_hal_sync_invalid_create(void)
{
	int nodes[ncores];

	printf("[test][fault injection] Invalid Create\n");

	/* Build nodes list. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	TEST_ASSERT((hal_sync_create(NULL, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((hal_sync_create(nodes, -1, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((hal_sync_create(nodes, 0, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((hal_sync_create(nodes, 1, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((hal_sync_create(nodes, HAL_NR_NOC_NODES + 1, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((hal_sync_create(nodes, ncores, -1)) < 0);
}

/*===================================================================*
 * Fault Injection Test: Bad Create                                  *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Bad Create 
 */
static void test_hal_sync_bad_create1(void)
{
	int nodes[ncores];

	/* Invalid list of NoC nodes. */
	for (int i = ncores - 1; i >= 0; i--)
		nodes[i] = -1;
	TEST_ASSERT((hal_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);

	/* Underlying NoC node is the sender. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;
	TEST_ASSERT((hal_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);

	/* Underlying NoC node is not listed. */
	for (int i = ncores - 1; i >=0; i--)
		nodes[i] = hal_get_node_id() + i - ncores + 1;
	TEST_ASSERT((hal_sync_create(nodes, ncores - 1, HAL_SYNC_ONE_TO_ALL)) < 0);

	/* Underlying NoC node appears twice in the list. */
	nodes[ncores - 1] = hal_get_node_id();
	nodes[ncores - 2] = hal_get_node_id();
	TEST_ASSERT((hal_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);
}

/**
 * @brief Fault Injection Test: Synchronization Point Bad Create 
 */
static void test_hal_sync_bad_create2(void)
{
	int nodes[ncores];

	/* Invalid list of NoC nodes. */
	for (int i = ncores - 1; i >= 0; i--)
		nodes[i] = -1;
	TEST_ASSERT((hal_sync_create(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) < 0);

	/* Underlying NoC node is not the receiver. */
	for (int i = ncores - 1; i >=0; i--)
		nodes[i] = hal_get_node_id() + i - ncores + 1;
	TEST_ASSERT((hal_sync_create(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) < 0);

	/* Underlying NoC node is not listed. */
	TEST_ASSERT((hal_sync_create(nodes, ncores - 1, HAL_SYNC_ALL_TO_ONE)) < 0);

	/* Underlying NoC node appears twice in the list. */
	nodes[ncores - 1] = hal_get_node_id();
	nodes[ncores - 2] = hal_get_node_id();
	TEST_ASSERT((hal_sync_create(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) < 0);
}

/**
 * @brief Fault Injection Test: Synchronization Point Bad Create 
 */
static void test_hal_sync_bad_create(void)
{
	printf("[test][fault injection] Bad Create\n");

	test_hal_sync_bad_create1();
	test_hal_sync_bad_create2();
}

/*===================================================================*
 * Fault Injection Test: Invalid Open                                *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Invalid Open 
 */
static void test_hal_sync_invalid_open(void)
{
	int nodes[ncores];

	printf("[test][fault injection] Invalid Open\n");

	/* Build nodes list. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	TEST_ASSERT((hal_sync_open(NULL, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((hal_sync_open(nodes, -1, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((hal_sync_open(nodes, 0, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((hal_sync_open(nodes, 1, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((hal_sync_open(nodes, HAL_NR_NOC_NODES + 1, HAL_SYNC_ONE_TO_ALL)) < 0);
	TEST_ASSERT((hal_sync_open(nodes, ncores, -1)) < 0);
}

/*===================================================================*
 * Fault Injection Test: Bad Open                                    *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Bad Open 
 */
static void test_hal_sync_bad_open1(void)
{
	int nodes[ncores];

	/* Invalid list of NoC nodes. */
	for (int i = ncores - 1; i >= 0; i--)
		nodes[i] = -1;
	TEST_ASSERT((hal_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);

	/* Underlying NoC node is not the sender. */
	for (int i = ncores - 1; i >=0; i--)
		nodes[i] = hal_get_node_id() + i - ncores + 1;
	TEST_ASSERT((hal_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);

	/* Underlying NoC node is not listed. */
	TEST_ASSERT((hal_sync_open(nodes, ncores - 1, HAL_SYNC_ONE_TO_ALL)) < 0);

	/* Underlying NoC node appears twice in the list. */
	nodes[ncores - 1] = hal_get_node_id();
	nodes[ncores - 2] = hal_get_node_id();
	TEST_ASSERT((hal_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) < 0);
}

/**
 * @brief Fault Injection Test: Synchronization Point Bad Open 
 */
static void test_hal_sync_bad_open2(void)
{
	int nodes[ncores];

	/* Invalid list of NoC nodes. */
	for (int i = ncores - 1; i >= 0; i--)
		nodes[i] = -1;
	TEST_ASSERT((hal_sync_open(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) < 0);

	/* Underlying NoC node is not the sender. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;
	TEST_ASSERT((hal_sync_open(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) < 0);

	/* Underlying NoC node is not listed. */
	TEST_ASSERT((hal_sync_open(&nodes[1], ncores - 1, HAL_SYNC_ALL_TO_ONE)) < 0);

	/* Underlying NoC node appears twice in the list. */
	nodes[ncores - 1] = hal_get_node_id();
	nodes[ncores - 2] = hal_get_node_id();
	TEST_ASSERT((hal_sync_open(&nodes[1], ncores, HAL_SYNC_ALL_TO_ONE)) < 0);
}

/**
 * @brief Fault Injection Test: Synchronization Point Bad Open 
 */
static void test_hal_sync_bad_open(void)
{
	printf("[test][fault injection] Bad Open\n");

	test_hal_sync_bad_open1();
	test_hal_sync_bad_open2();
}

/*===================================================================*
 * Fault Injection Test: Invalid Unlink                              *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Invalid Unlink
 */
static void test_hal_sync_invalid_unlink(void)
{
	printf("[test][fault injection] Invalid Unlink\n");

	TEST_ASSERT(hal_sync_unlink(-1) < 0);
	TEST_ASSERT(hal_sync_unlink(HAL_NR_SYNC) < 0);
	TEST_ASSERT(hal_sync_unlink(HAL_NR_SYNC + 1) < 0);
}

/*===================================================================*
 * Synchronization Point Test Driver                                 *
 *===================================================================*/

/**
 * @brief Synchronization Point Test Driver
 */
int main(int argc, const char **argv)
{
	((void) argc);
	((void) argv);

	hal_setup();

	ncores = hal_get_num_cores();

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, ncores - 1);

	/* API tests. */
	test_hal_sync_create_unlink();
	test_hal_sync_open_close();
	test_hal_sync_wait_signal();
	test_hal_sync_signal_wait();

	/* Fault injection tests. */
	test_hal_sync_invalid_create();
	test_hal_sync_bad_create();
	test_hal_sync_invalid_open();
	test_hal_sync_bad_open();
	test_hal_sync_invalid_unlink();

	hal_cleanup();
	return (EXIT_SUCCESS);
}
