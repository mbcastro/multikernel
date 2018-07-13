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
#include <string.h>

#include <mppa/osconfig.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_PORTAL_
#include <nanvix/hal.h>

#include "../kernel.h"

/**
 * @brief Master node NoC ID.
 */
static int master_node;

/**
 * @brief Underlying NoC node ID.
 */
static int nodeid;

/**
 * @brief Number of benchmark interations.
 */
static int niterations = 0;

/**
 * @brief Buffer size.
 */
static int bufsize = 0;

/**
 * @brief Buffer.
 */
static char buffer[BUFFER_SIZE_MAX];

/*============================================================================*
 * Broadcast Kernel                                                           *
 *============================================================================*/

/**
 * @brief Broadcast kernel. 
 */
static void kernel_broadcast(void)
{
	int inportal;

	/* Open output portal. */
	assert((inportal = hal_portal_create(nodeid)) >= 0);

	/* Benchmark. */
	for (int k = 0; k <= niterations; k++)
	{
		assert(hal_portal_allow(inportal, master_node) == 0);
		assert(hal_portal_read(inportal, buffer, bufsize) == bufsize);
	}

	/* House keeping. */
	assert(hal_portal_unlink(inportal) == 0);
}

/*============================================================================*
 * Gather Kernel                                                              *
 *============================================================================*/

/**
 * @brief Gather kernel. 
 */
static void kernel_gather(void)
{
	int outportal;

	assert((outportal = hal_portal_open(master_node)) >= 0);

	/* Benchmark. */
	for (int k = 0; k <= niterations; k++)
		assert(hal_portal_write(outportal, buffer, bufsize) == bufsize);

	assert(hal_portal_close(outportal) == 0);
}

/*============================================================================*
 * Ping-Pong Kernel                                                           *
 *============================================================================*/

/**
 * @brief Ping-Pong kernel. 
 */
static void kernel_pingpong(void)
{
	int inportal;
	int outportal;

	assert((inportal = hal_portal_create(nodeid)) >= 0);
	assert((outportal = hal_portal_open(master_node)) >= 0);

	/* Benchmark. */
	for (int k = 0; k <= niterations; k++)
	{
		assert(hal_portal_allow(inportal, master_node) == 0);
		assert(hal_portal_read(inportal, buffer, bufsize) == bufsize);
		assert(hal_portal_write(outportal, buffer, bufsize) == bufsize);
	}

	/* House keeping. */
	assert(hal_portal_unlink(inportal) == 0);
	assert(hal_portal_close(outportal) == 0);
}

/*============================================================================*
 * HAL Portal Microbenchmark Driver                                           *
 *============================================================================*/

/**
 * @brief Syncs with remote master.
 *
 * @param first_remote First remote in the nodes list.
 * @param last_remote  Last remote in the nodes list.
 */
static void sync_master(int first_remote, int last_remote)
{
	const int nremotes = last_remote - first_remote;
	int syncid;
	int nodes[nremotes + 1];

	/* Build nodes list. */
	nodes[0] = master_node;
	for (int i = 0; i < nremotes; i++)
		nodes[i + 1] = first_remote + i;

	/* Sync. */
	assert((syncid = hal_sync_open(nodes, nremotes + 1, HAL_SYNC_ALL_TO_ONE)) >= 0);
	assert(hal_sync_signal(syncid) == 0);
	assert(hal_sync_close(syncid) == 0);
}

/**
 * @brief HAL Portal Microbenchmark Driver
 */
int main(int argc, const char **argv)
{
	const char *mode;
	int first_remote;
	int last_remote;
	
	/* Initialization. */
	hal_setup();
	nodeid = hal_get_node_id();

	/* Retrieve kernel parameters. */
	assert(argc == 7);
	master_node = atoi(argv[1]);
	first_remote = atoi(argv[2]);
	last_remote = atoi(argv[3]);
	niterations = atoi(argv[4]);
	bufsize = atoi(argv[5]);
	mode = argv[6];

	sync_master(first_remote, last_remote);

	/* Run kernel. */
	if (!strcmp(mode, "broadcast"))
		kernel_broadcast();
	else if (!strcmp(mode, "gather"))
		kernel_gather();
	else if (!strcmp(mode, "pingpong"))
		kernel_pingpong();

	/* House keeping. */
	hal_cleanup();

	return (EXIT_SUCCESS);
}
