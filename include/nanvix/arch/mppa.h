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

#ifndef NANVIX_ARCH_MPPA256
#define NANVIX_ARCH_MPPA256

	#ifndef _KALRAY_MPPA256
		#error "bad target"
	#endif

/*===========================================================================*
 * Core Interface                                                            *
 *===========================================================================*/

	/**
	 * @brief Number of user cores.
	 */
	#define HAL_NR_CORES_USER 256

	/**
	 * @brief Number of resource management cores.
	 */
	#define HAL_NR_CORES_RMAN 16

	/**
	 * @brief Number of system cores.
	 */
	#define HAL_NR_CORES_SYSTEM 16

	/**
	 * @brief Number of cluster in the paltform.
	 */
	#define HAL_NR_CLUSTERS 20

/*===========================================================================*
 * NoC Interface                                                             *
 *===========================================================================*/

	/**
	 * @brief Number of NoC nodes.
	 */
	#define HAL_NR_NOC_NODES 24

/*===========================================================================*
 * Mailbox Interface                                                         *
 *===========================================================================*/

	/**
	 * @brief Number of mailboxes.
	 */
	#ifdef _KALRAY_MPPA_CCLUSTER_
		#define HAL_NR_MAILBOX HAL_NR_NOC_NODES
	#else
		#define HAL_NR_MAILBOX (4*HAL*NR_NOC_NODES)
	#endif

	/**
	 * @brief Size (in bytes) of a mailbox message.
	 */
	#define HAL_MAILBOX_MSG_SIZE 64

#endif /* NANVIX_ARCH_MPPA256 */
