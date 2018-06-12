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

#include <nanvix/config.h>
#include <nanvix/hal.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }
/**
 * @brief Global barrier for synchronization.
 */
static pthread_barrier_t barrier;

/**
 * @brief Number of cores in the underlying cluster.
 */
static int ncores = 0;

/*===================================================================*
 * API Test: Query Cluster ID                                        *
 *===================================================================*/

/**
 * @brief API Test: Query Cluster ID
 */
static void *test_thread_hal_get_cluster_id(void *args)
{
	int arg;

	hal_setup();
	pthread_barrier_wait(&barrier);

	arg = ((int *)args)[0];

	TEST_ASSERT(arg == hal_get_cluster_id());

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Query Cluster ID.
 */
static void test_hal_get_cluster_id(void)
{
	int args[ncores];
	pthread_t threads[ncores];

	printf("[test][api] Query Cluster ID\n");

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		args[i] = hal_noc_nodes[SPAWNER_SERVER_NODE];
		assert((pthread_create(&threads[i],
			NULL,
			test_thread_hal_get_cluster_id,
			&args[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * API Test: Query Core ID                                           *
 *===================================================================*/

/**
 * @brief API Test: Query Core ID
 */
static void *test_thread_hal_get_core_id(void *args)
{
	int tid;

	hal_setup();
	pthread_barrier_wait(&barrier);

	tid = ((int *)args)[0];

	TEST_ASSERT(tid == hal_get_core_id());

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Query Core ID.
 */
static void test_hal_get_core_id(void)
{
	int args[ncores];
	pthread_t threads[ncores];

	printf("[test][api] Query Core ID\n");

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		args[i] = i;
		assert((pthread_create(&threads[i],
			NULL,
			test_thread_hal_get_core_id,
			&args[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
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

	/*
	 * We hope that this is not buggy.
	 */
	ncores = hal_get_num_cores();
	printf("[test][api] Number of Cores = %d\n", ncores);

	pthread_barrier_init(&barrier, NULL, ncores - 1);

	/* API tests. */
	test_hal_get_cluster_id();
	test_hal_get_core_id();

	hal_cleanup();
	return (EXIT_SUCCESS);
}
