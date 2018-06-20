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
#include <stdio.h>
#include <string.h>

#include <mppa/osconfig.h>

#include <nanvix/config.h>
#include <nanvix/hal.h>
#include <nanvix/limits.h>

/**
 * @brief ID of slave processes.
 */
static int pids[NANVIX_PROC_MAX];

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

/**
 * @brief Spawns remote processes.
 *
 * @param nremotes Number of remote processes to spawn.
 */
static void spawn_remotes(int nremotes)
{
	const char *argv[] = {
		"mailbox-slave",
		NULL
	};

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
	int int outboxes[nremotes].

	((void) nlocals);

	/* Open outboxes. */
	for (int i = 0; i < nremotes; i++)
		assert(outboxes[i] = mailbox_open(i));


	/* Close outboxes. */
	for (int i = 0; i < nremotes; i++)
		assert(mailbox_close(outboxes[i]));
}

/*============================================================================*
 * HAL Mailbox Microbenchmark Driver                                          *
 *============================================================================*/

/**
 * @brief HAL Mailbox Microbenchmark Driver
 */
int main(int argc, const char **argv)
{
	int nlocals;  /* Number of local peers.   */
	int nremotes; /* Number of remotes peers. */
	int pattern;  /* Transfer pattern.        */

	assert(argc == 4);

	/* Retrieve kernel parameters. */
	nlocals = atoi(argv[1]);
	nremotes = atoi(argv[2]);
	pattern = argv[3];

	/* Run kernel. */
	spawn_remotes(nremotes, argv);
	kernel(nlocals, nremotes, pattern);
	join_remotes(nremotes);

	return (EXIT_SUCCESS);
}
