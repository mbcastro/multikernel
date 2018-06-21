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
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include <mppa/osconfig.h>
#include <mppaipc.h>

#include <nanvix/config.h>
#include <nanvix/hal.h>
#include <nanvix/pm.h>

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

/*============================================================================*
 * API Test: Create Unlink                                                    *
 *============================================================================*/

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

/*============================================================================*
 * API Test: Open Close                                                       *
 *============================================================================*/

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

/*============================================================================*
 * API Test: Barrier Mode                                                     *
 *============================================================================*/

/**
 * @brief API Test: Barrier Mode
 */
static void test_hal_sync_barrier(void)
{
	int syncid;
	int syncid_local;
	int nodes[2];
	int nodes_local[2];

	printf("[test][api] Barrier Mode\n");

	nodes[0] = 128;
	nodes[1] = 192;

	nodes_local[0] = 192;
	nodes_local[1] = 128;

	/* Open synchronization points. */
	TEST_ASSERT((syncid_local = hal_sync_create(nodes_local, 2, HAL_SYNC_ONE_TO_ALL)) >= 0);
	TEST_ASSERT((syncid = hal_sync_open(nodes, 2, HAL_SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(hal_sync_signal(syncid) == 0);
	TEST_ASSERT(hal_sync_wait(syncid_local) == 0);

	/* House keeping. */
	TEST_ASSERT(hal_sync_unlink(syncid_local) == 0);
	TEST_ASSERT(hal_sync_close(syncid) == 0);
}

/*===================================================================*
 * API Test: Compute Clusters tests                                  *
 *===================================================================*/

/**
 * @brief API Test: Compute Clusters tests
 */
static void test_hal_sync_create_unlink_cc(int nclusters)
{
	int status;
	int pids[nclusters];

	char nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/sync-slave",
		nclusters_str,
		test_str,
		NULL
	};

	printf("[test][api] Compute Clusters Create and Unlink\n");
	printf("[test][api] Compute Clusters Open and Close\n");

	sprintf(nclusters_str, "%d", nclusters);
	sprintf(test_str, "%d", 0);

	for (int i = 0; i < nclusters; i++)
		TEST_ASSERT((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);

	for (int i = 0; i < nclusters; i++)
	{
		TEST_ASSERT(mppa_waitpid(pids[i], &status, 0) != -1);
		TEST_ASSERT(status == EXIT_SUCCESS);
	}
}

/*===================================================================*
 * API Test: Wait Signal IO-Compute cluster                          *
 *===================================================================*/

/**
 * @brief API Test: Synchronization Point Wait Signal
 */
static void test_hal_sync_master_wait_signal_cc(const int *nodes, int nclusters)
{
	int syncid;

	TEST_ASSERT((syncid = hal_sync_open(nodes, nclusters + 1, HAL_SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(hal_sync_signal(syncid) == 0);

	TEST_ASSERT(hal_sync_close(syncid) == 0);
}

/**
 * @brief API Test: Synchronization Point Wait Signal
 */
static void test_hal_sync_wait_signal_cc(int nclusters)
{
	int nodes[nclusters + 1];
	int pids[nclusters];
	int status;

	printf("[test][api] Wait Signal IO / compute cluster\n");

	char nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/sync-slave",
		nclusters_str,
		test_str,
		NULL
	};

	/* Build nodes list. */
	nodes[0] = hal_get_node_id();

	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	sprintf(nclusters_str, "%d", nclusters);
	sprintf(test_str, "%d", 1);

	for (int i = 0; i < nclusters; i++)
		TEST_ASSERT((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);

	test_hal_sync_master_wait_signal_cc(nodes, nclusters);

	for (int i = 0; i < nclusters; i++)
	{
		TEST_ASSERT(mppa_waitpid(pids[i], &status, 0) != -1);
		TEST_ASSERT(status == EXIT_SUCCESS);
	}
}

/*===================================================================*
 * API Test: Signal Wait IO-Compute cluster                          *
 *===================================================================*/

/**
 * @brief API Test: Synchronization Point Signal Wait
 */
static void test_hal_sync_master_signal_wait_cc(const int *nodes, int nclusters)
{
	int syncid;

	TEST_ASSERT((syncid = hal_sync_create(nodes,
		nclusters + 1,
		HAL_SYNC_ALL_TO_ONE)) >= 0
	);

	printf("master blocking\n");
	TEST_ASSERT(hal_sync_wait(syncid) == 0);
	printf("master UNBLOCKING\n");

	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
}

/**
 * @brief API Test: Synchronization Point Wait Signal
 */
static void test_hal_sync_signal_wait_cc(int nclusters)
{
	int nodes[nclusters + 1];
	int pids[nclusters];
	int status;

	printf("[test][api] Signal Wait IO / compute cluster\n");

	char nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/sync-slave",
		nclusters_str,
		test_str,
		NULL
	};

	/* Build nodes list. */
	nodes[0] = hal_get_node_id();

	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	sprintf(nclusters_str, "%d", nclusters);
	sprintf(test_str, "%d", 2);

	for (int i = 0; i < nclusters; i++)
		TEST_ASSERT((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);

	test_hal_sync_master_signal_wait_cc(nodes, nclusters);

	for (int i = 0; i < nclusters; i++)
	{
		TEST_ASSERT(mppa_waitpid(pids[i], &status, 0) != -1);
		TEST_ASSERT(status == EXIT_SUCCESS);
	}
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
	TEST_ASSERT(hal_sync_unlink(1) < 0);
	TEST_ASSERT(hal_sync_unlink(HAL_NR_SYNC) < 0);
	TEST_ASSERT(hal_sync_unlink(HAL_NR_SYNC + 1) < 0);
}

/*===================================================================*
 * Fault Injection Test: Bad Unlink                                  *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Bad Unlink
 */
static void test_hal_sync_bad_unlink(void)
{
	int syncid;
	int nodes[ncores];

	printf("[test][fault injection] Bad Unlink\n");

	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	TEST_ASSERT((syncid = hal_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(hal_sync_unlink(syncid) < 0);
	TEST_ASSERT(hal_sync_close(syncid) == 0);
}

/*===================================================================*
 * Fault Injection Test: Double Unlink                               *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Double Unlink
 */
static void test_hal_sync_double_unlink(void)
{
	int syncid;
	int nodes[ncores];

	printf("[test][fault injection] Double Unlink\n");

	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	TEST_ASSERT((syncid = hal_sync_create(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) >= 0);
	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
	TEST_ASSERT(hal_sync_unlink(syncid) < 0);
}

/*===================================================================*
 * Fault Injection Test: Invalid Close                               *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Invalid Close
 */
static void test_hal_sync_invalid_close(void)
{
	printf("[test][fault injection] Invalid Close\n");

	TEST_ASSERT(hal_sync_close(-1) < 0);
	TEST_ASSERT(hal_sync_close(1) < 0);
	TEST_ASSERT(hal_sync_close(HAL_NR_SYNC) < 0);
	TEST_ASSERT(hal_sync_close(HAL_NR_SYNC + 1) < 0);
}

/*===================================================================*
 * Fault Injection Test: Bad Close                                   *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Bad Close
 */
static void test_hal_sync_bad_close(void)
{
	int syncid;
	int nodes[ncores];

	printf("[test][fault injection] Bad Close\n");

	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	TEST_ASSERT((syncid = hal_sync_create(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) >= 0);

	TEST_ASSERT(hal_sync_close(syncid) < 0);
	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
}

/*===================================================================*
 * Fault Injection Test: Double Close                                *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Double Close
 */
static void test_hal_sync_double_close(void)
{
	int syncid;
	int nodes[ncores];

	printf("[test][fault injection] Double Close\n");

	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	TEST_ASSERT((syncid = hal_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);
	TEST_ASSERT(hal_sync_close(syncid) == 0);
	TEST_ASSERT(hal_sync_close(syncid) < 0);
}

/*===================================================================*
 * Fault Injection Test: Invalid Signal                              *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Invalid Signal
 */
static void test_hal_sync_invalid_signal(void)
{
	printf("[test][fault injection] Invalid Signal\n");

	TEST_ASSERT(hal_sync_signal(-1) < 0);
	TEST_ASSERT(hal_sync_signal(1) < 0);
	TEST_ASSERT(hal_sync_signal(HAL_NR_SYNC) < 0);
	TEST_ASSERT(hal_sync_signal(HAL_NR_SYNC + 1) < 0);
}

/*===================================================================*
 * Fault Injection Test: Bad Signal                                  *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Bad Signal
 */
static void test_hal_sync_bad_signal(void)
{
	int syncid;
	int nodes[ncores];

	printf("[test][fault injection] Bad Signal\n");

	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	TEST_ASSERT((syncid = hal_sync_create(nodes, ncores, HAL_SYNC_ALL_TO_ONE)) >= 0);

	TEST_ASSERT(hal_sync_signal(syncid) < 0);
	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
}

/*===================================================================*
 * Fault Injection Test: Invalid Wait                                *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Invalid Wait
 */
static void test_hal_sync_invalid_wait(void)
{
	printf("[test][fault injection] Invalid Wait\n");

	TEST_ASSERT(hal_sync_wait(-1) < 0);
	TEST_ASSERT(hal_sync_wait(1) < 0);
	TEST_ASSERT(hal_sync_wait(HAL_NR_SYNC) < 0);
	TEST_ASSERT(hal_sync_wait(HAL_NR_SYNC + 1) < 0);
}

/*===================================================================*
 * Fault Injection Test: Bad Wait                                    *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Synchronization Point Bad Wait
 */
static void test_hal_sync_bad_wait(void)
{
	int syncid;
	int nodes[ncores];

	printf("[test][fault injection] Bad Wait\n");

	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	TEST_ASSERT((syncid = hal_sync_open(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(hal_sync_wait(syncid) < 0);
	TEST_ASSERT(hal_sync_close(syncid) == 0);
}

/*===================================================================*
 * Synchronization Point Test Driver                                 *
 *===================================================================*/

/**
 * @brief Synchronization Point Test Driver
 */
int main(int argc, const char **argv)
{
	int nclusters;

	hal_setup();

	TEST_ASSERT(argc == 2);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);

	ncores = hal_get_num_cores();

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, ncores - 1);

	/* API tests. */
	test_hal_sync_create_unlink();
	test_hal_sync_open_close();
	test_hal_sync_wait_signal();
	test_hal_sync_signal_wait();
	test_hal_sync_barrier();
	test_hal_sync_create_unlink_cc(nclusters);
	test_hal_sync_wait_signal_cc(nclusters);
	test_hal_sync_signal_wait_cc(nclusters);

	/* Fault injection tests. */
	test_hal_sync_invalid_create();
	test_hal_sync_bad_create();
	test_hal_sync_invalid_open();
	test_hal_sync_bad_open();
	test_hal_sync_invalid_unlink();
	test_hal_sync_bad_unlink();
	test_hal_sync_double_unlink();
	test_hal_sync_invalid_close();
	test_hal_sync_bad_close();
	test_hal_sync_double_close();
	test_hal_sync_invalid_signal();
	test_hal_sync_bad_signal();
	test_hal_sync_invalid_wait();
	test_hal_sync_bad_wait();

	hal_cleanup();
	return (EXIT_SUCCESS);
}
