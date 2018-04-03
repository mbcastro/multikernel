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
#include <assert.h>

/*====================================================================*
 * k1_is_iocluster()                                                  *
 *====================================================================*/

/**
 * @brief Asserts whether or not the target cluster is an IO Cluster.
 *
 * @param clusterid ID of the target cluster.
 *
 * @return Non zero if the target cluster is and IO Cluster and zero
 * otherwise
 */
inline int k1_is_iocluster(int clusterid)
{
	return ((clusterid == IOCLUSTER0) || (clusterid == IOCLUSTER1));
}	
