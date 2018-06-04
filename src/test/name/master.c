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
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define NR_SLAVE 16

/**
 * @brief ID of slave processes.
 */
static int pids[NR_SLAVE];

/*===================================================================*
 * Kernel                                                            *
 *===================================================================*/

/**
 * @brief Querying the name server.
 */
int main()
{
	int global_barrier;
	char pathname[15];
	char process_name[50];

	const char *argv[] = {
		"name-slave",
		NULL
	};

	/* Wait name server. */
	global_barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(global_barrier);

	/* IO cluster registration test */
	for(int i = 0; i < NR_IOCLUSTER_DMA; i++){
		sprintf(pathname, "/name%d", i);
		sprintf(process_name, "name-test%d", i);
	  register_name(k1_get_cluster_id() + i, pathname, process_name);

		printf("name_cluster_id(%s) call from IO cluster, id: %d.\n", pathname, name_cluster_id(pathname));
	  printf("name_cluster_dma(%s) call from IO cluster, dma: %d.\n", pathname, name_cluster_dma(pathname));
	  printf("id_cluster_name(%d) call from IO cluster, name: %s.\n", k1_get_cluster_id() + i, id_cluster_name(k1_get_cluster_id() + i));
	  printf("id_process_name(%d) call from IO cluster, name: %s.\n", k1_get_cluster_id() + i, id_process_name(k1_get_cluster_id() + i));
	}

	for (int i = 0; i < NR_SLAVE; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);

	for (int i = 0; i < NR_SLAVE; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);

	return (EXIT_SUCCESS);
}
