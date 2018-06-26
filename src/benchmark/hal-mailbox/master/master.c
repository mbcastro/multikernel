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
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <mppa/osconfig.h>
#include <mppaipc.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_MAILBOX_
#include <nanvix/hal.h>
#include <nanvix/limits.h>

#include "../kernel.h"

/**
 * @brief Benchmark parameters.
 */
/**@{*/
static const char *pattern; /**< Transfer pattern             */
static int nlocals;         /**< Number of local peers.       */
static int ntotalremotes;   /**< Number of remotes peers.     */
static int nremotes;        /**< Number of remotes per local. */
static int ncols;           /**< Number of remotes in a row.  */
/**@}*/

/**
 * @brief Global barrier for synchronization.
 */
static pthread_barrier_t barrier;

/**
 * @brief Global lock for critical sections.
 */
static pthread_mutex_t lock;

/**
 * @brief ID of slave processes.
 */
static int pids[NANVIX_PROC_MAX];

/*============================================================================*
 * Utility                                                                    *
 *============================================================================*/

/**
 * @brief Spawns remote processes.
 *
 * @param tnum Number of the calling thread.
 */
static void spawn_remotes(int tnum)
{
	const int off = tnum*nremotes;
	int syncid;
	int nodeid;
	char master_node[4];
	char first_remote[4];
	char last_remote[4];
	int nodes[nremotes + 1];
	const char *argv[] = {
		"/benchmark/hal-mailbox-slave",
		master_node,
		first_remote,
		last_remote,
		NULL
	};

	nodeid = hal_get_node_id();

	/* Build nodes list. */
	nodes[0] = nodeid;
	for (int i = 0; i < nremotes; i++)
		nodes[i + 1] = off + i;

	/* Create synchronization point. */
	assert((syncid = hal_sync_create(nodes, nremotes + 1, HAL_SYNC_ALL_TO_ONE)) >= 0);

	/* Spawn remotes. */
	sprintf(master_node, "%d", nodeid);
	sprintf(first_remote, "%d", off);
	sprintf(last_remote, "%d", off + nremotes);
	for (int i = 0; i < nremotes; i++)
		assert((pids[off + i] = mppa_spawn(off + i, NULL, argv[0], argv, NULL)) != -1);

	/* Sync. */
	assert(hal_sync_wait(syncid) == 0);

	/* House keeping. */
	assert(hal_sync_unlink(syncid) == 0);
}

/**
 * @brief Wait for remote processes.
 *
 * @param tnum Number of the calling thread.
 */
static void join_remotes(int tnum)
{
	const int off = tnum*nremotes;

	for (int i = 0; i < nremotes; i++)
		assert(mppa_waitpid(pids[off + i], NULL, 0) != -1);
}

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

/**
 * @brief Benchmark kernel.
 */
static void *kernel(void *args)
{
	int tnum;
	int inbox;
	int nodeid;
	uint64_t t1, t2;
	int outboxes[nremotes];
	char buffer[HAL_MAILBOX_MSG_SIZE];

	/* Initialization. */
	hal_setup();
	nodeid = hal_get_node_id();
	assert((inbox = hal_mailbox_create(nodeid)) >= 0);

	tnum = ((int *) args)[0];

	memset(buffer, 1, HAL_MAILBOX_MSG_SIZE);

	spawn_remotes(tnum);

	/* Open output mailboxes. */
	if (!strcmp(pattern, "row"))
	{
		for (int i = 0, k = 0; i < ntotalremotes/ncols; i++)
		{
			for (int  j = 0; j < ncols/nlocals; j++)
			{
				int remoteid;
			   
				remoteid = i*ncols + tnum*(ncols/nlocals) + j;
				assert((outboxes[k++] = hal_mailbox_open(remoteid)) >= 0);
			}
		}
	}
	else
	{
		for (int  j = 0, k = 0; j < ncols/nlocals; j++)
		{
			for (int i = 0; i < ntotalremotes/ncols; i++)
			{
				int remoteid;
			   
				remoteid = i*ncols + tnum*(ncols/nlocals) + j;
				assert((outboxes[k++] = hal_mailbox_open(remoteid)) >= 0);
			}
		}
	}

	for (int k = 0; k < NITERATIONS; k++)
	{
		pthread_barrier_wait(&barrier);

		t1 = hal_timer_get();
		for (int i = 0; i < nremotes; i++)
		{
			assert(hal_mailbox_write(outboxes[i], buffer, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);
			assert(hal_mailbox_read(inbox, buffer, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);
		}
		t2 = hal_timer_get();

		pthread_mutex_lock(&lock);
		printf("time: %.2lf\n", ((double)hal_timer_diff(t1, t2))/nremotes);
		pthread_mutex_unlock(&lock);
	}
	
	/* Close output mailboxes. */
	for (int k = 0; k < nremotes; k++)
		assert((hal_mailbox_close(outboxes[k])) == 0);

	join_remotes(tnum);
	
	/* House keeping. */
	assert(hal_mailbox_unlink(inbox) == 0);
	hal_cleanup();

	return (NULL);
}

/**
 * @brief HAL Mailbox microbenchmark.
 */
static void benchmark(void)
{
	int args[nlocals];
	pthread_t tnums[nlocals];

	hal_timer_init();

	/* Spawn benchmark threads. */
	for (int i = 0; i < nlocals; i++)
	{
		args[i] = i;

		/* Master thread is already running. */
		if (i == 0)
			continue;

		assert((pthread_create(&tnums[i],
			NULL,
			kernel,
			&args[i])) == 0
		);
	}

	kernel(&args[0]);

	/* Wait for driver threads. */
	for (int i = 1; i < nlocals; i++)
		pthread_join(tnums[i], NULL);
}

/*============================================================================*
 * HAL Mailbox Microbenchmark Driver                                          *
 *============================================================================*/

/**
 * @brief HAL Mailbox Microbenchmark Driver
 */
int main(int argc, const char **argv)
{
	assert(argc == 5);

	/* Retrieve kernel parameters. */
	nlocals = atoi(argv[1]);
	ntotalremotes = atoi(argv[2]);
	pattern = argv[3];
	ncols = atoi(argv[4]);
	nremotes = ntotalremotes/nlocals;

	/* Parameter checking. */
	assert((ntotalremotes%ncols) == 0);
	assert((ntotalremotes%nlocals) == 0);
	assert((ncols%nlocals) == 0);
	if (!strcmp(pattern, "row"))
		assert((ntotalremotes%ncols) == 0);

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, nlocals);

	benchmark();

	pthread_mutex_destroy(&lock);
	pthread_barrier_destroy(&barrier);

	return (EXIT_SUCCESS);
}
