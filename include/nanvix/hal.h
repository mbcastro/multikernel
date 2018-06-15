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

#ifndef NANVIX_HAL_H_
#define NANVIX_HAL_H_

	#include <stddef.h>

	#ifdef _KALRAY_MPPA256
		#include <nanvix/arch/mppa.h>
	#endif

/*============================================================================*
 * Misc                                                                       *
 *============================================================================*/

	/* Forward definitions. */
	extern void hal_setup(void);
	extern void hal_cleanup(void);

/*============================================================================*
 * Processor Management                                                       *
 *============================================================================*/

	/**
	 * @brief Type of cores.
	 */
	/**@{*/
	#define HAL_CORE_USER   0 /**< User core.                */
	#define HAL_CORE_RMAN   1 /**< Resource management core. */
	#define HAL_CORE_SYSTEM 2 /**< System core.              */
	/**@}*/

	/* Forward definitions. */
	extern int hal_get_cluster_id(void);
	extern int hal_get_core_id(void);
	extern int hal_get_core_type(void);
	extern int hal_get_num_cores(void);
	extern int hal_get_core_freq(void);

/*============================================================================*
 * NoC Interface                                                              *
 *============================================================================*/

	/* Forward definitions. */
	extern const int hal_noc_nodes[HAL_NR_NOC_NODES];

	/* Forward definitions. */
	extern int hal_get_node_id(void);

/*============================================================================*
 * Mailbox Interface                                                          *
 *============================================================================*/

	/* Forward definitions. */
	extern int hal_mailbox_create(int);
	extern int hal_mailbox_open(int);
	extern int hal_mailbox_unlink(int);
	extern int hal_mailbox_close(int);
	extern size_t hal_mailbox_write(int, const void *, size_t);
	extern size_t hal_mailbox_read(int, void *, size_t);

/*============================================================================*
 * Synchronization Point Interface                                            *
 *============================================================================*/

	/**
	 * @brief Types of synchronization points.
	 */
	/**@{*/
	#define HAL_SYNC_ONE_TO_ALL 0 /**< One to all. */
	#define HAL_SYNC_ALL_TO_ONE 1 /**< All to one. */
	/**@}*/

	/* Forward definitions .*/
	extern int hal_portal_allow(portal_t *, int);
	extern int hal_portal_create(portal_t *, int);
	extern int hal_portal_open(portal_t *, int);
	extern int hal_portal_read(portal_t *, void *, size_t);
	extern int hal_portal_write(portal_t *, const void *, size_t);
	extern int hal_portal_close(portal_t *);
	extern int hal_portal_unlink(portal_t *);

#endif /* NANVIX_HAL_H_ */
