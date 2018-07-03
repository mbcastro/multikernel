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
#include <stdlib.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_SYNC_
#include <nanvix/hal.h>

/**
 * @brief Number of cores in the underlying cluster.
 */
static int ncores = 0;

/**
 * @brief Nodes list.
 */
static int nodes[HAL_NR_NOC_NODES];

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Global barrier for synchronization.
 */
static pthread_barrier_t barrier;

/*============================================================================*
 * API Test: Create Unlink                                                    *
 *============================================================================*/

/**
 * @brief API Test: Synchronization Point Create Unlink
 */
static void *test_hal_sync_create_unlink_worker(void *args)
{
	int tnum;
	int syncid;

	hal_setup();

	tnum = ((int *)args)[0];

	nodes[tnum] = hal_get_node_id();

	/*
	 * Wait for nodes list to be initialized. 
	 */
	pthread_barrier_wait(&barrier);

	TEST_ASSERT((syncid = hal_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);

	/*
	 * Wait for all processes to create the 
	 * their synchronization points.
	 */
	pthread_barrier_wait(&barrier);

	TEST_ASSERT(hal_sync_unlink(syncid) == 0);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Create Unlink
 */
static void test_hal_sync_create_unlink(void)
{
	int args[ncores];
	pthread_t tids[ncores];

	printf("[nanvix][test][api] Create Unlink\n");

	nodes[0] = hal_get_node_id();

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		args[i] = i;
		assert((pthread_create(&tids[i],
			NULL,
			test_hal_sync_create_unlink_worker,
			&args[i])) == 0
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
static void *test_hal_sync_open_close_worker(void *args)
{
	int tnum;
	int syncid;

	hal_setup();

	tnum = ((int *)args)[0];

	nodes[tnum] = hal_get_node_id();

	/*
	 * Wait for nodes list to be initialized. 
	 */
	pthread_barrier_wait(&barrier);

	TEST_ASSERT((syncid = hal_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);

	/*
	 * Wait for all processes to open the 
	 * their synchronization points.
	 */
	pthread_barrier_wait(&barrier);

	TEST_ASSERT(hal_sync_unlink(syncid) == 0);

	hal_cleanup();
	return (NULL);
}


/**
 * @brief API Test: Synchronization Point Open Close
 */
static void test_hal_sync_open_close(void)
{
	int args[ncores];
	pthread_t tids[ncores];

	printf("[nanvix][test][api] Open Close\n");

	nodes[0] = hal_get_node_id();

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		args[i] = i;
		assert((pthread_create(&tids[i],
			NULL,
			test_hal_sync_open_close_worker,
			&args[i])) == 0
		);
	}

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
static void *test_hal_sync_wait_signal_worker(void *args)
{
	int tnum;
	int syncid;

	hal_setup();

	tnum = ((int *)args)[0];

	nodes[tnum] = hal_get_node_id();

	/*
	 * Wait for nodes list to be initialized. 
	 */
	pthread_barrier_wait(&barrier);

	if (tnum == 0)
	{
		TEST_ASSERT((syncid = hal_sync_open(&nodes[0], ncores - 1, HAL_SYNC_ONE_TO_ALL)) >= 0);

		/*
		 * Wait for all processes to open the 
		 * their synchronization points.
		 */
		pthread_barrier_wait(&barrier);

		TEST_ASSERT(hal_sync_signal(syncid) == 0);

		TEST_ASSERT(hal_sync_close(syncid) == 0);
	}
	else
	{
		TEST_ASSERT((syncid = hal_sync_create(&nodes[0], ncores - 1, HAL_SYNC_ONE_TO_ALL)) >= 0);

		/*
		 * Wait for all processes to open the 
		 * their synchronization points.
		 */
		pthread_barrier_wait(&barrier);

		TEST_ASSERT(hal_sync_wait(syncid) == 0);

		TEST_ASSERT(hal_sync_unlink(syncid) == 0);
	}

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Wait Signal
 */
static void test_hal_sync_wait_signal(void)
{
	int args[ncores];
	pthread_t tids[ncores];

	printf("[nanvix][test][api] Wait Signal\n");

	nodes[0] = hal_get_node_id();

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		args[i] = i - 1;
		assert((pthread_create(&tids[i],
			NULL,
			test_hal_sync_wait_signal_worker,
			&args[i])) == 0
		);
	}

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
static void *test_hal_sync_signal_wait_worker(void *args)
{
	int tnum;
	int syncid;

	hal_setup();

	tnum = ((int *)args)[0];

	nodes[tnum] = hal_get_node_id();

	/*
	 * Wait for nodes list to be initialized. 
	 */
	pthread_barrier_wait(&barrier);

	if (tnum == 0)
	{
		TEST_ASSERT((syncid = hal_sync_create(&nodes[0], ncores - 1, HAL_SYNC_ALL_TO_ONE)) >= 0);

		/*
		 * Wait for all processes to open the 
		 * their synchronization points.
		 */
		pthread_barrier_wait(&barrier);

		TEST_ASSERT(hal_sync_wait(syncid) == 0);

		TEST_ASSERT(hal_sync_unlink(syncid) == 0);
	}
	else
	{
		TEST_ASSERT((syncid = hal_sync_open(&nodes[0], ncores - 1, HAL_SYNC_ALL_TO_ONE)) >= 0);

		/*
		 * Wait for all processes to open the 
		 * their synchronization points.
		 */
		pthread_barrier_wait(&barrier);

		TEST_ASSERT(hal_sync_signal(syncid) == 0);

		TEST_ASSERT(hal_sync_close(syncid) == 0);
	}

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Signal Wait
 */
static void test_hal_sync_signal_wait(void)
{
	int args[ncores];
	pthread_t tids[ncores];

	printf("[nanvix][test][api] Signal Wait\n");

	/* Build nodes list. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		args[i] = i - 1;
		assert((pthread_create(&tids[i],
			NULL,
			test_hal_sync_signal_wait_worker,
			&args[i])) == 0
		);
	}

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
	printf("[nanvix][test][fault injection] Invalid Create\n");

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
	printf("[nanvix][test][fault injection] Bad Create\n");

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
	printf("[nanvix][test][fault injection] Invalid Open\n");

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
	printf("[nanvix][test][fault injection] Bad Open\n");

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
	printf("[nanvix][test][fault injection] Invalid Unlink\n");

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

	printf("[nanvix][test][fault injection] Bad Unlink\n");

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

	printf("[nanvix][test][fault injection] Double Unlink\n");

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
	printf("[nanvix][test][fault injection] Invalid Close\n");

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

	printf("[nanvix][test][fault injection] Bad Close\n");

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

	printf("[nanvix][test][fault injection] Double Close\n");

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
	printf("[nanvix][test][fault injection] Invalid Signal\n");

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

	printf("[nanvix][test][fault injection] Bad Signal\n");

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
	printf("[nanvix][test][fault injection] Invalid Wait\n");

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

	printf("[nanvix][test][fault injection] Bad Wait\n");

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
void test_hal_sync(void)
{
	ncores = hal_get_num_cores();

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
	test_hal_sync_bad_unlink();
	test_hal_sync_double_unlink();
	test_hal_sync_invalid_close();
	test_hal_sync_bad_close();
	test_hal_sync_double_close();
	test_hal_sync_invalid_signal();
	test_hal_sync_bad_signal();
	test_hal_sync_invalid_wait();
	test_hal_sync_bad_wait();
}
