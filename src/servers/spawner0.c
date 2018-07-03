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

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_MAILBOX_
#include <nanvix/config.h>
#include <nanvix/hal.h>

/**
 * @briwf Number of servers.
 */
#define NR_SERVERS 1

/* Import definitions. */
extern int name_server(int);

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

	hal_setup();

	servernum = ((int *)args)[0];

	nodenum = servers[servernum].nodenum;
	main_fn = servers[servernum].main;

	/* Open server mailbox. */
	inbox = hal_mailbox_create(hal_noc_nodes[nodenum]);

	/* Wait for other servers. */
	pthread_barrier_wait(&barrier);

	/* Spawn server. */
	main_fn(inbox);

	hal_cleanup();
	return (NULL);
}

/* Low-level unit-tests. */
extern void test_hal(void);
extern void test_hal_sync(void);
extern void test_hal_mailbox(void);
extern void test_hal_portal(void);

/* High-level unit-tests. */
extern void test_name(int);
extern void test_mailbox(int);

/**
 * @brief Generic test driver.
 */
static void test0(const char *module)
{
	printf("[nanvix][spawner0] running low-level self-tests\n");

	if (!strcmp(module, "--hal"))
		test_hal();
	else if (!strcmp(module, "--hal-sync"))
		test_hal_sync();
	else if (!strcmp(module, "--hal-mailbox"))
		test_hal_mailbox();
	else if (!strcmp(module, "--hal-portal"))
		test_hal_portal();
}

/**
 * @brief Generic test driver.
 */
static void test1(const char *module)
{
	printf("[nanvix][spawner0] running high-level self-tests\n");

	if (!strcmp(module, "--name"))
		test_name(NR_SERVERS);
	else if (!strcmp(module, "--mailbox"))
		test_mailbox(NR_SERVERS);

	exit(EXIT_SUCCESS);
}

/**
 * @brief Resolves process names.
 */
int main(int argc, char **argv)
{
	int syncid;
	int debug = 0;
	int args[NR_SERVERS];
	int nodes[NR_SERVERS + 1];
	pthread_t tids[NR_SERVERS];

	/* Debug mode? */
	if (argc >= 2)
	{
		if (!strcmp(argv[1] , "--debug"))
			debug = 1;
	}

	hal_setup();

	printf("[nanvix][spawner0] booting up server\n");

	pthread_barrier_init(&barrier, NULL, NR_SERVERS + 1);

	/* Run self-tests. */
	if (debug)
		test0(argv[2]);

	printf("[nanvix][spawner0] server alive\n");

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

	/* Run self-tests. */
	if (debug)
		test1(argv[2]);

	/* Release master IO cluster. */
	nodes[0] = hal_get_node_id();
	nodes[1] = 192;

	assert((syncid = hal_sync_open(nodes, NR_SERVERS + 1, HAL_SYNC_ONE_TO_ALL)) >= 0);
	assert(hal_sync_signal(syncid) == 0);

	/* Wait for name server thread. */
	for (int i = 0; i < NR_SERVERS; i++)
		pthread_join(tids[i], NULL);

	hal_cleanup();
	return (EXIT_SUCCESS);
}
