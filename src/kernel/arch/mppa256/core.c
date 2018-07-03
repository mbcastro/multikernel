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

/**
 * @brief Threads table.
 */
static pthread_t threads[NR_IOCLUSTER_CORES] = { 0, };

/**
 * @brief Core module lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*============================================================================*
 * mppa256_core_lock()                                                        *
 *============================================================================*/

/**
 * @brief Locks MPPA-256 core module.
 */
static void mppa256_core_lock(void)
{
	pthread_mutex_lock(&lock);
}

/*============================================================================*
 * mppa256_core_unlock()                                                      *
 *============================================================================*/

/**
 * @brief Unlocks MPPA-256 core module.
 */
static void mppa256_core_unlock(void)
{
	pthread_mutex_unlock(&lock);
}

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
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
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
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
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
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int mppa256_get_cluster_id(void)
{
	return (__k1_get_cluster_id());
}

/*============================================================================*
 * hal_core_setup()                                                           *
 *============================================================================*/

/**
 * @brief Initializes core module.
 */
void mppa256_core_setup(void)
{
	pthread_t tid;

	tid = pthread_self();

	if (mppa256_is_iocluster(mppa256_get_cluster_id()))
	{
		mppa256_core_lock();

			for (int i = 0; i < NR_IOCLUSTER_CORES; i++)
			{
				if (threads[i] == 0)
				{
					threads[i] = tid;
					break;
				}
			}

		mppa256_core_unlock();
	}
}

/*============================================================================*
 * hal_core_cleanup()                                                         *
 *============================================================================*/

/**
 * @brief Cleans up core module mess.
 */
void mppa256_core_cleanup(void)
{
	pthread_t tid;

	tid = pthread_self();

	mppa256_core_lock();
		
		if (mppa256_is_iocluster(mppa256_get_cluster_id()))
		{
			for (int i = 0; i < NR_IOCLUSTER_CORES; i++)
			{
				if (threads[i] == tid)
				{
					threads[i] = 0;
					break;
				}
			}
		}

	mppa256_core_unlock();
}

/*============================================================================*
 * hal_get_cluster_id()                                                       *
 *============================================================================*/

/**
 * @brief Gets the ID of the underlying cluster.
 *
 * @returns The ID of the underlying cluster
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int hal_get_cluster_id(void)
{
	return (mppa256_get_cluster_id());
}

/*============================================================================*
 * hal_get_core_id()                                                          *
 *============================================================================*/

/**
 * @brief Gets the ID of the underlying core.
 *
 * @returns The ID of the underlying core.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 */
int hal_get_core_id(void)
{
	if (mppa256_is_iocluster(mppa256_get_cluster_id()))
	{
		int coreid = 0;
		pthread_t tid;

		tid = pthread_self();

		mppa256_core_lock();

		for (int i = 0; i < NR_IOCLUSTER_CORES; i++)
		{
			if (threads[i] == tid)
			{
				coreid = i;
				break;
			}
		}

		mppa256_core_unlock();

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
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int hal_get_core_type(void)
{
	int clusterid;

	clusterid = mppa256_get_cluster_id();

	return (mppa256_is_ccluster(clusterid) ? HAL_CORE_USER : HAL_CORE_SYSTEM);
}

/*============================================================================*
 * hal_get_num_cores()                                                        *
 *============================================================================*/

/**
 * @brief Gets the number of cores in the processor.
 *
 * @returns The number of cores in the processor.
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int hal_get_num_cores(void)
{
	int clusterid;

	clusterid = mppa256_get_cluster_id();

	return (mppa256_is_ccluster(clusterid) ? 17 : 4);
}

/*============================================================================*
 * hal_get_core_freq()                                                        *
 *============================================================================*/

/**
 * @brief Gets the frequency of the underlying core.
 *
 * @returns The frequency of the underlying core.
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int hal_get_core_freq(void)
{
	return (__bsp_frequency);
}
