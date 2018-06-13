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
#include <nanvix/hal.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

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
 * API Test: Create Unlink                                           *
 *===================================================================*/

/**
 * @brief API Test: Portal Create Unlink
 */
static void *test_hal_portal_thread_create_unlink(void *args)
{
	int dma;
	int inportal;
	int nodeid;

	dma = ((int *)args)[0];

	nodeid = hal_get_cluster_id();

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inportal = hal_portal_create(nodeid + dma)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_portal_unlink(inportal) == 0);
	pthread_mutex_unlock(&lock);

	return (NULL);
}

/**
 * @brief API Test: Portal Create Unlink
 */
static void test_hal_portal_create_unlink(void)
{
	int dmas[NR_CORES];
	pthread_t tids[NR_CORES];

	printf("[test][api] Portal Create Unlink\n");

	/* Spawn driver threads. */
	for (int i = 0; i < NR_CORES; i++)
	{
		dmas[i] = i;
		assert((pthread_create(&tids[i],
			NULL,
			test_hal_portal_thread_create_unlink,
			&dmas[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 0; i < NR_CORES; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * HAL Portal Test Driver                                               *
 *===================================================================*/

/**
 * @brief HAL Portal Test Driver
 */
int main()
{
	test_hal_portal_create_unlink();

	return (EXIT_SUCCESS);
}
