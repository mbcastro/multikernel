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
#include <stdio.h>
#include <string.h>

#include <mppa/osconfig.h>
#include <mppaipc.h>

#include <nanvix/hal.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>
#include <nanvix/limits.h>

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

/*===================================================================*
 * API Test: Name Link                                               *
 *===================================================================*/

/**
 * @brief API Test: Name Link Unlink
 */
static void test_name_link_unlink(void)
{
	int nodeid;
	char pathname[NANVIX_PROC_NAME_MAX];

	printf("[test][api] Name Link Unlink\n");

	nodeid = hal_get_cluster_id();

	/* Link and unlink name. */
	sprintf(pathname, "cool-name");
	TEST_ASSERT(name_link(nodeid, pathname) == 0);
	TEST_ASSERT(name_unlink(pathname) == 0);
}

/*===================================================================*
 * API Test: Name Lookup                                             *
 *===================================================================*/

/**
 * @brief API Test: master name lookup.
 */
static void test_name_lookup(void)
{
	int nodeid;
	char pathname[NANVIX_PROC_NAME_MAX];

	printf("[test][api] Name Lookup\n");

	nodeid = hal_get_cluster_id();

	/* Lookup name. */
	sprintf(pathname, "cool-name");
	TEST_ASSERT(name_link(nodeid, pathname) == 0);
	TEST_ASSERT(name_lookup(pathname) == nodeid);
	TEST_ASSERT(name_unlink(pathname) == 0);
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

	nodeid = hal_get_cluster_id();

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

	nodeid = hal_get_cluster_id();

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
	int global_barrier;
	int nclusters;

	hal_setup();

	ncores = hal_get_num_cores();

	TEST_ASSERT(argc == 2);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);

	/* Wait name server. */
	global_barrier = barrier_open(0);
	barrier_wait(global_barrier);

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
	barrier_close(global_barrier);

	hal_cleanup();
	return (EXIT_SUCCESS);
}
