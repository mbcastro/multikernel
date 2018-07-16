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
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <mppaipc.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SYNC_
#include <nanvix/syscalls.h>
#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

#include "test.h"

/*============================================================================*
 * API Test: Name Link Unlink                                                 *
 *============================================================================*/

/**
 * @brief API Test: Name Link Unlink
 */
static void *test_name_thread_link_unlink(void *args)
{
	char pathname[NANVIX_PROC_NAME_MAX];
	int tid;
	int nodeid;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup() == 0);

	pthread_barrier_wait(&ipc_name_barrier);

	tid = ((int *)args)[0];

	nodeid = sys_get_node_id();

	/* Link and unlink name. */
	sprintf(pathname, "cool-name%d", tid);
	TEST_ASSERT(name_link(nodeid, pathname) == 0);

	pthread_barrier_wait(&ipc_name_barrier);

	TEST_ASSERT(name_unlink(pathname) == 0);

	pthread_barrier_wait(&ipc_name_barrier);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return(NULL);
}

/**
 * @brief API Test: Name Link Unlink
 */
static void test_name_link_unlink(void)
{
	int dmas[ipc_name_ncores];
	pthread_t tids[ipc_name_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_name_ncores; i++)
	{
		dmas[i] = i;
		TEST_ASSERT((pthread_create(&tids[i],
			NULL,
			test_name_thread_link_unlink,
			&dmas[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_name_ncores; i++)
		pthread_join(tids[i], NULL);
}

/*============================================================================*
 * API Test: Name Lookup                                                      *
 *============================================================================*/

/**
 * @brief API Test: Master name lookup.
 */
static void *test_name_thread_lookup(void *args)
{
	char pathname[NANVIX_PROC_NAME_MAX];
	int tid;
	int nodeid;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup() == 0);

	pthread_barrier_wait(&ipc_name_barrier);

	tid = ((int *)args)[0];

	nodeid = sys_get_node_id();

	/* Link and unlink name. */
	sprintf(pathname, "cool-name%d", tid);
	TEST_ASSERT(name_link(nodeid, pathname) == 0);

	pthread_barrier_wait(&ipc_name_barrier);

	TEST_ASSERT(name_lookup(pathname) == nodeid);

	pthread_barrier_wait(&ipc_name_barrier);

	TEST_ASSERT(name_unlink(pathname) == 0);

	pthread_barrier_wait(&ipc_name_barrier);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return(NULL);
}

/**
 * @brief API Test: Master name lookup.
 */
static void test_name_lookup(void)
{
	int dmas[ipc_name_ncores];
	pthread_t tids[ipc_name_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_name_ncores; i++)
	{
		dmas[i] = i;
		TEST_ASSERT((pthread_create(&tids[i],
			NULL,
			test_name_thread_lookup,
			&dmas[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_name_ncores; i++)
		pthread_join(tids[i], NULL);
}

/*============================================================================*
 * API Test: slave tests                                                      *
 *============================================================================*/

/**
 * @brief API Test: Slave Tests
 */
static void test_name_slave(void)
{
	int status;
	int pids[NANVIX_PROC_MAX];

	char ipc_name_nclusters_str[4];
	const char *args[] = {
		"/test/name-slave",
		ipc_name_nclusters_str,
		NULL
	};

	printf("[nanvix][test][api] Name Slaves\n");

	sprintf(ipc_name_nclusters_str, "%d", ipc_name_nclusters);

	for (int i = 0; i < ipc_name_nclusters; i++)
		TEST_ASSERT((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);

	for (int i = 0; i < ipc_name_nclusters; i++)
	{
		TEST_ASSERT(mppa_waitpid(pids[i], &status, 0) != -1);
		TEST_ASSERT(status == EXIT_SUCCESS);
	}
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test ipc_name_tests_api[] = {
	{ test_name_link_unlink, "Link Unlink" },
	{ test_name_lookup,      "Lookup"      },
	{ NULL,                  NULL          },
	{ test_name_slave,       "Slave Tests" },
	{ NULL,                  NULL          },
};
