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

#include <nanvix/const.h>
#include <nanvix/syscalls.h>

struct server
{
	int (*main) (int);
	int nodenum;
};

extern int usermode;
extern int NR_SERVERS;
extern struct server servers[];

/* Forward definitions. */
extern int main2(int, const char **);
extern void test_kernel(const char *module);
extern void test_runtime(const char *module, int nservers);
extern void spawners_sync(void);

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

	kernel_setup();

	servernum = ((int *)args)[0];

	nodenum = servers[servernum].nodenum;
	main_fn = servers[servernum].main;

	/* Open server mailbox. */
	inbox = sys_mailbox_create(nodenum);

	/* Wait for other servers. */
	pthread_barrier_wait(&barrier);

	/* Spawn server. */
	main_fn(inbox);

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
	int args[NR_SERVERS];
	pthread_t tids[NR_SERVERS];

	/* Debug mode? */
	if (argc >= 2)
	{
		if (!strcmp(argv[1] , "--debug"))
			debug = 1;
	}

	/* Initialization. */
	assert(kernel_setup() == 0);

	printf("[nanvix][spawner] booting up server\n");

	/* Run self-tests. */
	if (debug)
		test_kernel(argv[2]);

	printf("[nanvix][spawner] server alive\n");

	pthread_barrier_init(&barrier, NULL, NR_SERVERS + 1);

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
		test_runtime(argv[2], 0);

	if (usermode)
	{
		printf("[nanvix][spawner] switching to user mode\n");

		/* Initialization. */
		assert(runtime_setup() == 0);

		ret = main2(argc, argv);	

		/* Cleanup. */
		assert(runtime_cleanup() == 0);

		printf("[nanvix][spawner] shutting down\n");
	}

	/* Wait for name server thread. */
	for (int i = 0; i < NR_SERVERS; i++)
		pthread_join(tids[i], NULL);

	/* Cleanup. */
	assert(kernel_cleanup() == 0);
	return (ret);
}
