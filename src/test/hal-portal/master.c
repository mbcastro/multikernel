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
#include <nanvix/hal.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#define DATA_SIZE 1024
#define TID_READ 0

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
	portal_t inportal;
	int nodeid;

	hal_setup();

	pthread_barrier_wait(&barrier);

	((void) args);

	nodeid = hal_get_node_id();

	pthread_mutex_lock(&lock);
	TEST_ASSERT((hal_portal_create(&inportal, nodeid) == 0));
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_portal_unlink(&inportal) == 0);
	pthread_mutex_unlock(&lock);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Portal Create Unlink
 */
static void test_hal_portal_create_unlink(void)
{
	int tids[ncores];
	pthread_t threads[ncores];

	printf("[test][api] Portal Create Unlink\n");

	/* Spawn driver threads. */
	for (int i = 0; i < ncores; i++)
	{
		tids[i] = i;
		assert((pthread_create(&threads[i],
			NULL,
			test_hal_portal_thread_create_unlink,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 0; i < ncores; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * API Test: Open Close                                              *
 *===================================================================*/

/**
 * @brief API Test: Portal Open Close
 */
static void *test_hal_portal_thread_open_close(void *args)
{
	portal_t outportal;
	int tid;
	int nodeid;

	hal_setup();

	pthread_barrier_wait(&barrier);

	tid = ((int *)args)[0];

	nodeid = hal_get_node_id();

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((hal_portal_open(&outportal, ((tid + 1) == ncores) ?
		nodeid + 1 - ncores + 1:
		nodeid + 1)) == 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_portal_close(&outportal) == 0);
	pthread_mutex_unlock(&lock);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Portal Open Close
 */
static void test_hal_portal_open_close(void)
{
	int tids[ncores];
	pthread_t threads[ncores];

	printf("[test][api] Portal Open Close\n");

	/* Spawn driver threads. */
	for (int i = 0; i < ncores; i++)
	{
		tids[i] = i;
		assert((pthread_create(&threads[i],
			NULL,
			test_hal_portal_thread_open_close,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 0; i < ncores; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * API Test: Read Write                                              *
 *===================================================================*/

/**
 * @brief API Test: Portal Read Write thread
 */
static void *test_hal_portal_thread_read_write(void *args)
{
	portal_t outportal;
	portal_t inportal;
	char buf[DATA_SIZE];
	int nodeid;
	int clusterid;

	hal_setup();

	pthread_barrier_wait(&barrier);

	((void) args);

	nodeid = hal_get_node_id();
	clusterid = hal_get_cluster_id();

	/* Reader thread */
	if (nodeid == clusterid + TID_READ)
	{
		pthread_mutex_lock(&lock);
		TEST_ASSERT((hal_portal_create(&inportal, nodeid)) == 0);
		pthread_mutex_unlock(&lock);

		for (int dma = 0; dma < ncores; dma++)
		{
			if (clusterid + dma == nodeid)
				continue;

			/* Enables read operations. */
			TEST_ASSERT(hal_portal_allow(&inportal, clusterid + dma) == 0);

			memset(buf, 0, DATA_SIZE);
			TEST_ASSERT(hal_portal_read(&inportal, buf, DATA_SIZE)
													== DATA_SIZE);

			for (int i = 0; i < DATA_SIZE; i++)
				TEST_ASSERT(buf[i] == 1);
		}

		pthread_mutex_lock(&lock);
		TEST_ASSERT(hal_portal_unlink(&inportal) == 0);
		pthread_mutex_unlock(&lock);
	}
	else
	{
		pthread_mutex_lock(&lock);
		TEST_ASSERT(hal_portal_open(&outportal, clusterid + TID_READ) == 0);
		pthread_mutex_unlock(&lock);

		memset(buf, 1, DATA_SIZE);
		TEST_ASSERT(hal_portal_write(&outportal, buf, DATA_SIZE)
												  == DATA_SIZE);

		pthread_mutex_lock(&lock);
		TEST_ASSERT(hal_portal_close(&outportal) == 0);
		pthread_mutex_unlock(&lock);
	}

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Portal Read Write
 */
static void test_hal_portal_read_write(void)
{
	int tids[ncores];
	pthread_t threads[ncores];

	printf("[test][api] Portal Read Write\n");

	/* Spawn driver threads. */
	for (int i = 0; i < ncores; i++)
	{
		tids[i] = i;

		assert((pthread_create(&threads[i],
			NULL,
			test_hal_portal_thread_read_write,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 0; i < ncores; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * HAL Portal Test Driver                                            *
 *===================================================================*/

/**
 * @brief HAL Portal Test Driver
 */
int main(int argc, const char **argv)
{
	((void) argc);
	((void) argv);

	hal_setup();

	ncores = hal_get_num_cores();

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, ncores);

	test_hal_portal_create_unlink();
	test_hal_portal_open_close();
	test_hal_portal_read_write();

	hal_cleanup();
	return (EXIT_SUCCESS);
}
