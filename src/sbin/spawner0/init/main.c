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
#include <string.h>

#include <nanvix/const.h>
#include <nanvix/syscalls.h>
#include <nanvix/spawner.h>

/* Forward definitions. */
extern void test_kernel_sys_core(void);
extern void test_kernel_sys_sync(void);
extern void test_kernel_sys_mailbox(void);
extern void test_kernel_sys_portal(void);
extern void test_kernel_name(int);
extern void test_kernel_ipc_mailbox(int);
extern void test_kernel_ipc_barrier(int);

/**
 * @brief Number of servers.
 */
int NR_SERVERS = 0;

/**
 * @brief Servers.
 */
struct serverinfo servers[] = {
};

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
static void test_runtime(const char *module)
{
	if (!strcmp(module, "--name"))
		test_kernel_name(NR_SERVERS);
	else if (!strcmp(module, "--mailbox"))
		test_kernel_ipc_mailbox(NR_SERVERS);
	else if (!strcmp(module, "--barrier"))
		test_kernel_ipc_barrier(NR_SERVERS);
}

/**
 * @brief Sync spawners.
 */
void spawners_sync(void)
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

	/* House keeping. */
	assert(sys_sync_unlink(syncid_local) == 0);
	assert(sys_sync_close(syncid) == 0);
}


SPAWNER_NAME("spawner0")
SPAWNER_MAIN2(main2)
SPAWNER_KERNEL_TESTS(test_kernel)
SPAWNER_RUNTIME_TESTS(test_runtime)
