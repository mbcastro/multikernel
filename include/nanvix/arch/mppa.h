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

	#define NR_CLUSTER 16

	#define CLUSTER0 0
	#define CLUSTER1 1

	extern void nanvix_connector_init(void);
	extern int nanvix_connector_receive(void *, size_t);
	extern int nanvix_connector_send(int, const void *, size_t);

#endif /* NANVIX_ARCH_MPPA256_ */
