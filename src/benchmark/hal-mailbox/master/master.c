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

#include <mppa/osconfig.h>
#include <mppaipc.h>

#include <nanvix/config.h>
#include <nanvix/hal.h>
#include <nanvix/limits.h>

#include "../kernel.h"

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
 * @param nremotes Number of remote processes to spawn.
 */
static void spawn_remotes(int nremotes)
{
	char tmp[4];
	const char *argv[] = {
		"/benchmark/hal-mailbox-slave",
		tmp,
		NULL
	};

	sprintf(tmp, "%d", nremotes);
	for (int i = 0; i < nremotes; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);
}

/**
 * @brief Wait for remote processes.
 *
 * @param nremotes Number of remote processes to wait.
 */
static void join_remotes(int nremotes)
{
	for (int i = 0; i < nremotes; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);
}

/**
 * @brief Syncs with remotes.
 *
 * @param nremotes Number of remote processes to sync.
 */
static void sync_remotes(int nremotes)
{
	int syncid;
	int nodes[nremotes + 1];

	/* Build nodes list. */
	nodes[0] = hal_get_node_id();
	for (int i = 0; i < nremotes; i++)
		nodes[i + 1] = i;

	/* Sync. */
	assert((syncid = hal_sync_create(nodes, nremotes + 1, HAL_SYNC_ALL_TO_ONE)) >= 0);
	assert(hal_sync_wait(syncid) == 0);
	assert(hal_sync_unlink(syncid) == 0);
}

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

/**
 * @brief HAL Mailbox Microbenchmark Kernel
 *
 * @param nlocals  Number of local peers.
 * @param nremotes Number of remote peers.
 * @param pattern  Transfer pattern.
 */
static void kernel(int nlocals, int nremotes, const char *pattern)
{
	int inbox;
	uint64_t t1, t2;
	int outboxes[nremotes];
	char buffer[HAL_MAILBOX_MSG_SIZE];

	((void) nlocals);

	/* Create inbox. */
	assert((inbox = hal_mailbox_create(hal_get_node_id())) >= 0);

	/* Open outboxes. */
	for (int i = 0; i < nremotes; i++)
		assert(outboxes[i] = hal_mailbox_open(i));

	memset(buffer, 1, HAL_MAILBOX_MSG_SIZE);

	sync_remotes(nremotes);

	hal_timer_init();

	if (!strcmp(pattern, "row"))
	{
		for (int j = 0; j < NITERATIONS; j++)
		{			
			t1 = hal_timer_get();
			for (int i = 0; i < nremotes; i++)
			{
				assert(hal_mailbox_write(outboxes[i], buffer, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);
				assert(hal_mailbox_read(inbox, buffer, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);
			}

			t2 = hal_timer_get();

			printf("time: %" PRIu64 "\n", hal_timer_diff(t1, t2));
		}
	}
	else
	{
		for (int i = 0; i < nremotes; i++)
		{
			assert(hal_mailbox_write(outboxes[i], buffer, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);
			assert(hal_mailbox_read(inbox, buffer, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);
		}
	}

	/* Close outboxes. */
	for (int i = 0; i < nremotes; i++)
		assert(hal_mailbox_close(outboxes[i]) == 0);

	/* House keeping. */
	assert(hal_mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * HAL Mailbox Microbenchmark Driver                                          *
 *============================================================================*/

/**
 * @brief HAL Mailbox Microbenchmark Driver
 */
int main(int argc, const char **argv)
{
	int nlocals;         /* Number of local peers.   */
	int nremotes;        /* Number of remotes peers. */
	const char *pattern; /* Transfer pattern.        */
	
	hal_setup();

	assert(argc == 4);

	/* Retrieve kernel parameters. */
	nlocals = atoi(argv[1]);
	nremotes = atoi(argv[2]);
	pattern = argv[3];

	/* Run kernel. */
	spawn_remotes(nremotes);
	kernel(nlocals, nremotes, pattern);
	join_remotes(nremotes);

	
	hal_cleanup();
	return (EXIT_SUCCESS);
}
