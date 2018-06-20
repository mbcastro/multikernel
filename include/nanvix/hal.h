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

#ifndef NANVIX_HAL_H_
#define NANVIX_HAL_H_

	#include <stddef.h>
	#include <inttypes.h>

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
 * Performance Monitoring                                                     *
 *============================================================================*/

	/* Forward definitions. */
	extern uint64_t hal_timer_get(void);
	extern uint64_t hal_timer_diff(uint64_t, uint64_t);
	extern void hal_timer_init(void);

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
 * Portal Interface                                                           *
 *============================================================================*/

	/* Forward definitions .*/
	extern int hal_portal_allow(portal_t *, int);
	extern int hal_portal_create(portal_t *, int);
	extern int hal_portal_open(portal_t *, int);
	extern int hal_portal_read(portal_t *, void *, size_t);
	extern int hal_portal_write(portal_t *, const void *, size_t);
	extern int hal_portal_close(portal_t *);
	extern int hal_portal_unlink(portal_t *);

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

	/* Forward definitions. */
	extern int hal_sync_create(const int *, int, int);
	extern int hal_sync_open(const int *, int, int);
	extern int hal_sync_wait(int);
	extern int hal_sync_signal(int);
	extern int hal_sync_close(int);
	extern int hal_sync_unlink(int);

#endif /* NANVIX_HAL_H_ */
