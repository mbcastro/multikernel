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

#include <nanvix/const.h>
#include <nanvix/syscalls.h>

/* Forward definitions. */
extern int main2(int, const char **);
extern void test_kernel_sys_core(void);
extern void test_kernel_sys_sync(void);
extern void test_kernel_sys_mailbox(void);
extern void test_kernel_sys_portal(void);
extern void test_kernel_name(int);
extern void test_kernel_ipc_mailbox(int);
extern void test_kernel_ipc_barrier(int);

/**
 * @brief Generic kernel test driver.
 */
static void test_kernel(const char *module)
{
	if (!strcmp(module, "--hal-core"))
		test_kernel_sys_core();
	else if (!strcmp(module, "--hal-sync"))
		test_kernel_sys_sync();
	else if (!strcmp(module, "--hal-mailbox"))
		test_kernel_sys_mailbox();
	else if (!strcmp(module, "--hal-portal"))
		test_kernel_sys_portal();
}

/**
 * @brief Generic runtime test driver.
 */
static void test_runtime(const char *module, int nservers)
{
	if (!strcmp(module, "--name"))
		test_kernel_name(nservers);
	else if (!strcmp(module, "--mailbox"))
		test_kernel_ipc_mailbox(nservers);
	else if (!strcmp(module, "--barrier"))
		test_kernel_ipc_barrier(nservers);
}

/**
 * @brief Sync spawners.
 */
static void spawners_sync(void)
{
	int nodenum;
	int syncid;
	int syncid_local;
	int nodes[2];
	int nodes_local[2];

	nodenum = sys_get_node_num();

	nodes[0] = nodenum;
	nodes[1] = SPAWNER1_SERVER_NODE;

	nodes_local[0] = SPAWNER1_SERVER_NODE;
	nodes_local[1] = nodenum;

	/* Open synchronization points. */
	assert((syncid_local = sys_sync_create(nodes_local, 2, SYNC_ONE_TO_ALL)) >= 0);
	assert((syncid = sys_sync_open(nodes, 2, SYNC_ONE_TO_ALL)) >= 0);

	assert(sys_sync_signal(syncid) == 0);
	assert(sys_sync_wait(syncid_local) == 0);

	printf("[nanvix][spawner0] synced\n");

	/* House keeping. */
	assert(sys_sync_unlink(syncid_local) == 0);
	assert(sys_sync_close(syncid) == 0);
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

	/* Initialization. */
	assert(kernel_setup() == 0);

	printf("[nanvix][spawner0] booting up server\n");

	/* Run self-tests. */
	if (debug)
		test_kernel(argv[2]);

	printf("[nanvix][spawner0] server alive\n");

	spawners_sync();

	/* Run self-tests. */
	if (debug)
		test_runtime(argv[2], 0);

	printf("[nanvix][spawner0] switching to user mode\n");

	/* Initialization. */
	assert(runtime_setup() == 0);

	ret = main2(argc, argv);	

	/* Cleanup. */
	assert(runtime_cleanup() == 0);

	printf("[nanvix][spawner0] shutting down\n");

	/* Cleanup. */
	assert(kernel_cleanup() == 0);
	return (ret);
}
