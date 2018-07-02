/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <pthread.h>

#include <HAL/hal/core/mp.h>

#define __NEED_HAL_CORE_
#include <nanvix/hal.h>

#include "core.h" 
#include "lock.h"

/**
 * @brief Threads table.
 */
pthread_t __threads[NR_IOCLUSTER_CORES] = { 0, };

/*============================================================================*
 * mppa256_is_ccluster()                                                      *
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
int mppa256_is_ccluster(int clusterid)
{
	return ((clusterid >= CCLUSTER0) && (clusterid <= CCLUSTER15));
}

/*============================================================================*
 * mppa256_is_iocluster()                                                     *
 *============================================================================*/

/**
 * @brief Asserts whether or not the target cluster is an IO cluster.
 *
 * @param clusterid ID of the target cluster.
 *
 * @return Non zero if the target cluster is an IO cluster and zero
 * otherwise.
 */
int mppa256_is_iocluster(int clusterid)
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
	if (mppa256_is_iocluster(__k1_get_cluster_id()))
	{
		int coreid = 0;
		pthread_t tid;

		tid = pthread_self();

		mppa256_lock();
		for (int i = 0; i < NR_IOCLUSTER_CORES; i++)
		{
			if (__threads[i] == tid)
			{
				coreid = i;
				break;
			}
		}
		mppa256_unlock();

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

	return (mppa256_is_ccluster(clusterid) ? HAL_CORE_USER : HAL_CORE_SYSTEM);
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

	return (mppa256_is_ccluster(clusterid) ? 17 : 4);
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
