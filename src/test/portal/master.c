/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
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
	register_name(IOCLUSTER1, "/portal1", "portal-test");

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
