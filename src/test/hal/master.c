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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <mppa/osconfig.h>

#include <nanvix/hal.h>

#define NR_CORES 4

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Global barrier for synchronization.
 */
static pthread_barrier_t barrier;

/**
 * @brief Lock for critical sections.
 */
static pthread_mutex_t lock;

/*===================================================================*
 * API Test: Query Core ID                                           *
 *===================================================================*/

/**
 * @brief API Test: Query Core ID
 */
static void *test_thread_hal_get_core_id(void *args)
{
	int tid;
	int coreid;

	hal_setup();

	tid = ((int *)args)[0];

	coreid = hal_get_core_id();

	pthread_mutex_lock(&lock);
	printf("coreid = %d %d\n", coreid, tid);
	assert (tid == coreid);
	pthread_mutex_unlock(&lock);

	return (NULL);
}

/**
 * @brief API Test: Query Core ID.
 */
static void test_hal_get_core_id(void)
{
	int tids[NR_CORES];
	pthread_t threads[NR_CORES];

	printf("[test][api] Query Core ID\n");

	/* Spawn driver threads. */
	for (int i = 1; i < NR_CORES; i++)
	{
		tids[i] = i;
		assert((pthread_create(&threads[i],
			NULL,
			test_thread_hal_get_core_id,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 0; i < NR_CORES; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * Mailbox Test Driver                                               *
 *===================================================================*/

/**
 * @brief Mailbox Test Driver
 */
int main(int argc, const char **argv)
{
	((void) argc);
	((void) argv);

	hal_setup();

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, NR_CORES);

	/* API tests. */
	test_hal_get_core_id();

	return (EXIT_SUCCESS);
}
