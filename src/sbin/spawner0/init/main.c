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

#define __NEED_HAL_BARRIER_
#include <nanvix/const.h>
#include <nanvix/spawner.h>
#include <nanvix/syscalls.h>

/* Forward definitions. */
extern int main2(int, const char **);
extern void test_kernel_sys_core(void);
extern void test_kernel_sys_sync(void);
extern void test_kernel_sys_mailbox(void);
extern void test_kernel_sys_portal(void);
extern int shm_server(int, int);
extern int mqueue_server(int, int);

/**
 * @brief Number of servers launched from this spawner.
 */
#ifdef _UNIX_
#define NR_SERVERS 0
#else
#define NR_SERVERS 2
#endif

/**
 * @brief Servers.
 */
static struct serverinfo servers[NR_SERVERS] = {
#ifndef _UNIX_
	{ shm_server,    SHM_SERVER_NODE,    1 },
	{ mqueue_server, MQUEUE_SERVER_NODE, 1 }
#endif
};

/**
 * @brief Input mailbox.
 */
static int inbox = -1;

/**
 * @brief Local sync.
 */
static int syncid_local = -1;

/**
 * @brief Local sync.
 */
static int syncid_remote = -1;

/**
 * @brief Sync's nodes.
 */
static int sync_nodes[2] = { -1, };

/**
 * @brief Spawner NoC node number.
 */
static int nodenum = -1;

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
 * @brief Initializes spawner.
 */
void spawner_init(void)
{
	/* Sanity check at compile time: Mailbox compliant */
	CHECK_MAILBOX_MSG_SIZE(struct spawner_message);

	nodenum = sys_get_node_num();

	assert((inbox = sys_mailbox_create(nodenum)) >= 0);

	sync_nodes[0] = SPAWNER1_SERVER_NODE;
	sync_nodes[1] = nodenum;

	/* Create synchronization point. */
	assert((syncid_local = sys_sync_create(sync_nodes, 2, SYNC_ONE_TO_ALL)) >= 0);
	assert((syncid_remote = sys_sync_open(sync_nodes, 2, SYNC_ALL_TO_ONE)) >= 0);
}

/**
 * @brief Finalizes local sync.
 */
void spawner_finalize(void)
{
	assert(sys_sync_close(syncid_remote) == 0);
	assert(sys_sync_unlink(syncid_local) == 0);
	assert(sys_mailbox_unlink(inbox) == 0);
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
	assert((outbox = sys_mailbox_open(SPAWNER_SERVER_NODE)) >= 0);
	assert(sys_mailbox_write(outbox, &msg, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
	assert(sys_mailbox_close(outbox) == 0);
}

/**
 * @brief Sync with server.
 */
void server_sync(void)
{
	struct spawner_message msg;

	assert(sys_mailbox_read(inbox, &msg, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
	assert(msg.status == 0);
}

/**
 * @brief Sync spawners.
 */
void spawners_sync(void)
{
	assert(sys_sync_wait(syncid_local) == 0);
	assert(sys_sync_signal(syncid_remote) == 0);
}

/**
 * @brief Shutdown servers.
 */
void servers_shutdown(void)
{
    int shutdown_order[] = {
        MQUEUE_SERVER_NODE,
        SHM_SERVER_NODE,
        RMEM_SERVER_NODE,
        SEMAPHORE_SERVER_NODE,
        NAME_SERVER_NODE
    };

	int outbox;
	struct spawner_message msg = { .header.opcode = SHUTDOWN_REQ };

    /* Shutdown request */
    for (int i = 0; i < 5; ++i)
    {
        assert((outbox = sys_mailbox_open(shutdown_order[i])) >= 0);
	    assert(sys_mailbox_write(outbox, &msg, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
	    assert(sys_mailbox_close(outbox) == 0);
    }
}

SPAWNER_NAME("spawner0")
SPAWNER_SERVERS(NR_SERVERS, servers)
#ifdef _UNIX_
SPAWNER_MAIN2(NULL)
SPAWNER_SHUTDOWN(NULL)
#else
SPAWNER_MAIN2(main2)
SPAWNER_SHUTDOWN(servers_shutdown)
#endif
SPAWNER_KERNEL_TESTS(test_kernel)

