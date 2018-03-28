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
	 * @brief Data unit prefixes.
	 */
	/**@{*/
	#define KB (1024)    /**< Kilobyte. */
	#define MB (1024*KB) /**< Megabyte. */
	#define GB (1024*KB) /**< Gigabyte. */
	/**@}*/

	/**
	 * @brief Physical unit prefixes.
	 */
	/**@{*/
	#define MILLI (1.0/1000)   /**< Milli 10^-3 */
	#define MICRO (MILLI/1000) /**< Micro 10^-6 */
	#define NANO  (MICRO/1000) /**< Nano 10^-9  */
	/**@}*/

	#define MAX_BUFFER_SIZE (1024*KB)

	#define NITERATIONS 30

#endif /* _KERNEL_H_ */
