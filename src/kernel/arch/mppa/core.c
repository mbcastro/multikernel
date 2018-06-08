/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of Nanvix.
 * 
 * Nanvix is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * Nanvix is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nanvix/arch/mppa.h>
#include <HAL/hal/core/mp.h>

/*====================================================================*
 * k1_get_cluster_id()                                                *
 *====================================================================*/

/**
 * @brief Gets the ID of the underlying cluster.
 *
 * @return The ID of the underlying cluster.
 */
inline int k1_get_cluster_id(void)
{
	return (__k1_get_cluster_id());
}

/*====================================================================*
 * k1_is_ccluster()                                                   *
 *====================================================================*/

/**
 * @brief Asserts whether or not the target cluster is a compute
 * cluster.
 *
 * @param clusterid ID of the target cluster.
 *
 * @return Non zero if the target cluster is a compute cluster and zero
 * otherwise.
 */
inline int k1_is_ccluster(int clusterid)
{
	return ((clusterid >= CCLUSTER0) && (clusterid <= CCLUSTER15));
}

/*====================================================================*
 * k1_is_iocluster()                                                  *
 *====================================================================*/

/**
 * @brief Asserts whether or not the target cluster is an IO cluster.
 *
 * @param clusterid ID of the target cluster.
 *
 * @return Non zero if the target cluster is an IO cluster and zero
 * otherwise.
 */
inline int k1_is_iocluster(int clusterid)
{
	return ((clusterid == IOCLUSTER0) || (clusterid == IOCLUSTER1));
}	

/*====================================================================*
 * k1_is_iocpu()                                                      *
 *====================================================================*/

/**
 * @brief Asserts wheter or not the target CPU ID is a CPU in an IO
 * cluster.
 *
 * @param cpuid ID of the target CPU.
 *
 * @return Non zero if the target CPU is a CPU in an IO cluster and
 * zero otherwise.
 */
inline int k1_is_iocpu(int cpuid)
{
	if ((cpuid >= IOCLUSTER0) && (cpuid < IOCLUSTER0 + NR_IOCLUSTER_DMA))
		return (1);
	else if ((cpuid >= IOCLUSTER1) && (cpuid < IOCLUSTER1 + NR_IOCLUSTER_DMA))
		return (1);
	return (0);
}

/*====================================================================*
 * k1_is_ccpu()                                                      *
 *====================================================================*/

/**
 * @brief Asserts wheter or not the target CPU ID is a CPU in a
 * Compute cluster.
 *
 * @param cpuid ID of the target CPU.
 *
 * @return Non zero if the target CPU is a CPU in a compute cluster
 * and zero otherwise.
 */
inline int k1_is_ccpu(int cpuid)
{
	return ((cpuid >= CCLUSTER0) && (cpuid <= CCLUSTER15));
}

/*====================================================================*
 * k1_get_ccluster_freq()                                             *
 *====================================================================*/

/**
 * @brief Gets the clock frequency of a compute cluster.
 *
 * @return The clock frequency of a compute cluster in Hertz.
 */
inline long k1_get_ccluster_freq(void)
{
	return (__bsp_frequency);
}
