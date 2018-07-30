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
#include <pthread.h>

#include <mppa/osconfig.h>

#include <nanvix/syscalls.h>
#include <nanvix/spawner.h>
#include <nanvix/pm.h>

/**
 * @brief Barrier for synchronization.
 */
pthread_barrier_t spawner_barrier;

/**
 * @brief Server wrapper.
 */
static void *server(void *args)
{
	int servernum;
	int runlevel;
	int (*main_fn) (int, int);

	kernel_setup();

	servernum = ((int *)args)[0];

	runlevel = spawner_servers[servernum].runlevel;
	main_fn = spawner_servers[servernum].main;

	/* Initialize runtime. */
	assert(runtime_setup(runlevel) == 0);

	/* Spawn server. */
	main_fn(get_inbox(), get_inportal());

	assert(runtime_cleanup() == 0);

	kernel_cleanup();
	return (NULL);
}

/**
 * @brief Spawns User Application
 */
int main(int argc, const char **argv)
{
	int ret = EXIT_SUCCESS;
	int debug = 0;
	int args[spawner_nservers];
	pthread_t tids[spawner_nservers];

	/* Debug mode? */
	if (argc >= 2)
	{
		if (!strcmp(argv[1] , "--debug"))
			debug = 1;
	}

	/* Initialization. */
	assert(kernel_setup() == 0);

	printf("[nanvix][%s] booting up server\n", spawner_name);

	/* Run self-tests. */
	if ((debug) && (test_kernel_fn != NULL))
		test_kernel_fn(argv[2]);

	printf("[nanvix][%s] server alive\n", spawner_name);

	pthread_barrier_init(&spawner_barrier, NULL, spawner_nservers + 1);

	/* Spawn servers. */
	for (int i = 0; i < spawner_nservers; i++)
	{
		args[i] = 0;
		assert((pthread_create(&tids[i],
			NULL,
			server,
			&args[i])) == 0
		);
	}

	pthread_barrier_wait(&spawner_barrier);

	spawners_sync();

	printf("[nanvix][%s] synced\n", spawner_name);

	/* Run self-tests. */
	if ((debug) && (test_runtime_fn != NULL))
		test_runtime_fn(argv[2]);

	if (main2_fn != NULL)
	{
		printf("[nanvix][%s] switching to user mode\n", spawner_name);

		/* Initialization. */
		assert(runtime_setup(2) == 0);

		ret = main2_fn(argc, argv);

		/* Cleanup. */
		assert(runtime_cleanup() == 0);
	}

	/* Wait for servers. */
	if (!spawner_shutdown)
	{
		for (int i = 0; i < spawner_nservers; i++)
			pthread_join(tids[i], NULL);
	}

	printf("[nanvix][%s] shutting down\n", spawner_name);

	/* Cleanup. */
	assert(kernel_cleanup() == 0);
	return (ret);
}
