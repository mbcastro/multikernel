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
 * Machine Setup                                                              *
 *============================================================================*/

#ifdef __NEED_HAL_SETUP_

	/* Forward definitions. */
	extern void hal_setup(void);
	extern void hal_cleanup(void);

#endif /* __NEED_HAL_SETUP */

/*============================================================================*
 * Performance Monitoring Interface                                           *
 *============================================================================*/

	/* Forward definitions. */
	extern uint64_t hal_timer_get(void);
	extern uint64_t hal_timer_diff(uint64_t, uint64_t);
	extern void hal_timer_init(void);

/*============================================================================*
 * Core Interface                                                             *
 *============================================================================*/

#ifdef __NEED_HAL_CORE_

	/* Sanity check. */
	#ifndef HAL_NR_CORES_USER
		#error "undefined symbol: HAL_NR_CORES_USER"
	#endif

	/* Sanity check. */
	#ifndef HAL_NR_CORES_RMAN
		#error "undefined symbol: HAL_NR_CORES_RMAN"
	#endif
	
	/* Sanity check. */
	#ifndef HAL_NR_CORES_SYSTEM
		#error "undefined symbol: HAL_NR_CORES_SYSTEM"
	#endif

	/* Sanity check. */
	#ifndef HAL_NR_IOCLUSTERS
		#error "undefined symbol: HAL_NR_IOCLUSTERS"
	#endif

	/* Sanity check. */
	#ifndef HAL_NR_CCLUSTERS
		#error "undefined symbol: HAL_NR_CCLUSTERS"
	#endif

	/* Sanity check. */
	#ifndef HAL_NR_CLUSTERS
		#error "undefined symbol: HAL_NR_CLUSTERS"
	#endif

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

#endif /* __NEED_HAL_CORE_ */

/*============================================================================*
 * NoC Interface                                                              *
 *============================================================================*/

#ifdef __NEED_HAL_NOC_

	/* Sanity check. */
	#ifndef HAL_NR_NOC_IONODES
		#error "undefined symbol: HAL_NR_NOC_IONODES"
	#endif

	/* Sanity check. */
	#ifndef HAL_NR_NOC_CNODES
		#error "undefined symbol: HAL_NR_NOC_CNODES"
	#endif

	/* Sanity check. */
	#ifndef HAL_NR_NOC_NODES
		#error "undefined symbol: HAL_NR_NOC_NODES"
	#endif

	/* Forward definitions. */
	extern const int hal_noc_nodes[HAL_NR_NOC_NODES];

	/* Forward definitions. */
	extern int hal_get_node_id(void);

#endif /* __NEED_HAL_NOC_ */

/*============================================================================*
 * Mailbox Interface                                                          *
 *============================================================================*/

#ifdef __NEED_HAL_MAILBOX_

	/* Sanity check. */
	#ifndef HAL_NR_MAILBOX
		#error "undefined symbol: HAL_NR_MAILBOX"
	#endif

	/* Sanity check. */
	#ifndef HAL_MAILBOX_MSG_SIZE
		#error "undefined symbol: HAL_MAILBOX_MSG_SIZE"
	#endif

	/* Forward definitions. */
	extern int hal_mailbox_create(int);
	extern int hal_mailbox_open(int);
	extern int hal_mailbox_unlink(int);
	extern int hal_mailbox_close(int);
	extern size_t hal_mailbox_write(int, const void *, size_t);
	extern size_t hal_mailbox_read(int, void *, size_t);

#endif /* __NEED_HAL_MAILBOX_ */

/*============================================================================*
 * Portal Interface                                                           *
 *============================================================================*/

#ifdef __NEED_HAL_PORTAL_

	/* Forward definitions .*/
	extern int hal_portal_allow(int, int);
	extern int hal_portal_create(int);
	extern int hal_portal_open(int);
	extern int hal_portal_read(int, void *, size_t);
	extern int hal_portal_write(int, const void *, size_t);
	extern int hal_portal_close(int);
	extern int hal_portal_unlink(int);

#endif /* __NEED_HAL_PORTAL_ */

/*============================================================================*
 * Synchronization Point Interface                                            *
 *============================================================================*/

#ifdef __NEED_HAL_SYNC_

	/* Sanity check. */
	#ifndef HAL_NR_SYNC
		#error "undefined symbol: HAL_NR_SYNC"
	#endif

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

#endif /* __NEED_HAL_SYNC_ */

#endif /* NANVIX_HAL_H_ */
