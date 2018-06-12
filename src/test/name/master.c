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

#include <mppa/osconfig.h>
#include <mppaipc.h>

#include <nanvix/hal.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>

/**
 * @brief Number of IO clusters.
 */
#define NR_IOCLUSTER 2

/**
 * @brief Number of compute clusters.
 */
#define NR_CCLUSTER 16

/**
 * @brief Number of DMAs per compute cluster.
 */
#define NR_IOCLUSTER_DMA 4

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief ID of slave processes.
 */
static int pids[NR_CCLUSTER];

/*===================================================================*
 * API Test: Name Unlink                                             *
 *===================================================================*/

/**
 * @brief API Test: Name Unlink
 */
static void test_name_unlink(void)
{
	char pathname[PROC_NAME_MAX];

	printf("[test][api] Name Unlink\n");

	/* IO cluster registration test. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
	{
		sprintf(pathname, "/name%d", i);

		/* Remove name. */
		TEST_ASSERT(name_unlink(pathname) == 0);
		TEST_ASSERT(name_lookup(pathname) < 0);
	}
}

/*===================================================================*
 * API Test: Name Link                                               *
 *===================================================================*/

/**
 * @brief API Test: Name Link
 */
static void test_name_link(void)
{
	int nodeid;
	char pathname[PROC_NAME_MAX];

	printf("[test][api] Name Link\n");

	nodeid = hal_get_cluster_id();

	/* IO cluster registration test. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
	{
		sprintf(pathname, "/name%d", i);

		/* Register name. */
		TEST_ASSERT(name_link(nodeid + i, pathname) == 0);
	}
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
	char pathname[PROC_NAME_MAX];

	printf("[test][api] Name Lookup\n");

	nodeid = hal_get_cluster_id();

	/* IO cluster registration test. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
	{
		sprintf(pathname, "/name%d", i);

		TEST_ASSERT(name_lookup(pathname) == nodeid + i);
	}
}

/*===================================================================*
 * API Test: Name Lookup                                             *
 *===================================================================*/

/**
 * @brief API Test: slave name registration.
 */
void test_name_slave(int nclusters)
{
	char nclusters_str[4];
	const char *args[] = {
		"name-slave",
		nclusters_str,
		NULL
	};

	printf("[test][api] Name Slaves\n");

	sprintf(nclusters_str, "%d", nclusters);

	for (int i = 0; i < nclusters; i++)
		TEST_ASSERT((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);

	for (int i = 0; i < nclusters; i++)
		TEST_ASSERT(mppa_waitpid(pids[i], NULL, 0) != -1);
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

	TEST_ASSERT(argc == 2);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);

	/* Wait name server. */
	global_barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(global_barrier);

	/* API tests. */
	test_name_link();
	test_name_lookup();
	test_name_unlink();
	test_name_slave(nclusters);

	/* House keeping. */
	barrier_close(global_barrier);

	return (EXIT_SUCCESS);
}
