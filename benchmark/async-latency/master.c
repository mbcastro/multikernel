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
#include "kernel.h"

int main(int argc, const char **argv)
{
	((void) argc);
	((void) argv);

	int size;
	utask_t t;
	int status;
	int nclusters;

	assert(argc == 3);
	assert((nclusters = atoi(argv[1])) <= NR_CCLUSTER);
	assert((size = atoi(argv[2])) <= MAX_BUFFER_SIZE);

	mppa_rpc_server_init(1, 0, nclusters);
	mppa_async_server_init();

	const char *args[] = { "slave.elf", argv[1] , argv[2], NULL };
	for(int i = 0; i < nclusters; i++)
		assert(mppa_power_base_spawn(i, args[0], args, NULL, MPPA_POWER_SHUFFLING_ENABLED) != -1);

	utask_create(&t, NULL, (void*)mppa_rpc_server_start, NULL);

	for(int i = 0; i < nclusters; i++)
		assert(mppa_power_base_waitpid(i, &status, 0) >= 0);

	return 0;
}
