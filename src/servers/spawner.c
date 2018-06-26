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
#include <nanvix/hal.h>

/**
 * @briwf Number of servers.
 */
#define NR_SERVERS 1

/* Import definitions. */
extern void *name_server(void *);

/**
 * @brief Servers.
 */
static struct
{
	void *(*main) (void *);
} servers[NR_SERVERS] = {
	{ name_server },
};

/**
 * @brief Servers lock.
 */
pthread_mutex_t lock;

/**
 * @brief Barrier for synchronization.
 */
pthread_barrier_t barrier;

/**
 * @brief Resolves process names.
 */
int main(int argc, char **argv)
{
	int syncid;
	int nodes[NR_SERVERS + 1];
	pthread_t tids[NR_SERVERS];

	((void) argc);
	((void) argv);

	hal_setup();

	printf("[SPAWNER] booting up server\n");

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, NR_SERVERS + 1);

	/* Spawn servers. */
	for (int i = 0; i < NR_SERVERS; i++)
	{
		assert((pthread_create(&tids[i],
			NULL,
			servers[i].main,
			NULL)) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	/* Release master IO cluster. */
	nodes[0] = hal_get_node_id();
	nodes[1] = 192;

	assert((syncid = hal_sync_open(nodes, NR_SERVERS + 1, HAL_SYNC_ONE_TO_ALL)) >= 0);
	assert(hal_sync_signal(syncid) == 0);

	printf("[SPAWNER] server alive\n");

	/* Wait for name server thread. */
	for (int i = 0; i < NR_SERVERS; i++)
		pthread_join(tids[i], NULL);

	hal_cleanup();
	return (EXIT_SUCCESS);
}
