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
#include <pthread.h>

#include <mppa/osconfig.h>
#include <mppaipc.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_MAILBOX_
#include <nanvix/hal.h>
#include <nanvix/limits.h>

#include "../kernel.h"

/**
 * @brief Benchmark parameters.
 */
/**@{*/
static int nclusters = 0;         /**< Number of remotes processes.    */
static int niterations = 0;       /**< Number of benchmark parameters. */
static const char *kernel = NULL; /**< Benchmark kernel.               */
/**@}*/

/**
 * @brief ID of slave processes.
 */
static int pids[NANVIX_PROC_MAX];

/**
 * @brief Buffer.
 */
static char buffer[HAL_MAILBOX_MSG_SIZE];

/**
 * @brief Underlying NoC node ID.
 */
static int nodeid;

/**
 * @brief Inbox for receiving messages.
 */
static int inbox;

/*============================================================================*
 * Utility                                                                    *
 *============================================================================*/

/**
 * @brief Spawns remote processes.
 */
static void spawn_remotes(void)
{
	int syncid;
	char master_node[4];
	char first_remote[4];
	char last_remote[4];
	char niterations_str[4];
	int nodes[nclusters + 1];
	const char *argv[] = {
		"/benchmark/hal-mailbox-slave",
		master_node,
		first_remote,
		last_remote,
		niterations_str,
		kernel,
		NULL
	};

	/* Build nodes list. */
	nodes[0] = nodeid;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create synchronization point. */
	assert((syncid = hal_sync_create(nodes, nclusters + 1, HAL_SYNC_ALL_TO_ONE)) >= 0);

	/* Spawn remotes. */
	sprintf(master_node, "%d", nodeid);
	sprintf(first_remote, "%d", 0);
	sprintf(last_remote, "%d", nclusters);
	sprintf(niterations_str, "%d", niterations);
	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);

	/* Sync. */
	assert(hal_sync_wait(syncid) == 0);

	/* House keeping. */
	assert(hal_sync_unlink(syncid) == 0);
}

/**
 * @brief Wait for remote processes.
 */
static void join_remotes(void)
{
	for (int i = 0; i < nclusters; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);
}

/*============================================================================*
 * Kernels                                                                    *
 *============================================================================*/

/**
 * @brief Opens output mailboxes.
 *
 * @param outboxes Location to store IDs of output mailboxes.
 */
static void open_mailboxes(int *outboxes)
{
	/* Open output portales. */
	for (int i = 0; i < nclusters; i++)
		assert((outboxes[i] = hal_mailbox_open(i)) >= 0);
}

/**
 * @brief Closes output mailboxes.
 *
 * @param outboxes IDs of target output mailboxes.
 */
static void close_mailboxes(const int *outboxes)
{
	/* Close output mailboxes. */
	for (int i = 0; i < nclusters; i++)
		assert((hal_mailbox_close(outboxes[i])) == 0);
}

/**
 * @brief Broadcast kernel.
 */
static void kernel_broadcast(void)
{
	int outboxes[nclusters];

	/* Initialization. */
	open_mailboxes(outboxes);
	memset(buffer, 1, HAL_MAILBOX_MSG_SIZE);

	/* Benchmark. */
	for (int k = 0; k <= niterations; k++)
	{
		double total;
		uint64_t t1, t2;

		t1 = hal_timer_get();
		for (int i = 0; i < nclusters; i++)
			assert(hal_mailbox_write(outboxes[i], buffer, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);
		t2 = hal_timer_get();

		total = hal_timer_diff(t1, t2)/((double) hal_get_core_freq());

		/* Warmup. */
		if (k == 0)
			continue;

		printf("nanvix;%s;%d;%d;%.2lf;%.2lf\n",
			kernel,
			HAL_MAILBOX_MSG_SIZE,
			nclusters,
			(total*MEGA)/nclusters,
			(nclusters*HAL_MAILBOX_MSG_SIZE)/total
		);
	}
	
	/* Close output mailboxes. */
	close_mailboxes(outboxes);
}

/**
 * @brief Gather kernel.
 */
static void kernel_gather(void)
{
	double total;
	uint64_t t1, t2;

	/* Benchmark. */
	for (int k = 0; k <= niterations; k++)
	{
		t1 = hal_timer_get();
		for (int i = 0; i < nclusters; i++)
			assert(hal_mailbox_read(inbox, buffer, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);
		t2 = hal_timer_get();

		total = hal_timer_diff(t1, t2)/((double) hal_get_core_freq());

		/* Warmup. */
		if (k == 0)
			continue;

		printf("nanvix;%s;%d;%d;%.2lf;%.2lf\n",
			kernel,
			HAL_MAILBOX_MSG_SIZE,
			nclusters,
			(total*MEGA)/nclusters,
			(nclusters*HAL_MAILBOX_MSG_SIZE)/total
		);
	}
}

/**
 * @brief HAL Mailbox microbenchmark.
 */
static void benchmark(void)
{
	/* Initialization. */
	hal_setup();
	nodeid = hal_get_node_id();
	assert((inbox = hal_mailbox_create(nodeid)) >= 0);
	spawn_remotes();

	if (!strcmp(kernel, "broadcast"))
		kernel_broadcast();
	else if (!strcmp(kernel, "gather"))
		kernel_gather();
	
	/* House keeping. */
	assert(hal_mailbox_unlink(inbox) == 0);
	join_remotes();
	hal_cleanup();
}

/*============================================================================*
 * HAL Mailbox Microbenchmark Driver                                          *
 *============================================================================*/

/**
 * @brief HAL Mailbox Microbenchmark Driver
 */
int main(int argc, const char **argv)
{
	assert(argc == 4);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);
	niterations = atoi(argv[2]);
	kernel = argv[3];

	/* Parameter checking. */
	assert(niterations > 0);

	benchmark();

	return (EXIT_SUCCESS);
}
