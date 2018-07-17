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

#include <nanvix/const.h>
#include <nanvix/syscalls.h>

#include "test.h"

/*============================================================================*
 * API Test: Create Unlink                                                    *
 *============================================================================*/

/**
 * @brief API Test: Synchronization Point Create Unlink
 */
static void *test_sys_sync_create_unlink_worker(void *args)
{
	int tnum;
	int syncid;

	kernel_setup();

	tnum = ((int *)args)[0];

	nodes[tnum] = sys_get_node_num();

	/*
	 * Wait for nodes list to be initialized. 
	 */
	pthread_barrier_wait(&barrier);

	TEST_ASSERT((syncid = sys_sync_create(nodes, ncores, SYNC_ONE_TO_ALL)) >= 0);

	/*
	 * Wait for all processes to create the 
	 * their synchronization points.
	 */
	pthread_barrier_wait(&barrier);

	TEST_ASSERT(sys_sync_unlink(syncid) == 0);

	kernel_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Create Unlink
 */
static void test_sys_sync_create_unlink(void)
{
	int args[ncores];
	pthread_t tids[ncores];

	nodes[0] = sys_get_node_num();

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		args[i] = i;
		TEST_ASSERT((pthread_create(&tids[i],
			NULL,
			test_sys_sync_create_unlink_worker,
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
static void *test_sys_sync_open_close_worker(void *args)
{
	int tnum;
	int syncid;

	kernel_setup();

	tnum = ((int *)args)[0];

	nodes[tnum] = sys_get_node_num();

	/*
	 * Wait for nodes list to be initialized. 
	 */
	pthread_barrier_wait(&barrier);

	TEST_ASSERT((syncid = sys_sync_create(nodes, ncores, SYNC_ONE_TO_ALL)) >= 0);

	/*
	 * Wait for all processes to open the 
	 * their synchronization points.
	 */
	pthread_barrier_wait(&barrier);

	TEST_ASSERT(sys_sync_unlink(syncid) == 0);

	kernel_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Open Close
 */
static void test_sys_sync_open_close(void)
{
	int args[ncores];
	pthread_t tids[ncores];

	nodes[0] = sys_get_node_num();

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		args[i] = i;
		TEST_ASSERT((pthread_create(&tids[i],
			NULL,
			test_sys_sync_open_close_worker,
			&args[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(tids[i], NULL);
}

/*============================================================================*
 * API Test: Wait Signal                                                      *
 *============================================================================*/

/**
 * @brief API Test: Synchronization Point Wait Signal
 */
static void *test_sys_sync_wait_signal_worker(void *args)
{
	int tnum;
	int syncid;

	kernel_setup();

	tnum = ((int *)args)[0];

	nodes[tnum] = sys_get_node_num();

	/*
	 * Wait for nodes list to be initialized. 
	 */
	pthread_barrier_wait(&barrier);

	if (tnum == 0)
	{
		TEST_ASSERT((syncid = sys_sync_open(&nodes[0], ncores - 1, SYNC_ONE_TO_ALL)) >= 0);

		/*
		 * Wait for all processes to open the 
		 * their synchronization points.
		 */
		pthread_barrier_wait(&barrier);

		TEST_ASSERT(sys_sync_signal(syncid) == 0);

		TEST_ASSERT(sys_sync_close(syncid) == 0);
	}
	else
	{
		TEST_ASSERT((syncid = sys_sync_create(&nodes[0], ncores - 1, SYNC_ONE_TO_ALL)) >= 0);

		/*
		 * Wait for all processes to open the 
		 * their synchronization points.
		 */
		pthread_barrier_wait(&barrier);

		TEST_ASSERT(sys_sync_wait(syncid) == 0);

		TEST_ASSERT(sys_sync_unlink(syncid) == 0);
	}

	kernel_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Wait Signal
 */
static void test_sys_sync_wait_signal(void)
{
	int args[ncores];
	pthread_t tids[ncores];

	nodes[0] = sys_get_node_num();

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		args[i] = i - 1;
		TEST_ASSERT((pthread_create(&tids[i],
			NULL,
			test_sys_sync_wait_signal_worker,
			&args[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(tids[i], NULL);
}

/*============================================================================*
 * API Test: Signal Wait                                                      *
 *============================================================================*/

/**
 * @brief API Test: Synchronization Point Signal Wait
 */
static void *test_sys_sync_signal_wait_worker(void *args)
{
	int tnum;
	int syncid;

	kernel_setup();

	tnum = ((int *)args)[0];

	nodes[tnum] = sys_get_node_num();

	/*
	 * Wait for nodes list to be initialized. 
	 */
	pthread_barrier_wait(&barrier);

	if (tnum == 0)
	{
		TEST_ASSERT((syncid = sys_sync_create(&nodes[0], ncores - 1, SYNC_ALL_TO_ONE)) >= 0);

		/*
		 * Wait for all processes to open the 
		 * their synchronization points.
		 */
		pthread_barrier_wait(&barrier);

		TEST_ASSERT(sys_sync_wait(syncid) == 0);

		TEST_ASSERT(sys_sync_unlink(syncid) == 0);
	}
	else
	{
		TEST_ASSERT((syncid = sys_sync_open(&nodes[0], ncores - 1, SYNC_ALL_TO_ONE)) >= 0);

		/*
		 * Wait for all processes to open the 
		 * their synchronization points.
		 */
		pthread_barrier_wait(&barrier);

		TEST_ASSERT(sys_sync_signal(syncid) == 0);

		TEST_ASSERT(sys_sync_close(syncid) == 0);
	}

	kernel_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Signal Wait
 */
static void test_sys_sync_signal_wait(void)
{
	int args[ncores];
	pthread_t tids[ncores];

	/* Build nodes list. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = sys_get_node_num() + i;

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		args[i] = i - 1;
		TEST_ASSERT((pthread_create(&tids[i],
			NULL,
			test_sys_sync_signal_wait_worker,
			&args[i])) == 0
		);
	}

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
static void test_sys_sync_barrier(void)
{
	int nodenum;
	int syncid;
	int syncid_local;
	int _nodes[2];
	int _nodes_local[2];

	nodenum = sys_get_node_num();

	_nodes[0] = nodenum;
	_nodes[1] = SPAWNER1_SERVER_NODE;

	_nodes_local[0] = SPAWNER1_SERVER_NODE;
	_nodes_local[1] = nodenum;

	/* Open synchronization points. */
	TEST_ASSERT((syncid_local = sys_sync_create(_nodes_local, 2, SYNC_ONE_TO_ALL)) >= 0);
	TEST_ASSERT((syncid = sys_sync_open(_nodes, 2, SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(sys_sync_signal(syncid) == 0);
	TEST_ASSERT(sys_sync_wait(syncid_local) == 0);

	/* House keeping. */
	TEST_ASSERT(sys_sync_unlink(syncid_local) == 0);
	TEST_ASSERT(sys_sync_close(syncid) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_api[] = {
	/* Intra-Cluster API Tests */
	{ test_sys_sync_create_unlink,    "Create Unlink" },
	{ test_sys_sync_open_close,       "Open Close"    },
	{ test_sys_sync_wait_signal,      "Wait Signal"   },
	{ test_sys_sync_signal_wait,      "Signal Wait"   },
	{ test_sys_sync_barrier,          "Barrier Mode"  },
	{ NULL,                           NULL            },
};
