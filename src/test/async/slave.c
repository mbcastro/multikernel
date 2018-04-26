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

#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "kernel.h"

static char buffer[MAX_BUFFER_SIZE];

int main(int argc, const char **argv)
{
	long t[2];
	int size;
	int clusterid;
	off64_t offset;
	long total_time;

	assert(argc == 3);
	assert((size = atoi(argv[2])) <= MAX_BUFFER_SIZE);

	mppa_rpc_client_init();
	mppa_async_init();

	clusterid = k1_get_cluster_id();

	assert(mppa_async_malloc(MPPA_ASYNC_DDR_0,  NR_CCLUSTER*size, &offset, NULL) == 0);

	k1_timer_init();

	for (int i = 0; i < NITERATIONS; i++)
	{
		mppa_rpc_barrier_all();
		t[0] = k1_timer_get();

		assert(mppa_async_put(buffer,
				MPPA_ASYNC_DDR_0,
				offset + clusterid*size,
				size,
				NULL) == 0
		);
		assert(mppa_async_fence(MPPA_ASYNC_DDR_0, NULL) == 0);

		mppa_rpc_barrier_all();
		t[1] = k1_timer_get();

		if (i == 0)
			continue;

		total_time = k1_timer_diff(t[0], t[1]);
		printf("%s;%d;%d;%ld\n",
			"write",
			clusterid,
			size,
			total_time
		);
	}

	assert(mppa_async_free(MPPA_ASYNC_DDR_0, offset, NULL) == 0);

	mppa_async_final();

	return (EXIT_SUCCESS);
}
