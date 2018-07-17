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
#include <pthread.h>
#include <stdlib.h>

#include <mppa/osconfig.h>

#include <nanvix/const.h>
#include <nanvix/syscalls.h>

/**
 * @brief Number of servers.
 */
#define NR_SERVERS 1

/* Forward definitions. */
extern int name_server(int);
extern void test_kernel_sys_sync(void);
extern void test_kernel_sys_mailbox(void);
extern void test_kernel_barrier(void);

/**
 * @brief Servers.
 */
static struct
{
	int (*main) (int);
	int nodenum;
} servers[NR_SERVERS] = {
	{ name_server, NAME_SERVER_NODE },
};

/**
 * @brief Barrier for synchronization.
 */
static pthread_barrier_t barrier;

/**
 * @brief Server wrapper.
 */
static void *server(void *args)
{
	int inbox;
	int nodenum;
	int servernum;
	int (*main_fn) (int);

	sys_setup();

	servernum = ((int *)args)[0];

	nodenum = servers[servernum].nodenum;
	main_fn = servers[servernum].main;

	/* Open server mailbox. */
	inbox = sys_mailbox_create(nodenum);

	/* Wait for other servers. */
	pthread_barrier_wait(&barrier);

	/* Spawn server. */
	main_fn(inbox);

	sys_cleanup();
	return (NULL);
}

/**
 * @brief Generic test driver.
 */
static void test_kernel(const char *module)
{
	if (!strcmp(module, "--hal-sync"))
		test_kernel_sys_sync();
	else if (!strcmp(module, "--hal-mailbox"))
		test_kernel_sys_mailbox();
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

/**
 * @brief Resolves process names.
 */
int main(int argc, char **argv)
{
	int debug = 0;
	int args[NR_SERVERS];
	pthread_t tids[NR_SERVERS];

	/* Debug mode? */
	if (argc >= 2)
	{
		if (!strcmp(argv[1] , "--debug"))
			debug = 1;
	}

	sys_setup();

	printf("[nanvix][spawner1] booting up server\n");

	pthread_barrier_init(&barrier, NULL, NR_SERVERS + 1);

	/* Run self-tests. */
	if (debug)
		test_kernel(argv[2]);

	printf("[nanvix][spawner1] server alive\n");

	/* Spawn servers. */
	for (int i = 0; i < NR_SERVERS; i++)
	{
		args[i] = 0;
		assert((pthread_create(&tids[i],
			NULL,
			server,
			&args[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	spawners_sync();

	/* Run self-tests. */
	if (debug)
		test_runtime(argv[2]);

	/* Wait for name server thread. */
	for (int i = 0; i < NR_SERVERS; i++)
		pthread_join(tids[i], NULL);

	sys_cleanup();
	return (EXIT_SUCCESS);
}
