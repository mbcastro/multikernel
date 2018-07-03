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
#include <string.h>
#include <stdlib.h>

#include <mppa/osconfig.h>
#include <mppaipc.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SYNC_
#include <nanvix/hal.h>
#include <nanvix/init.h>
#include <nanvix/name.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Number of cores in the underlying cluster.
 */
static int ncores = 0;

/**
 * @brief ID of slave processes.
 */
static int pids[NANVIX_PROC_MAX];

/**
 * @brief Global barrier for synchronization.
 */
static pthread_barrier_t barrier;

/*===================================================================*
 * API Test: Name Link Unlink                                        *
 *===================================================================*/

/**
 * @brief API Test: Name Link Unlink
 */
static void *test_name_thread_link_unlink(void *args)
{
	char pathname[NANVIX_PROC_NAME_MAX];
	int tid;
	int nodeid;

	TEST_ASSERT(kernel_setup() == 0);

	pthread_barrier_wait(&barrier);

	tid = ((int *)args)[0];

	nodeid = hal_get_node_id();

	/* Link and unlink name. */
	sprintf(pathname, "cool-name%d", tid);
	TEST_ASSERT(name_link(nodeid, pathname) == 0);

	pthread_barrier_wait(&barrier);

	TEST_ASSERT(name_unlink(pathname) == 0);

	TEST_ASSERT(kernel_cleanup() == 0);
	return(NULL);
}

/**
 * @brief API Test: Name Link Unlink
 */
static void test_name_link_unlink(void)
{
	int dmas[ncores];
	pthread_t tids[ncores];

	printf("[test][api] Name Link Unlink\n");

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		dmas[i] = i;
		assert((pthread_create(&tids[i],
			NULL,
			test_name_thread_link_unlink,
			&dmas[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * API Test: Name Lookup                                             *
 *===================================================================*/

/**
 * @brief API Test: Master name lookup.
 */
static void *test_name_thread_lookup(void *args)
{
	char pathname[NANVIX_PROC_NAME_MAX];
	int tid;
	int nodeid;

	TEST_ASSERT(kernel_setup() == 0);

	pthread_barrier_wait(&barrier);

	tid = ((int *)args)[0];

	nodeid = hal_get_node_id();

	/* Link and unlink name. */
	sprintf(pathname, "cool-name%d", tid);
	TEST_ASSERT(name_link(nodeid, pathname) == 0);

	pthread_barrier_wait(&barrier);

	TEST_ASSERT(name_lookup(pathname) == nodeid);

	TEST_ASSERT(name_unlink(pathname) == 0);

	TEST_ASSERT(kernel_cleanup() == 0);
	return(NULL);
}

/**
 * @brief API Test: Master name lookup.
 */
static void test_name_lookup(void)
{
	int dmas[ncores];
	pthread_t tids[ncores];

	printf("[test][api] Name Lookup\n");

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		dmas[i] = i;
		assert((pthread_create(&tids[i],
			NULL,
			test_name_thread_lookup,
			&dmas[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
* Fault Injection Test: Duplicate Name                               *
*====================================================================*/

/**
* @brief Fault Injection Test: Link the Same Name Twice
*/
static void test_name_duplicate(void)
{
	int nodeid;
	char pathname[NANVIX_PROC_NAME_MAX];

	printf("[test][fault injection] Duplicate Name\n");

	nodeid = hal_get_node_id();

	/* Link name. */
	sprintf(pathname, "cool-name");
	TEST_ASSERT(name_link(nodeid, pathname) == 0);
	TEST_ASSERT(name_link(nodeid, pathname) < 0);
	TEST_ASSERT(name_unlink(pathname) == 0);
}

/*===================================================================*
* Fault Injection Test: Invalid Link                                 *
*====================================================================*/

/**
* @brief Fault Injection Test: Link Invalid Names
*/
static void test_name_invalid_link(void)
{
	int nodeid;
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	printf("[test][fault injection] Invalid Link\n");

	nodeid = hal_get_node_id();

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	/* Link invalid names. */
	TEST_ASSERT(name_link(nodeid, pathname) < 0);
	TEST_ASSERT(name_link(nodeid, NULL) < 0);
	TEST_ASSERT(name_link(nodeid, "") < 0);
}

/*===================================================================*
* Fault Injection Test: Invalid Unlink                               *
*====================================================================*/

/**
* @brief Fault Injection Test: Unlink Invalid Name
*/
static void test_name_invalid_unlink(void)
{
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	printf("[test][fault onjection] Invalid Unlink\n");

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	/* Unlink invalid names. */
	TEST_ASSERT(name_unlink(pathname) < 0);
	TEST_ASSERT(name_unlink(NULL) < 0);
	TEST_ASSERT(name_unlink("") < 0);
}

/*===================================================================*
* Fault Injection Test: Bad Unlink                                   *
*====================================================================*/

/**
* @brief Fault Injection Test: Unlink Bad Name
*/
static void test_name_bad_unlink(void)
{
	printf("[test][fault injection] Bad Unlink\n");

	/* Unlink missing name. */
	TEST_ASSERT(name_unlink("missing_name") < 0);
}

/*===================================================================*
* Fault Injection Test: Bad Lookup                                   *
*====================================================================*/

/**
* @brief Fault Injection Test: Lookup Missing Name
*/
static void test_name_bad_lookup(void)
{
	printf("[test][fault injection] Bad Lookup\n");

	/* Lookup missing name. */
	TEST_ASSERT(name_lookup("missing_name") < 0);
}

/*===================================================================*
* Fault Injection Test: Invalid Lookup                               *
*====================================================================*/

/**
* @brief Fault Injection Test: Lookup Invalid Name
*/
static void test_name_invalid_lookup(void)
{
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	printf("[test][fault injection] Invalid Lookup\n");

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	/* Lookup invalid names. */
	TEST_ASSERT(name_lookup(pathname) < 0);
	TEST_ASSERT(name_lookup(NULL) < 0);
	TEST_ASSERT(name_lookup("") < 0);
}

/*===================================================================*
 * API Test: slave tests                                             *
 *===================================================================*/

/**
 * @brief API Test: Slave Tests
 */
static void test_name_slave(int nclusters)
{
	int status;

	char nclusters_str[4];
	const char *args[] = {
		"/test/name-slave",
		nclusters_str,
		NULL
	};

	printf("[test][api] Name Slaves\n");

	sprintf(nclusters_str, "%d", nclusters);

	for (int i = 0; i < nclusters; i++)
		TEST_ASSERT((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);

	for (int i = 0; i < nclusters; i++)
	{
		TEST_ASSERT(mppa_waitpid(pids[i], &status, 0) != -1);
		TEST_ASSERT(status == EXIT_SUCCESS);
	}
}

/*===================================================================*
 * Name Service Test Driver                                          *
 *===================================================================*/

/**
 * @brief Name Service Test Driver
 */
int main(int argc, char **argv)
{
	int syncid;
	int nodes[2];
	int nclusters;

	TEST_ASSERT(kernel_setup() == 0);

	ncores = hal_get_num_cores();

	pthread_barrier_init(&barrier, NULL, ncores - 1);

	TEST_ASSERT(argc == 2);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);

	/* Wait spawner server. */
	nodes[0] = 128;
	nodes[1] = hal_get_node_id();

	TEST_ASSERT((syncid = hal_sync_create(nodes, 2, HAL_SYNC_ONE_TO_ALL)) >= 0);
	TEST_ASSERT(hal_sync_wait(syncid) == 0);

	/* API tests. */
	test_name_link_unlink();
	test_name_lookup();

	/* Fault injection tests. */
	test_name_duplicate();
	test_name_invalid_link();
	test_name_invalid_unlink();
	test_name_bad_unlink();
	test_name_bad_lookup();
	test_name_invalid_lookup();
	test_name_slave(nclusters);

	/* House keeping. */
	TEST_ASSERT(hal_sync_unlink(syncid) == 0);

	TEST_ASSERT(kernel_cleanup() == 0);
	return (EXIT_SUCCESS);
}
