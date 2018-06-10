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

/*============================================================================*
 * hal_get_cluster_id()                                                       *
 *============================================================================*/

/**
 * @brief Gets the ID of the underlying cluster.
 *
 * @returns The ID of the underlying cluster
 */
inline int hal_get_cluster_id(void)
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
}

/*============================================================================*
 * hal_is_ucore()                                                             *
 *============================================================================*/

/**
 * @brief Asserts whether or not the target core is a user core.
 *
 * @param coreid ID of the target core.
 *
 * @returns One if the target core is a user core, and zero otherwise.
 */
int hal_is_ucore(int coreid)
{
}

/*============================================================================*
 * hal_is_rcore()                                                             *
 *============================================================================*/

/**
 * @brief Asserts whether or not the target core is a resource
 * management core.
 *
 * @param coreid ID of the target core.
 *
 * @returns One if the target core is a resource management core, and
 * zero otherwise.
 */
int hal_is_rcore(int coreid)
{
}

/*============================================================================*
 * hal_is_score()                                                             *
 *============================================================================*/

/**
 * @brief Asserts whether or not the taget core is a system core.
 *
 * @param coreid ID of the target core.
 *
 * @returns One if the target core is a system core, and zero
 * otherwise.
 */
int hal_is_score(int coreid)
{
}

/*============================================================================*
 * hal_get_num_clusters()                                                     *
 *============================================================================*/

/**
 * @brief Gets the number of clusters in the processor.
 *
 * @returns The number of clusters in the processor.
 */
int hal_get_num_clusters(void)
{
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
}

/*============================================================================*
 * hal_get_num_ucores()                                                       *
 *============================================================================*/

/**
 * @brief Gets the number of user cores in the processor.
 *
 * @returns The number of user cores in the processor.
 */
int hal_get_num_ucores(void)
{
}

/*============================================================================*
 * hal_get_num_rcores()                                                       *
 *============================================================================*/

/**
 * @brief Gets the number of resource management cores in the
 * processor.
 *
 * @returns The number of resource management cores in the processor.
 */
int hal_get_num_rcores(void)
{
}

/*============================================================================*
 * hal_get_num_scores()                                                       *
 *============================================================================*/

/**
 * @brief Gets the number of system cores in the processor.
 *
 * @returns The number of system cores in the processor.
 */
int hal_get_num_scores(void)
{
}

