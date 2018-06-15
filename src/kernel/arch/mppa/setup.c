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

#include <pthread.h>

#include <nanvix/hal.h>

#include "mppa.h"

/**
 * @brief Number of running threads.
 */
static int nthreads = 0;

/**
 * @brief Lock for critical region.
 */
pthread_mutex_t hal_lock;

/**
 * @brief Threads table.
 */
pthread_t __threads[NR_IOCLUSTER_CORES] = { 0, };

/**
 * @brief Initializes platform-dependent structures.
 */
void hal_setup(void)
{
	pthread_t tid;

	/*
	 * Master thread initializes the lock. This is safe
	 * because it means that we are sequential.
	 */
	if (nthreads == 0)
		pthread_mutex_init(&hal_lock, NULL);

	tid = pthread_self();

	pthread_mutex_lock(&hal_lock);
		if (k1_is_iocluster(__k1_get_cluster_id()))
		{
			for (int i = 0; i < NR_IOCLUSTER_CORES; i++)
			{
				if (__threads[i] == 0)
				{
					__threads[i] = tid;
					nthreads++;
					break;
				}
			}
		}
	pthread_mutex_unlock(&hal_lock);
}

/**
 * @brief Cleans platform-dependent structures.
 */
void hal_cleanup(void)
{
	pthread_t tid;

	/*
	 * Master thread initializes the lock. This is safe
	 * because it means that we are sequential.
	 */
	if (nthreads == 1)
		pthread_mutex_destroy(&hal_lock);

	tid = pthread_self();

	pthread_mutex_lock(&hal_lock);
		if (k1_is_iocluster(__k1_get_cluster_id()))
		{
			for (int i = 0; i < NR_IOCLUSTER_CORES; i++)
			{
				if (__threads[i] == tid)
				{
					__threads[i] = 0;
					nthreads--;
					break;
				}
			}
		}
	pthread_mutex_unlock(&hal_lock);
}

