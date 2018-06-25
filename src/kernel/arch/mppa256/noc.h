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

#ifndef _MPPA256_NOC_H_
#define _MPPA256_NOC_H_

	#ifndef _KALRAY_MPPA256
		#error "bad target"
	#endif

	#include <inttypes.h>
	#include <HAL/hal/core/timer.h>
	#include <HAL/hal/core/diagnostic.h>
#ifdef _KALRAY_MPPA_256_HIGH_LEVEL
	#include <mppaipc.h>
	#include <pthread.h>
#endif
#ifdef _KALRAY_MPPA_256_LOW_LEVEL
	#include <mppa_power.h>
	#include <mppa_rpc.h>
	#include <mppa_async.h>
	#include <utask.h>
#endif

	/**
	 * @brief Number DMAs per compute cluster.
	 */
	#define NR_CCLUSTER_DMA 1

	/**
	 * @brief Number of DMAs per compute cluster.
	 */
	#define NR_IOCLUSTER_DMA 4

	/* Forward definitions. */
	extern int noctag_mailbox(int);
	extern int noctag_sync(int);
	extern int noctag_portal(int);
	extern int noc_get_node_num(int);
	extern void noc_get_remotes(char *, int);
	extern void noc_get_names(char *, const int *, int);
	extern int noc_get_dma(int);
	extern int noc_is_ionode(int);
	extern int noc_is_ionode0(int);
	extern int noc_is_ionode1(int);
	extern int noc_is_cnode(int);

#endif /* _MPPA256_NOC_H_ */
