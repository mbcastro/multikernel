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

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_PORTAL_
#include <nanvix/syscalls.h>

#include "test.h"

#define DATA_SIZE 128

/*============================================================================*
 * API Test: Create Unlink                                                    *
 *============================================================================*/

/**
 * @brief API Test: Portal Create Unlink
 */
static void *test_sys_portal_thread_create_unlink(void *args)
{
	int inportal;
	int nodeid;
	
	((void)args);

	sys_setup();

	pthread_barrier_wait(&barrier);

	nodeid = sys_get_node_id();

	TEST_ASSERT((inportal = sys_portal_create(nodeid)) >= 0);

	pthread_barrier_wait(&barrier);

	TEST_ASSERT(sys_portal_unlink(inportal) == 0);

	sys_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Portal Create Unlink
 */
static void test_sys_portal_create_unlink(void)
{
	int tids[sys_portal_ncores];
	pthread_t threads[sys_portal_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < sys_portal_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_sys_portal_thread_create_unlink,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < sys_portal_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Open Close                                                       *
 *============================================================================*/

/**
 * @brief API Test: Portal Open Close
 */
static void *test_sys_portal_thread_open_close(void *args)
{
	int outportal;
	int tid;
	int nodeid;

	sys_setup();

	pthread_barrier_wait(&barrier);

	tid = ((int *)args)[0];

	nodeid = sys_get_node_id();

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outportal = sys_portal_open(((tid + 1) == sys_portal_ncores) ?
		nodeid + 1 - sys_portal_ncores + 1:
		nodeid + 1)) >= 0);

	pthread_barrier_wait(&barrier);

	TEST_ASSERT(sys_portal_close(outportal) == 0);

	sys_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Portal Open Close
 */
static void test_sys_portal_open_close(void)
{
	int tids[sys_portal_ncores];
	pthread_t threads[sys_portal_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < sys_portal_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_sys_portal_thread_open_close,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < sys_portal_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Read Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Portal Read Write thread
 */
static void *test_sys_portal_thread_read_write(void *args)
{
	int tnum;
	int outportal;
	int inportal;
	char buf[DATA_SIZE];
	int nodeid;
	int clusterid;
	int TID_READ = 1;

	sys_setup();

	pthread_barrier_wait(&barrier);

	tnum = ((int *)args)[0];

	nodeid = sys_get_node_id();
	clusterid = sys_get_cluster_id();

	/* Reader thread */
	if (tnum == 1)
	{
		TEST_ASSERT((inportal = sys_portal_create(nodeid)) >= 0);
		pthread_barrier_wait(&barrier);

		for (int i = 1; i < sys_portal_ncores; i++)
		{
			if (clusterid + i == nodeid)
				continue;

			/* Enables read operations. */
			TEST_ASSERT(sys_portal_allow(inportal, clusterid + i) == 0);

			memset(buf, 0, DATA_SIZE);
			TEST_ASSERT(sys_portal_read(inportal, buf, DATA_SIZE) == DATA_SIZE);

			for (int j = 0; j < DATA_SIZE; j++)
				TEST_ASSERT(buf[j] == 1);
		}

		TEST_ASSERT(sys_portal_unlink(inportal) == 0);
	}
	else
	{
		TEST_ASSERT((outportal = sys_portal_open(clusterid + TID_READ)) >= 0);
		pthread_barrier_wait(&barrier);

		memset(buf, 1, DATA_SIZE);
		TEST_ASSERT(sys_portal_write(outportal, buf, DATA_SIZE) == DATA_SIZE);

		TEST_ASSERT(sys_portal_close(outportal) == 0);
	}

	sys_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Portal Read Write
 */
static void test_sys_portal_read_write(void)
{
	int tids[sys_portal_ncores];
	pthread_t threads[sys_portal_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < sys_portal_ncores; i++)
	{
		tids[i] = i;

		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_sys_portal_thread_read_write,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < sys_portal_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test sys_portal_tests_api[] = {
	{ test_sys_portal_create_unlink, "Create Unlink" },
	{ test_sys_portal_read_write,    "Read Write"    },
	{ test_sys_portal_open_close,    "Open Close"    },
	{ NULL,                           NULL           },
};
