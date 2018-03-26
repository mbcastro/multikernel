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

#include <stdio.h>
#include <utask.h>
#include <stdlib.h>
#include <mppa_power.h>
#include <mppa_async.h>
#include <mppa_rpc.h>
#include <string.h>
#include <assert.h>
#include "kernel.h"

#define CHIP_FREQ (__bsp_frequency*MICRO)

static char buffer[MAX_BUFFER_SIZE];

int main(int argc, const char **argv)
{
	int size;
	int clusterid;
	uint64_t t[2];
	off64_t offset;
	double total_time;

	assert(argc == 2);
	assert((size = atoi(argv[1])) <= MAX_BUFFER_SIZE);

	mppa_rpc_client_init();
	mppa_async_init();

	clusterid = __k1_get_cluster_id();

	assert(mppa_async_malloc(MPPA_ASYNC_DDR_0, size, &offset, NULL) == 0);

	for (int i = 0; i < NITERATIONS; i++)
	{
		t[0] = __k1_read_dsu_timestamp();

		assert(mppa_async_put(buffer,
				MPPA_ASYNC_DDR_0,
				offset,
				size,
				NULL) == 0
		);

		t[1] = __k1_read_dsu_timestamp();

		if (i == 0)
			continue;

		total_time = (double) ((t[1] - t[0])/(double)CHIP_FREQ);
		printf("%2d;%2d;%d;%.2lf\n", i, clusterid, size, total_time);
	}

	assert(mppa_async_free(MPPA_ASYNC_DDR_0, offset, NULL) == 0);

	mppa_async_final();

	return (EXIT_SUCCESS);
}
