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

#include <HAL/hal/core/mp.h>

#include <nanvix/hal.h>

#include "mppa.h" 

/**
 * @brief Threads table.
 */
pthread_t __threads[NR_IOCLUSTER_CORES] = { 0, };

/**
 * @brief Lock for critical region.
 */
pthread_mutex_t core_lock;

/*============================================================================*
 * k1_is_ccluster()                                                           *
 *============================================================================*/

/**
 * @brief Asserts whether or not the target cluster is a compute
 * cluster.
 *
 * @param clusterid ID of the target cluster.
 *
 * @return Non zero if the target cluster is a compute cluster and
 * zero otherwise.
 */
int k1_is_ccluster(int clusterid)
{
	return ((clusterid >= CCLUSTER0) && (clusterid <= CCLUSTER15));
}

/*============================================================================*
 * k1_is_iocluster()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not the target cluster is an IO cluster.
 *
 * @param clusterid ID of the target cluster.
 *
 * @return Non zero if the target cluster is an IO cluster and zero
 * otherwise.
 */
int k1_is_iocluster(int clusterid)
{
	return ((clusterid == IOCLUSTER0) || (clusterid == IOCLUSTER1));
} 

/*============================================================================*
 * hal_get_cluster_id()                                                       *
 *============================================================================*/

/**
 * @brief Gets the ID of the underlying cluster.
 *
 * @returns The ID of the underlying cluster
 */
int hal_get_cluster_id(void)
{
	return (__k1_get_cluster_id());
}

/*============================================================================*
 * hal_get_core_id()                                                          *
 *============================================================================*/

/**
 * @brief Gets the ID of the underlying core.
 *
 * @returns The ID of the underlying core.
 */
int hal_get_core_id(void)
{
	if (k1_is_iocluster(__k1_get_cluster_id()))
	{
		int coreid = 0;
		pthread_t tid;

		tid = pthread_self();


		pthread_mutex_lock(&core_lock);
		for (int i = 0; i < NR_IOCLUSTER_CORES; i++)
		{
			if (__threads[i] == tid)
			{
				coreid = i;
				break;
			}
		}
		pthread_mutex_unlock(&core_lock);

		return (coreid);
	}

	return (__k1_get_cpu_id());
}

/*============================================================================*
 * hal_get_core_type()                                                        *
 *============================================================================*/

/**
 * @brief Gets the type of the underlying core.
 *
 * @returns The type of the underlying core.
 */
int hal_get_core_type(void)
{
	int clusterid;

	clusterid = hal_get_cluster_id();

	return (k1_is_ccluster(clusterid) ? HAL_CORE_USER : HAL_CORE_SYSTEM);
}

/*============================================================================*
 * hal_get_num_cores()                                                        *
 *============================================================================*/

/**
 * @brief Gets the number of cores in the processor.
 *
 * @returns The number of cores in the processor.
 */
int hal_get_num_cores(void)
{
	int clusterid;

	clusterid = hal_get_cluster_id();

	return (k1_is_ccluster(clusterid) ? 17 : 4);
}

/*============================================================================*
 * hal_get_core_freq()                                                        *
 *============================================================================*/

/**
 * @brief Gets the frequency of the underlying core.
 *
 * @returns The frequency of the underlying core.
 */
int hal_get_core_freq(void)
{
	return (__bsp_frequency);
}
