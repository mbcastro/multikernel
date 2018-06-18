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

#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <stdlib.h>
#include "kernel.h"

#ifdef DEBUG
#include <stdio.h>
#endif

/**
 * @brief ID of slave processes.
 */
static int pids[NR_CCLUSTER];

/**
 * @brief Spawns slave processes.
 *
 * @param nclusters Number of clusters to spawn.
 * @param args      Cluster arguments.
 */
static void spawn_slaves(int nclusters, char **args)
{
	const char *argv[] = {
		"mailbox-slave",
		args[2],
		NULL
	};

	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);
}

/**
 * @brief Wait for slaves to complete.
 *
 * @param nclusters Number of slaves to wait.
 */
static void join_slaves(int nclusters)
{
	for (int i = 0; i < nclusters; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);
}

/*===================================================================*
 * Kernel                                                            *
 *===================================================================*/

/**
 * @brief Send several messages through mailboxes.
 *
 * @param inbox     Input mailbox.
 * @param nclusters Number of clusters.
 * @param nmessages Number of messages.
 */
static void kernel(int inbox, int nclusters, int nmessages)
{
	/* Receive messages. */
	for (int i = 0; i < nclusters; i++)
	{
		for (int j = 0; j < nmessages; j++)
		{
			struct message msg;
			mailbox_read(inbox, &msg);
			assert(msg.magic == MESSAGE_MAGIC);
#ifdef DEBUG
	printf("[mailbox] message received %d\n", i*nmessages + j + 1);
#endif
		}
	}

	/* Send messages. */
	for (int i = 0; i < nclusters; i++)
	{
		int outbox;

		outbox = hal_mailbox_open(i);

		for (int j = 0; j < nmessages; j++)
		{
			struct message msg;
			msg.magic = MESSAGE_MAGIC;
			mailbox_write(outbox, &msg);
#ifdef DEBUG
	printf("[mailbox] message sent %d\n", i*nmessages + j + 1);
#endif
		}

		mailbox_close(outbox);
	}
}

/**
 * @brief Benchmarks mailbox connector.
 */
int main(int argc, char **argv)
{
	int inbox;
	int nmessages;
	int nclusters;

	assert(argc == 3);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);
	assert(nmessages = atoi(argv[2]));

#ifdef DEBUG
	printf("[mailbox] spawning kernels\n");
#endif

	/* Open mailbox. */
	inbox = hal_mailbox_create(IOCLUSTER0);

	spawn_slaves(nclusters, argv);

#ifdef DEBUG
	printf("[mailbox] sending messages\n");
#endif

	kernel(inbox, nclusters, nmessages);

#ifdef DEBUG
	printf("[mailbox] waiting for kernels\n");
#endif

	/* House keeping. */
	join_slaves(nclusters);

	mailbox_unlink(inbox);

	return (EXIT_SUCCESS);
}
