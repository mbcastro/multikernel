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
	int nodenum;
	
	((void)args);

	sys_setup();

	pthread_barrier_wait(&barrier);

	nodenum = sys_get_node_num();

	TEST_ASSERT((inportal = sys_portal_create(nodenum)) >= 0);

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
	int nodenum;

	sys_setup();

	pthread_barrier_wait(&barrier);

	tid = ((int *)args)[0];

	nodenum = sys_get_node_num();

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outportal = sys_portal_open(((tid + 1) == sys_portal_ncores) ?
		nodenum + 1 - sys_portal_ncores + 1:
		nodenum + 1)) >= 0);

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
	int nodenum;
	int TID_READ = 1;

	sys_setup();

	pthread_barrier_wait(&barrier);

	tnum = ((int *)args)[0];

	nodenum = sys_get_node_num();

	/* Reader thread */
	if (tnum == TID_READ)
	{
		TEST_ASSERT((inportal = sys_portal_create(nodenum)) >= 0);
		pthread_barrier_wait(&barrier);

		for (int i = 1; i < sys_portal_ncores - 1; i++)
		{
			/* Enables read operations. */
			TEST_ASSERT(sys_portal_allow(inportal, nodenum + i) == 0);

			memset(buf, 0, DATA_SIZE);
			TEST_ASSERT(sys_portal_read(inportal, buf, DATA_SIZE) == DATA_SIZE);

			for (int j = 0; j < DATA_SIZE; j++)
				TEST_ASSERT(buf[j] == 1);
		}

		TEST_ASSERT(sys_portal_unlink(inportal) == 0);
	}
	else
	{
		pthread_barrier_wait(&barrier);
		TEST_ASSERT((outportal = sys_portal_open(nodenum - tnum + TID_READ)) >= 0);

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
