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
#include <stdlib.h>
#include <stdio.h>

#include <mppaipc.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SYNC_
#include <nanvix/hal.h>
#include <nanvix/limits.h>

/**
 * @brief Nodes list.
 */
static int nodes[NANVIX_PROC_MAX + 1];

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
		assert((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);
}

/**
 * @brief Wait for slaves to terminate.
 */
static void join_slaves(void)
{
	int status;

	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		assert(mppa_waitpid(pids[i], &status, 0) != -1);
		assert(status == EXIT_SUCCESS);
	}
}

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_hal_sync_create_unlink_cc(void)
{
	char masternode_str[4];
	char sync_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-sync-slave",
		masternode_str,
		sync_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][sync] Create Unlink CC\n");

	/* Build arguments. */
	sprintf(masternode_str, "%d", hal_get_node_id());
	sprintf(sync_nclusters_str, "%d", NANVIX_PROC_MAX);
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
static void test_hal_sync_open_close_cc(void)
{
	char masternode_str[4];
	char sync_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-sync-slave",
		masternode_str,
		sync_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][sync] Open Close CC\n");

	/* Build arguments. */
	sprintf(masternode_str, "%d", hal_get_node_id());
	sprintf(sync_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 1);

	spawn_slaves(args);
	join_slaves();
}

/*============================================================================*
 * API Test: Wait Signal CC                                                   *
 *============================================================================*/

/**
 * @brief API Test: Wait Signal CC
 */
static void test_hal_sync_wait_signal_cc(void)
{
	int syncid;
	char masternode_str[4];
	char sync_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-sync-slave",
		masternode_str,
		sync_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][sync] Wait Signal CC\n");

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodes[0]);
	sprintf(sync_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 2);

	spawn_slaves(args);

	assert((syncid = hal_sync_open(nodes, NANVIX_PROC_MAX + 1, HAL_SYNC_ONE_TO_ALL)) >= 0);
	assert(hal_sync_signal(syncid) == 0);
	assert(hal_sync_close(syncid) == 0);

	join_slaves();
}

/*============================================================================*
 * API Test: Signal Wait CC                                                   *
 *============================================================================*/

/**
 * @brief API Test: Signal Wait CC
 */
static void test_hal_sync_signal_wait_cc(void)
{
	int syncid;
	char masternode_str[4];
	char sync_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-sync-slave",
		masternode_str,
		sync_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][sync] Signal Wait CC\n");

	/* Create synchronization point. */
	assert((syncid = hal_sync_create(
		nodes,
		NANVIX_PROC_MAX + 1,
		HAL_SYNC_ALL_TO_ONE)) >= 0
	);

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodes[0]);
	sprintf(sync_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 3);

	spawn_slaves(args);

	/* Wait. */
	assert(hal_sync_wait(syncid) == 0);

	join_slaves();

	/* House keeping. */
	assert(hal_sync_unlink(syncid) == 0);
}

/*============================================================================*
 * API Test: Barrier CC                                                       *
 *============================================================================*/

/**
 * @brief API Test: Barrier CC
 */
static void test_hal_sync_barrier_cc(void)
{
	int syncid1, syncid2;
	char masternode_str[4];
	char sync_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-sync-slave",
		masternode_str,
		sync_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][sync] Barrier CC\n");

	/* Create synchronization point. */
	assert((syncid1 = hal_sync_create(
		nodes,
		NANVIX_PROC_MAX + 1,
		HAL_SYNC_ALL_TO_ONE)) >= 0
	);
	assert((syncid2 = hal_sync_open(
		nodes,
		NANVIX_PROC_MAX + 1,
		HAL_SYNC_ONE_TO_ALL)) >= 0
	);

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodes[0]);
	sprintf(sync_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 4);

	spawn_slaves(args);

	/* Wait. */
	assert(hal_sync_wait(syncid1) == 0);
	assert(hal_sync_signal(syncid2) == 0);

	join_slaves();

	/* House keeping. */
	assert(hal_sync_close(syncid2) == 0);
	assert(hal_sync_unlink(syncid1) == 0);
}

/*============================================================================*
 * API Test: Barrier 2 CC                                                     *
 *============================================================================*/

/**
 * @brief API Test: Barrier 2 CC
 */
static void test_hal_sync_barrier2_cc(void)
{
	char masternode_str[4];
	char sync_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/hal-sync-slave",
		masternode_str,
		sync_nclusters_str,
		test_str,
		NULL
	};

	printf("[nanvix][test][api][hal][sync] Barrier 2 CC\n");

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodes[0]);
	sprintf(sync_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 5);

	spawn_slaves(args);

	join_slaves();
}

/*============================================================================*/

/**
 * @brief Automated HAL sync test driver.
 */
void test_hal_sync(void)
{
	/* Build nodes list. */
	nodes[0] = hal_get_node_id();
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;
	
	test_hal_sync_create_unlink_cc();
	test_hal_sync_open_close_cc();
	test_hal_sync_wait_signal_cc();
	test_hal_sync_signal_wait_cc();
	test_hal_sync_barrier_cc();
	test_hal_sync_barrier2_cc();
}
