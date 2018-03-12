/*
 * Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#ifndef NANVIX_ARCH_MPPA256_
#define NANVIX_ARCH_MPPA256_

	#ifndef _KALRAY_MPPA256_
		#error "bad target"
	#endif

	#include <mppaipc.h>

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
	#define CCLUSTER0    1 /**< Compute cluster  1. */
	#define CCLUSTER0    2 /**< Compute cluster  2. */
	#define CCLUSTER0    3 /**< Compute cluster  3. */
	#define CCLUSTER0    4 /**< Compute cluster  4. */
	#define CCLUSTER0    5 /**< Compute cluster  5. */
	#define CCLUSTER0    6 /**< Compute cluster  6. */
	#define CCLUSTER0    7 /**< Compute cluster  7. */
	#define CCLUSTER0    8 /**< Compute cluster  8. */
	#define CCLUSTER0    9 /**< Compute cluster  9. */
	#define CCLUSTER0   10 /**< Compute cluster 10. */
	#define CCLUSTER0   11 /**< Compute cluster 11. */
	#define CCLUSTER0   12 /**< Compute cluster 12. */
	#define CCLUSTER0   13 /**< Compute cluster 13. */
	#define CCLUSTER0   14 /**< Compute cluster 14. */
	#define CCLUSTER0   15 /**< Compute cluster 15. */
	#define IOCLUSTER0 128 /**< IO cluster 0.       */
	#define IOCLUSTER1 192 /**< IO cluster 1.       */

	/* Forward definitions. */
	extern void nanvix_connector_init(void);
	extern int nanvix_connector_receive(void *, size_t);
	extern int nanvix_connector_send(int, const void *, size_t);

#endif /* NANVIX_ARCH_MPPA256_ */
