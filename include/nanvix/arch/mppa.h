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

	/**
	 * @brief Number of NoC nodes.
	 */
	#define HAL_NR_NOC_NODES 24

/*===========================================================================*
 * Mailbox Interface                                                         *
 *===========================================================================*/

	/**
	 * @brief Number DMAs per compute cluster.
	 */
	#define NR_CCLUSTER_DMA 1

	/**
	 * @brief Number of DMAs per compute cluster.
	 */
	#define NR_IOCLUSTER_DMA 4

	/**
	 * @brief Number of compute clusters.
	 */
	#define NR_CCLUSTER 16

	/**
	 * @brief Number of IO clusters.
	 */
	#define NR_IOCLUSTER 2

	/* Cluster IDs. */
	#define CCLUSTER0    0 /**< Compute cluster  0. */
	#define CCLUSTER1    1 /**< Compute cluster  1. */
	#define CCLUSTER2    2 /**< Compute cluster  2. */
	#define CCLUSTER3    3 /**< Compute cluster  3. */
	#define CCLUSTER4    4 /**< Compute cluster  4. */
	#define CCLUSTER5    5 /**< Compute cluster  5. */
	#define CCLUSTER6    6 /**< Compute cluster  6. */
	#define CCLUSTER7    7 /**< Compute cluster  7. */
	#define CCLUSTER8    8 /**< Compute cluster  8. */
	#define CCLUSTER9    9 /**< Compute cluster  9. */
	#define CCLUSTER10  10 /**< Compute cluster 10. */
	#define CCLUSTER11  11 /**< Compute cluster 11. */
	#define CCLUSTER12  12 /**< Compute cluster 12. */
	#define CCLUSTER13  13 /**< Compute cluster 13. */
	#define CCLUSTER14  14 /**< Compute cluster 14. */
	#define CCLUSTER15  15 /**< Compute cluster 15. */
	#define IOCLUSTER0 128 /**< IO cluster 0.       */
	#define IOCLUSTER1 192 /**< IO cluster 1.       */

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
