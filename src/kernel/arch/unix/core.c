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

#define __NEED_HAL_NOC_
#define __NEED_HAL_CORE_
#define __NEED_HAL_MUTEX_
#include <hal.h>
#include <klib.h>

#include "noc.h"

/**
 * @brief Maximum number of cores that are available in a cluster.
 */
#define NR_CORES                                        \
	((HAL_NR_IOCLUSTER_CORES > HAL_NR_CCLUSTER_CORES) ? \
	 	HAL_NR_IOCLUSTER_CORES : HAL_NR_CCLUSTER_CORES)

/**
 * @brief Threads table.
 */
static pthread_t threads[NR_CORES] = { 0, };

/**
 * @brief Core module lock.
 */
static hal_mutex_t lock = HAL_MUTEX_INITIALIZER;

/*============================================================================*
 * unix_core_lock()                                                           *
 *============================================================================*/

/**
 * @brief Locks core module.
 */
static inline void unix_core_lock(void)
{
	hal_mutex_lock(&lock);
}

/*============================================================================*
 * unix_core_unlock()                                                         *
 *============================================================================*/

/**
 * @brief Unlocks core module.
 */
static inline void unix_core_unlock(void)
{
	hal_mutex_unlock(&lock);
}

/*============================================================================*
 * unix_get_cluster_id()                                                       *
 *============================================================================*/

/**
 * @brief Gets the ID of the underlying cluster.
 *
 * @returns The ID of the underlying cluster
 *
 * @note This function is non-blocking.
 */
static inline int unix_get_cluster_id(void)
{
	static int clusterid = -1;

	unix_core_lock();

		/* Query ID of cluster once. */
		if (clusterid < 0)
		{
			int nodeid = hal_get_node_id();

			clusterid = (nodeid < IOCLUSTER1) ?
				IOCLUSTER0 : ((nodeid < CCLUSTER0) ? IOCLUSTER1 : nodeid);
		}

	unix_core_unlock();

	return (clusterid);
}

/*============================================================================*
 * unix_is_ccluster()                                                         *
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
static inline int unix_is_ccluster(int clusterid)
{
	return ((clusterid >= CCLUSTER0) && (clusterid <= CCLUSTER15));
}

/*============================================================================*
 * unix_is_iocluster()                                                        *
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
static inline int unix_is_iocluster(int clusterid)
{
	return ((clusterid == IOCLUSTER0) || (clusterid == IOCLUSTER1));
} 

/*============================================================================*
 * hal_core_setup()                                                           *
 *============================================================================*/

/**
 * @brief Initializes core module.
 */
void unix_core_setup(void)
{
	pthread_t tid;

	tid = pthread_self();

	unix_core_lock();

		/* Check if core is already initialized. */
		for (int i = 0; i < NR_CORES; i++)
		{
			/* Yes, it is. */
			if (threads[i] == tid)
				goto out;
		}

		/* Find an idle core. */
		for (int i = 0; i < NR_CORES; i++)
		{
			/* Found. */
			if (threads[i] == 0)
			{
				threads[i] = tid;
				goto out;
			}
		}

	kpanic("cannot allocate a core");

out:

	unix_core_unlock();
}

/*============================================================================*
 * hal_core_cleanup()                                                         *
 *============================================================================*/

/**
 * @brief Cleans up core module mess.
 */
void unix_core_cleanup(void)
{
	pthread_t tid;

	tid = pthread_self();
		
	unix_core_lock();
	
		/* Search for core. */
		for (int i = 0; i < NR_CORES; i++)
		{
			/* found. */
			if (threads[i] == tid)
			{
				threads[i] = 0;
				goto found;
			}
		}

		kpanic("cannot free an unattached core");

found:

	unix_core_unlock();
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
	return (unix_get_cluster_id());
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
	int coreid = 0;
	pthread_t tid;

	tid = pthread_self();

	unix_core_lock();

		/* Search for target core. */
		for (int i = 0; i < NR_CORES; i++)
		{
			/* Found. */
			if (threads[i] == tid)
			{
				coreid = i;
				goto found;
			}
		}

		kpanic("unattached core");

found:

	unix_core_unlock();

	return (coreid);
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

	clusterid = unix_get_cluster_id();

	return (unix_is_ccluster(clusterid) ? HAL_CORE_USER : HAL_CORE_SYSTEM);
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

	clusterid = unix_get_cluster_id();

	return (unix_is_ccluster(clusterid) ? HAL_NR_CCLUSTER_CORES : HAL_NR_IOCLUSTER_CORES);
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
	return (3000000000);
}

