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

/* Forward definitions. */
extern int name_server(int);
extern void test_kernel_sys_sync(void);
extern void test_kernel_sys_mailbox(void);
extern void test_kernel_barrier(void);

/**
 * @brief Number of servers.
 */
int NR_SERVERS = 1;

int usermode = 0;

struct server
{
	int (*main) (int);
	int nodenum;
};

/**
 * @brief Servers.
 */
struct server servers[] = {
	{ name_server, NAME_SERVER_NODE },
};

/**
 * @brief Generic test driver.
 */
void test_kernel(const char *module)
{
	if (!strcmp(module, "--hal-sync"))
		test_kernel_sys_sync();
	else if (!strcmp(module, "--hal-mailbox"))
		test_kernel_sys_mailbox();
}

/**
 * @brief Generic test driver.
 */
void test_runtime(const char *module)
{
	if (!strcmp(module, "--barrier"))
		test_kernel_barrier();
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
	nodes[1] = SPAWNER_SERVER_NODE;

	nodes_local[0] = SPAWNER_SERVER_NODE;
	nodes_local[1] = nodenum;

	/* Open syncrhonization points. */
	assert((syncid_local = sys_sync_create(nodes_local, 2, SYNC_ONE_TO_ALL)) >= 0);
	assert((syncid = sys_sync_open(nodes, 2, SYNC_ONE_TO_ALL)) >= 0);

	assert(sys_sync_wait(syncid_local) == 0);
	assert(sys_sync_signal(syncid) == 0);

	printf("[nanvix][spawner1] synced\n");

	/* House keeping. */
	assert(sys_sync_unlink(syncid_local) == 0);
	assert(sys_sync_close(syncid) == 0);
}

int main2(int argc, const char *argv)
{
	((void) argc);
	((void) argv);

	return (EXIT_SUCCESS);
}
