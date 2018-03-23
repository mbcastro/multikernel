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

#ifndef _KERNEL_H_
#define _KERNEL_H_

	/**
	 * @brief Data unit sizes.
	 */
	#define KB (1024)    /**< Kilobyte. */
	#define MB (1024*KB) /**< Megabyte. */
	#define GB (1024*KB) /**< Gigabyte. */

	#define milli (1.0/1000)
	#define micro (milli/1000)
	#define nano  (micro/1000)

	#define MAX_BUFFER_SIZE (1024*KB)

	#define NR_DMA 4

	#define NITERATIONS 5

	#define BARRIER_SLAVE_CNOC 4
	#define BARRIER_MASTER_CNOC 12
	#define PORTAL_DNOC 8

#endif /* _KERNEL_H_ */
