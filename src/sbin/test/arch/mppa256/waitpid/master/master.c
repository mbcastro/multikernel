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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include <mppa/osconfig.h>
#include <mppaipc.h>

#include <nanvix/const.h>
#include <nanvix/syscalls.h>
#include <nanvix/pm.h>

/**
 * @brief ID of slave processes.
 */
static int pids[50];

/**
 * @brief Barrier for synchronization.
 */
static pthread_barrier_t barrier;

static void spawn_slaves(int nclusters)
{
	int status;
	int barrier_slave;
	int nodes[2] = {0, SPAWNER_SERVER_NODE};

	const char *args[] = {
		"/test/waitpid-slave",
		NULL
	};

	assert((barrier_slave = barrier_create(nodes, 2)) >= 0);

	printf("Spawning slaves...\n");

	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);

	for (int i = 0; i < nclusters; i++)
	{
		// assert(barrier_wait(barrier_slave) == 0);
		assert(mppa_waitpid(pids[i], &status, 0) != -1);
		assert(status == EXIT_SUCCESS);
	}
}

/**
 * @brief Server.
 */
static void *server(void *args)
{
	((void) args);

	sys_setup();

	pthread_barrier_wait(&barrier);

	printf("Server alive\n");

	/* Wait for slaves spawn and waitpid call. */
	sleep(5);

	/** Termination never reached due to cooperative threads policy
	 * and mppa_waitpid() busy wait.
	 */
	exit(EXIT_SUCCESS);

	sys_cleanup();
	return (NULL);
}

/**
 * @brief waitpid test.
 */
int main(int argc, const char **argv)
{
	pthread_t tid;

	((void) argc);
	((void) argv);

	sys_setup();

	pthread_barrier_init(&barrier, NULL, 2);

	assert((pthread_create(&tid,
		NULL,
		server,
		NULL)) == 0
	);

	/* Wait for server. */
	pthread_barrier_wait(&barrier);

	spawn_slaves(1);

	pthread_join(tid, NULL);

	sys_cleanup();
	return (EXIT_SUCCESS);
}
