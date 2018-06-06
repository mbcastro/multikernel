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
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/klib.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief ID of slave processes.
 */
static int pids[NR_CCLUSTER];

/*===================================================================*
 * Kernel                                                            *
 *===================================================================*/

/**
 * @brief Querying the name server.
 */
int main(int argc, char **argv)
{
	int global_barrier;
	int nclusters;
	char pathname[50];
	char process_name[50];

	assert(argc == 2);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);

	const char *args[] = {
		"name-slave",
		argv[1],
		NULL
	};

	/* Wait name server. */
	global_barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(global_barrier);

	/* IO cluster registration test */
	for(int i = 0; i < NR_IOCLUSTER_DMA; i++){
		snprintf(pathname, ARRAY_LENGTH(pathname), "/name%d", i);
		snprintf(process_name, ARRAY_LENGTH(process_name), "name-test%d", i);
	  register_name(k1_get_cluster_id() + i, pathname, process_name);

		assert(name_cluster_id(pathname) == k1_get_cluster_id());
	  assert(name_cluster_dma(pathname) == k1_get_cluster_id() + i);
	  assert(strcmp(id_cluster_name(k1_get_cluster_id() + i), pathname) == 0);
	  assert(strcmp(id_process_name(k1_get_cluster_id() + i), process_name) == 0);
	}

	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);

	for (int i = 0; i < nclusters; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);

	/* House keeping. */
	barrier_close(global_barrier);

	return (EXIT_SUCCESS);
}
