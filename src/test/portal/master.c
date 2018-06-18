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
#include <stdio.h>
#include <string.h>
#include "kernel.h"

/*===================================================================*
 * Process Management                                                *
 *===================================================================*/

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
		"portal-slave",
		args[1],
		args[2],
		args[3],
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
 * @brief Buffer.
 */
static char buffer[NR_CCLUSTER*MAX_BUFFER_SIZE];

/**
 * @brief Benchmarks write operations on a portal connector.
 */
int main(int argc, char **argv)
{
	int size;      /* Write size.            */
	int nclusters; /* Number of cclusters.   */
	int inportal;  /* Input portal.          */

	assert(argc == 4);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[2]);
	assert((size = atoi(argv[3])) <= MAX_BUFFER_SIZE);

	/* Register process name*/
	name_link(IOCLUSTER1, "/portal1");

	/*
	 * Open input portal before sapwning
	 * slaves so that we are synced.
	 */
	inportal = portal_create("/portal1");

	spawn_slaves(nclusters, argv);

	/*
	 * Touch data to initialize all pages
	 * and warmup D-cache.
	 */
	memset(buffer, 0, nclusters*size);

	/*
	 * Benchmark. First iteration is
	 * used to warmup resources.
	 */
	for (int i = 0; i <= NITERATIONS; i++)
	{
		/* Read. */
		for (int j = 0; j < nclusters; j++)
		{
			portal_allow(inportal, j);
			portal_read(inportal, buffer, size);
		}
	}

	/* House keeping. */
	portal_close(inportal);
	join_slaves(nclusters);

	return (EXIT_SUCCESS);
}
