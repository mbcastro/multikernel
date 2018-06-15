/*
 * Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include <mppa/osconfig.h>

#include <nanvix/hal.h>
#include <nanvix/pm.h>

/**
 * @briwf Number of servers.
 */
#define NR_SERVERS 1

/* Import definitions. */
extern void *name_server(void *);

/**
 * @brief Servers.
 */
static struct
{
	void *(*main) (void *);
} servers[NR_SERVERS] = {
	{ name_server },
};

/**
 * @brie Servers lock.
 */
pthread_mutex_t lock;

/**
 * @brief Resolves process names.
 */
int main(int argc, char **argv)
{
	int global_barrier;
	pthread_t tids[NR_SERVERS];

	((void) argc);
	((void) argv);

	hal_setup();

	printf("[SPAWNER] booting up server\n");

	pthread_mutex_init(&lock, NULL);

	/* Spawn servers. */
	for (int i = 0; i < NR_SERVERS; i++)
	{
		assert((pthread_create(&tids[i],
			NULL,
			servers[i].main,
			NULL)) == 0
		);
	}

	/* Release master IO cluster. */
	global_barrier = barrier_open(0);
	barrier_wait(global_barrier);

	printf("[SPAWNER] server alive\n");

	/* Wait for name server thread. */
	for (int i = 0; i < NR_SERVERS; i++)
		pthread_join(tids[i], NULL);

	hal_cleanup();
	return (EXIT_SUCCESS);
}
