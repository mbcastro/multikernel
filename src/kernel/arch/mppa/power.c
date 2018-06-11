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

#include "mppa.h" 

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

/*============================================================================*
 * hal_get_ucore_freq()                                                       *
 *============================================================================*/

/**
 * @brief Gets the frequency of a user core.
 *
 * @returns The frequency of a user core.
 */
int hal_get_ucore_freq(void)
{
}

/*============================================================================*
 * hal_get_rcore_freq()                                                       *
 *============================================================================*/

/**
 * @brief Gets the frequency of a resource management core.
 *
 * @returns The frequency of a resource management core.
 */
int hal_get_rcore_freq(void)
{
}

/*============================================================================*
 * hal_get_score_freq()                                                       *
 *============================================================================*/

/**
 * @brief Gets the frequency of a system core.
 *
 * @return Returns the frequency of a resource management core.
 */
int hal_get_score_freq(void)
{
}

