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
#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef _KALRAY_MPPA256
#include <mppa/osconfig.h>
#endif

#include <nanvix/spawner.h>
#include <nanvix/syscalls.h>
#include <nanvix/pm.h>

/* Forward definitions. */
extern int runtime_setup(int);
extern int runtime_cleanup(void);

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
	int initialized_servers = 0;
	int args[spawner_nservers];
	pthread_t tids[spawner_nservers];

	/* Debug mode? */
	if (argc >= 2)
	{
		if (!strcmp(argv[1] , "--debug"))
			debug = 1;
	}

	printf("[nanvix][%s] booting up", spawner_name);
	if (argc > 1)
	{
		printf(" [");
		for (int i = 1; i < argc; i++)
			printf("%s%s", argv[i], ((i + 1) < argc) ? " " : "]");
	}
	printf("\n");

	/* Initialization. */
	assert(kernel_setup() == 0);

	/* Run self-tests. */
	if ((debug) && (test_kernel_fn != NULL))
	{
		printf("[nanvix][%s] launching unit tests\n", spawner_name);
		test_kernel_fn(argv[2]);
	}

	spawner_init();

	printf("[nanvix][%s] alive\n", spawner_name);

	/* Spawn servers. */
	for (int curr_runlevel = 0; curr_runlevel < NR_RUNLEVELS; curr_runlevel++)
	{
		int new_services_amount = 0;

		printf("[nanvix][%s] initializing runtime %d\n", spawner_name, curr_runlevel);

		for (int i = 0; i < spawner_nservers; i++)
		{
			if (curr_runlevel != spawner_servers[i].runlevel)
				continue;

			args[i] = i;
			assert((pthread_create(&tids[i],
				NULL,
				server,
				&args[i])) == 0
			);

			server_sync();

			new_services_amount++;
		}

		/* Runlevel sync */
		spawners_sync();

		initialized_servers += new_services_amount;
	}

	spawner_finalize();

	printf("[nanvix][%s] %d/%d servers successfully launched\n", spawner_name, initialized_servers, spawner_nservers);
	
	if (initialized_servers != spawner_nservers)
		goto error;

	printf("[nanvix][%s] synced\n", spawner_name);

	if (main2_fn != NULL)
	{
		printf("[nanvix][%s] switching to user mode\n", spawner_name);

		/* Initialization. */
		assert(runtime_setup(3) == 0);

		ret = main2_fn(argc, argv);

		/* Cleanup. */
		assert(runtime_cleanup() == 0);
	}

	/* Shutdown servers. */
	if (spawner_shutdown != NULL)
	{
		printf("[nanvix][%s] broadcasting shutdown signal\n", spawner_name);
		spawner_shutdown();
	}

	/* Wait for servers. */
	for (int i = 0; i < spawner_nservers; i++)
		pthread_join(tids[i], NULL);

	/* Cleanup. */
	assert(kernel_cleanup() == 0);

	/* Sync spawners. */
	if (spawner_shutdown == NULL)
	{
		printf("[nanvix][%s] down\n", spawner_name);
#ifndef _UNIX_
	while(1);
#endif
	}

	return (ret);

error:
	kernel_cleanup();
	printf("[nanvix][kernel] failed to synchronize services initialization\n");
	return (-EAGAIN);
}
