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

	/**
	 * @brief NoC Address.
	 */
	struct noc_addr
	{
		int clusterid; /**< Cluster ID.      */
		int cnoc_tag;  /**< Control NoC tag. */
		int dnoc_tag;  /**< Data NoC tag.    */
	};

#endif /* NANVIX_ARCH_MPPA256_ */
