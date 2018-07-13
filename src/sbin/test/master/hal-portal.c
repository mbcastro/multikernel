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

#include <stdlib.h>
#include <stdio.h>

#include <mppaipc.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_PORTAL_
#include <nanvix/hal.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Data Size.
 */
#define DATA_SIZE 128

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

/**
 * @brief PID of slaves.
 */
static int pids[NANVIX_PROC_MAX];

/**
 * @brief Spawn slave processes.
 */
static void spawn_slaves(const char **args)
{
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		TEST_ASSERT((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);
}

/**
 * @brief Wait for slaves to terminate.
 */
static void join_slaves(void)
{
	int status;

	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		TEST_ASSERT(mppa_waitpid(pids[i], &status, 0) != -1);
		TEST_ASSERT(status == EXIT_SUCCESS);
	}
}

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_hal_portal_create_unlink_cc(void)
{
	char masternode_str[4];
	char portal_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-portal-slave",
		masternode_str,
		portal_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][portal] Create Unlink CC\n");

	/* Build arguments. */
	sprintf(masternode_str, "%d", hal_get_node_id());
	sprintf(portal_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 0);

	spawn_slaves(args);
	join_slaves();
}

/*============================================================================*
 * API Test: Open Close CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Open Close CC
 */
static void test_hal_portal_open_close_cc(void)
{
	char masternode_str[4];
	char portal_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-portal-slave",
		masternode_str,
		portal_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][portal] Open Close CC\n");

	/* Build arguments. */
	sprintf(masternode_str, "%d", hal_get_node_id());
	sprintf(portal_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 1);

	spawn_slaves(args);
	join_slaves();
}

/*============================================================================*
 * API Test: Read Write CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Read Write CC
 */
static void test_hal_portal_read_write_cc(void)
{
	char masternode_str[4];
	char portal_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-portal-slave",
		masternode_str,
		portal_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][portal] Read Write CC\n");

	/* Build arguments. */
	sprintf(masternode_str, "%d", hal_get_node_id());
	sprintf(portal_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 2);

	spawn_slaves(args);
	join_slaves();
}

/*============================================================================*
 * API Test: Read Write 2 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Read Write 2 CC
 */
static void test_hal_portal_read_write2_cc(void)
{
	int syncid;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char portal_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-portal-slave",
		masternode_str,
		portal_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][portal] Read Write 2 CC\n");

	/* Build arguments. */
	sprintf(masternode_str, "%d", hal_get_node_id());
	sprintf(portal_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 3);

	/* Build nodes list. */
	nodes[0] = hal_get_node_id();
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	/* Create synchronization point. */
	TEST_ASSERT((syncid = hal_sync_create(
		nodes,
		NANVIX_PROC_MAX + 1,
		HAL_SYNC_ALL_TO_ONE)) >= 0
	);

	spawn_slaves(args);

	/* Wait. */
	TEST_ASSERT(hal_sync_wait(syncid) == 0);

	/* Send messages. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		int outportal;
		char buffer[DATA_SIZE];

		TEST_ASSERT((outportal = hal_portal_open(i)) >=0);
		TEST_ASSERT((hal_portal_write(
			outportal,
			buffer,
			DATA_SIZE) == DATA_SIZE)
		);
		TEST_ASSERT(hal_portal_close(outportal) == 0);
	}

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
}

/*============================================================================*/

/**
 * @brief Automated HAL portal test driver.
 */
void test_hal_portal(void)
{
	test_hal_portal_create_unlink_cc();
	test_hal_portal_open_close_cc();
	test_hal_portal_read_write_cc();
	test_hal_portal_read_write2_cc();
}

