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

#include <mppa/osconfig.h>
#include <mppaipc.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_PORTAL_
#include <nanvix/hal.h>
#include <nanvix/limits.h>

#include "../kernel.h"

/**
 * @brief Benchmark parameters.
 */
/**@{*/
static int nremotes = 0;        /**< Number of remotes processes.    */
static int niterations = 0;     /**< Number of benchmark parameters. */
static int bufsize = 0;         /**< Buffer size.                    */
static const char *mode = NULL; /**< Benchmark mode.                 */
/**@}*/

/**
 * @brief ID of slave processes.
 */
static int pids[NANVIX_PROC_MAX];

/**
 * @brief Buffer.
 */
static char buffer[BUFFER_SIZE_MAX];

/*============================================================================*
 * Utility                                                                    *
 *============================================================================*/

/**
 * @brief Spawns remote processes.
 */
static void spawn_remotes(void)
{
	int syncid;
	int nodeid;
	char master_node[4];
	char first_remote[4];
	char last_remote[4];
	char niterations_str[4];
	char bufsize_str[20];
	int nodes[nremotes + 1];
	const char *argv[] = {
		"/benchmark/hal-portal-slave",
		master_node,
		first_remote,
		last_remote,
		niterations_str,
		bufsize_str,
		mode,
		NULL
	};

	nodeid = hal_get_node_id();

	/* Build nodes list. */
	nodes[0] = nodeid;
	for (int i = 0; i < nremotes; i++)
		nodes[i + 1] = i;

	/* Create synchronization point. */
	assert((syncid = hal_sync_create(nodes, nremotes + 1, HAL_SYNC_ALL_TO_ONE)) >= 0);

	/* Spawn remotes. */
	sprintf(master_node, "%d", nodeid);
	sprintf(first_remote, "%d", 0);
	sprintf(last_remote, "%d", nremotes);
	sprintf(niterations_str, "%d", niterations);
	sprintf(bufsize_str, "%d", bufsize);
	for (int i = 0; i < nremotes; i++)
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
	for (int i = 0; i < nremotes; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);
}

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

/**
 * @brief Opens output portals.
 *
 * @param outportals Location to store IDs of output portals.
 */
static void open_portals(int *outportals)
{
	/* Open output portales. */
	for (int i = 0; i < nremotes; i++)
		assert((outportals[i] = hal_portal_open(i)) >= 0);
}

/**
 * @brief Closes output portals.
 *
 * @param outportals IDs of target output portals.
 */
static void close_portals(const int *outportals)
{
	/* Close output portals. */
	for (int i = 0; i < nremotes; i++)
		assert((hal_portal_close(outportals[i])) == 0);
}

/**
 * @brief Broadcast kernel.
 */
static void kernel_broadcast(void)
{
	uint64_t total;
	uint64_t t1, t2;
	int outportals[nremotes];

	/* Initialization. */
	open_portals(outportals);
	memset(buffer, 1, bufsize);

	/* Benchmark. */
	for (int k = 0; k <= niterations; k++)
	{
		t1 = hal_timer_get();
		for (int i = 0; i < nremotes; i++)
			assert(hal_portal_write(outportals[i], buffer, bufsize) == bufsize);
		t2 = hal_timer_get();

		total = hal_timer_diff(t1, t2);

		/* Warmup. */
		if (k == 0)
			continue;

		printf("%s;%d;%.2lf;%.2lf\n", 
			mode, 
			bufsize,
			((double)total)/nremotes,
			(nremotes*bufsize)/((double)total)*MEGA
		);
	}

	/* House keeping. */
	close_portals(outportals);
}

/**
 * @brief Gather kernel.
 */
static void kernel_gather(void)
{
	int inportal;
	int nodeid;
	uint64_t total;
	uint64_t t1, t2;

	nodeid = hal_get_node_id();

	/* Initialization. */
	assert((inportal = hal_portal_create(nodeid)) >= 0);

	/* Benchmark. */
	for (int k = 0; k <= niterations; k++)
	{
		t1 = hal_timer_get();
		for (int i = 0; i < nremotes; i++)
		{
			assert(hal_portal_allow(inportal, i) == 0);
			assert(hal_portal_read(inportal, buffer, bufsize) == bufsize);
		}
		t2 = hal_timer_get();

		total = hal_timer_diff(t1, t2);

		/* Warmup. */
		if (k == 0)
			continue;

		printf("%s;%d;%.2lf;%.2lf\n", 
			mode, 
			bufsize,
			((double)total)/nremotes,
			(nremotes*bufsize)/((double)total)*MEGA
		);
	}

	/* House keeping. */
	assert(hal_portal_unlink(inportal) == 0);
}

/**
 * @brief HAL Portal microbenchmark.
 */
static void benchmark(void)
{
	/* Initialization. */
	hal_setup();
	spawn_remotes();

	if (!strcmp(mode, "broadcast"))
		kernel_broadcast();
	else if (!strcmp(mode, "gather"))
		kernel_gather();
	
	/* House keeping. */
	join_remotes();
	hal_cleanup();
}

/*============================================================================*
 * HAL Portal Microbenchmark Driver                                           *
 *============================================================================*/

/**
 * @brief HAL Portal Microbenchmark Driver
 */
int main(int argc, const char **argv)
{
	assert(argc == 5);

	/* Retrieve kernel parameters. */
	nremotes = atoi(argv[1]);
	niterations = atoi(argv[2]);
	bufsize = atoi(argv[3]);
	mode = argv[4];

	/* Parameter checking. */
	assert(niterations > 0);
	assert((bufsize > 0) && (bufsize < (BUFFER_SIZE_MAX)));
	assert((bufsize%2) == 0);

	benchmark();

	return (EXIT_SUCCESS);
}
