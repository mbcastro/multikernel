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

#include <mppa/osconfig.h>

#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_SYNC_
#include <nanvix/const.h>
#include <nanvix/hal.h>
#include <nanvix/init.h>

/* Forward definitions. */
extern int main2(int, const char **);
extern void test_hal_core(void);
extern void test_hal_sync(void);
extern void test_hal_mailbox(void);
extern void test_hal_portal(void);
extern void test_name(int);
extern void test_ipc_mailbox(int);
extern void test_ipc_barrier(int);

/**
 * @brief Generic kernel test driver.
 */
static void test_kernel(const char *module)
{
	if (!strcmp(module, "--hal-core"))
	{
		test_hal_core();
		exit(EXIT_SUCCESS);
	}
	else if (!strcmp(module, "--hal-sync"))
	{
		test_hal_sync();
		exit(EXIT_SUCCESS);
	}
	else if (!strcmp(module, "--hal-mailbox"))
	{
		test_hal_mailbox();
		exit(EXIT_SUCCESS);
	}
	else if (!strcmp(module, "--hal-portal"))
	{
		test_hal_portal();
		exit(EXIT_SUCCESS);
	}
}

/**
 * @brief Generic runtime test driver.
 */
static void test_runtime(const char *module, int nservers)
{
	if (!strcmp(module, "--name"))
	{
		test_name(nservers);
		exit(EXIT_SUCCESS);
	}
	else if (!strcmp(module, "--mailbox"))
	{
		test_ipc_mailbox(nservers);
		exit(EXIT_SUCCESS);
	}
	else if (!strcmp(module, "--barrier"))
	{
		test_ipc_barrier(nservers);
		exit(EXIT_SUCCESS);
	}
}

/**
 * @brief Sync spawners.
 */
static void spawners_sync(void)
{
	int nodeid;
	int syncid;
	int syncid_local;
	int nodes[2];
	int nodes_local[2];

	nodeid = hal_get_node_id();

	nodes[0] = nodeid;
	nodes[1] = hal_noc_nodes[SPAWNER1_SERVER_NODE];

	nodes_local[0] = hal_noc_nodes[SPAWNER1_SERVER_NODE];
	nodes_local[1] = nodeid;

	/* Open synchronization points. */
	assert((syncid_local = hal_sync_create(nodes_local, 2, HAL_SYNC_ONE_TO_ALL)) >= 0);
	assert((syncid = hal_sync_open(nodes, 2, HAL_SYNC_ONE_TO_ALL)) >= 0);

	assert(hal_sync_signal(syncid) == 0);
	assert(hal_sync_wait(syncid_local) == 0);

	printf("[nanvix][spawner0] synced\n");

	/* House keeping. */
	assert(hal_sync_unlink(syncid_local) == 0);
	assert(hal_sync_close(syncid) == 0);
}

/**
 * @brief Spawns User Application
 */
int main(int argc, const char **argv)
{
	int ret;
	int debug = 0;

	/* Debug mode? */
	if (argc >= 2)
	{
		if (!strcmp(argv[1] , "--debug"))
			debug = 1;
	}

	hal_setup();

	printf("[nanvix][spawner0] booting up server\n");

	/* Run self-tests. */
	if (debug)
		test_kernel(argv[2]);

	printf("[nanvix][spawner0] server alive\n");

	spawners_sync();

	/* Run self-tests. */
	if (debug)
		test_runtime(argv[2], 0);

	/* Initialization. */
	if ((ret = kernel_setup()) != 0)
	{
		printf("[KERNEL] startup failed\n");
		return (EXIT_FAILURE);
	}

	ret = main2(argc, argv);	

	/* Cleanup. */
	if ((ret = kernel_cleanup()) != 0)
	{
		printf("[KERNEL] cleanup failed\n");
		return (EXIT_FAILURE);
	}

	hal_cleanup();
	return (EXIT_SUCCESS);
}
