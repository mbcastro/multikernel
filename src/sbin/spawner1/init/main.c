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
#include <stddef.h>
#include <string.h>

#include <nanvix/const.h>
#include <nanvix/syscalls.h>
#include <nanvix/spawner.h>

/**
 * @brief Number of servers launched from this spawner.
 */
#define NR_SERVERS 1

/**
 * @brief Input mailbox.
 */
static int inbox = -1;

/**
 * @brief Spawner NoC node number.
 */
static int nodenum = -1;

/* Forward definitions. */
extern int name_server(int, int);
extern void test_kernel_sys_sync(void);
extern void test_kernel_barrier(void);

/**
 * @brief Generic test driver.
 */
static void test_kernel(const char *module)
{
	if (!strcmp(module, "--hal-sync"))
		test_kernel_sys_sync();
}

/**
 * @brief Generic test driver.
 */
static void test_runtime(const char *module)
{
	if (!strcmp(module, "--barrier"))
		test_kernel_barrier();
}

/**
 * @brief Initializes spawner.
 */
void spawner_init(void)
{
	nodenum = sys_get_node_num();

	assert((inbox = sys_mailbox_create(nodenum)) >= 0);
}

/**
 * @brief Acknowledges spawner.
 */
void spawner_ack(void)
{
	int outbox;
	struct spawner_message msg;

	msg.status = 0;

	/* Send acknowledge message. */
	assert((outbox = sys_mailbox_open(SPAWNER1_SERVER_NODE)) >= 0);
	assert(sys_mailbox_write(outbox, &msg, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
	assert(sys_mailbox_close(outbox) == 0);
}

/**
 * @brief Sync spawners.
 */
void spawners_sync(void)
{
	int syncid;
	int syncid_local;
	int nodes[2];
	int nodes_local[2];
	struct spawner_message msg;

	/* Wait for acknowledge message of all servers. */
	for (int i = 0; i < NR_SERVERS; i++)
	{
		assert(sys_mailbox_read(inbox, &msg, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
		assert(msg.status == 0);
	}

	nodes[0] = nodenum;
	nodes[1] = SPAWNER_SERVER_NODE;

	nodes_local[0] = SPAWNER_SERVER_NODE;
	nodes_local[1] = nodenum;

	/* Open syncrhonization points. */
	assert((syncid_local = sys_sync_create(nodes_local, 2, SYNC_ONE_TO_ALL)) >= 0);
	assert((syncid = sys_sync_open(nodes, 2, SYNC_ONE_TO_ALL)) >= 0);

	assert(sys_sync_wait(syncid_local) == 0);
	assert(sys_sync_signal(syncid) == 0);

	/* House keeping. */
	assert(sys_mailbox_unlink(inbox) == 0);
	assert(sys_sync_unlink(syncid_local) == 0);
	assert(sys_sync_close(syncid) == 0);
}

SPAWNER_NAME("spawner1")
SPAWNER_SHUTDOWN(SHUTDOWN_DISABLE)
SPAWNER_SERVERS(NR_SERVERS, { name_server, NAME_SERVER_NODE, 0 } )
SPAWNER_MAIN2(NULL)
SPAWNER_KERNEL_TESTS(test_kernel)
SPAWNER_RUNTIME_TESTS(test_runtime)
