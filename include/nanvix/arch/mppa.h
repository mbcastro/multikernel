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
	 * @brief Number of cluster in the platform.
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
		#define HAL_NR_MAILBOX (4*HAL_NR_NOC_NODES)
	#endif

	/**
	 * @brief Size (in bytes) of a mailbox message.
	 */
	#define HAL_MAILBOX_MSG_SIZE 64

/*===========================================================================*
 * Synchronization Interface                                                 *
 *===========================================================================*/

	/**
	 * @brief Number of mailboxes.
	 */
	#ifdef _KALRAY_MPPA_CCLUSTER_
		#define HAL_NR_SYNC HAL_NR_NOC_NODES
	#else
		#define HAL_NR_SYNC (4*HAL_NR_NOC_NODES)
	#endif

/*===========================================================================*
 * Portal                                                                    *
 *===========================================================================*/

	/**
	 * @brief HAL portal.
	 */
	typedef struct
	{
		int portal_fd;  /* Portal NoC connector.     */
		int sync_fd;    /* Sync NoC connector.       */
		int remote;     /* Remote NoC node ID.       */
		int local;      /* Local NoC node ID.        */
	} portal_t;

#endif /* NANVIX_ARCH_MPPA256 */
