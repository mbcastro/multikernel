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
#include <time.h>

#include <nanvix/const.h>
#include <nanvix/syscalls.h>
#include <nanvix/spawner.h>

/* Forward definitions. */
extern int name_server(int, int);
extern int rmem_server(int, int);
extern int semaphore_server(int, int);

/**
 * @brief Number of servers launched from this spawner.
 */
#define NR_SERVERS 3

/**
 * @brief Servers.
 */
static struct serverinfo servers[NR_SERVERS] = {
	{ name_server,      NAME_SERVER_NODE,      0 },
	{ rmem_server,      RMEM_SERVER_NODE,      1 },
	{ semaphore_server, SEMAPHORE_SERVER_NODE, 1 }
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
 * @brief Remote sync.
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
 * @brief Initializes spawner.
 */
void spawner_init(void)
{
	time_t startup_time;

	startup_time = time(0);
	
	nodenum = sys_get_node_num();

	assert((inbox = sys_mailbox_create(nodenum)) >= 0);

	sync_nodes[0] = nodenum;
	sync_nodes[1] = SPAWNER_SERVER_NODE;

	/* Create synchronization point. */
	assert((syncid_local = sys_sync_create(sync_nodes, 2, SYNC_ALL_TO_ONE)) >= 0);
	assert((syncid_remote = sys_sync_open(sync_nodes, 2, SYNC_ONE_TO_ALL)) >= 0);

	/*
	 * Ensure some minimum startup delay, so 
	 * that we are sure that spawner zero is up.
	 */
	while (time(0) < (startup_time + STARTUP_DELAY))
		/* noop()*/;

}

/**
 * @brief Finalizes spawner.
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
	assert((outbox = sys_mailbox_open(SPAWNER1_SERVER_NODE)) >= 0);
	assert(sys_mailbox_write(outbox, &msg, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
	assert(sys_mailbox_close(outbox) == 0);
}

/**
 * @brief Sync spawners.
 */
void spawners_sync(int requested_acks)
{
	struct spawner_message msg;

	/* Wait for acknowledge message of all servers. */
	for (int i = 0; i < requested_acks; i++)
	{
		assert(sys_mailbox_read(inbox, &msg, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
		assert(msg.status == 0);
	}

	/* Synchronization point. */
	assert(sys_sync_signal(syncid_remote) == 0);
	assert(sys_sync_wait(syncid_local) == 0);
}

SPAWNER_NAME("spawner1")
SPAWNER_SHUTDOWN(SHUTDOWN_DISABLE)
SPAWNER_SERVERS(NR_SERVERS, servers)
SPAWNER_MAIN2(NULL)
SPAWNER_KERNEL_TESTS(NULL)

