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

#ifndef HAL_H_
#define HAL_H_

#ifdef TARGET_UNIX

	#include <string.h>
	#include <stdio.h>

	/**
	 * @brief Kernel memcpy().
	 *
	 * @param dest Target memory area.
	 * @param src  Source memory area.
	 * @param n    Number of bytes to copy.
	 *
	 * @return See memcpy().
	 */
	#define kmemcpy(dest,src,n) memcpy(dest,src,n)

	/**
	 * @brief Kernel puts().
	 *
	 * @param str String to print.
	 *
	 * @return see puts();
	 */
	#define kputs(str) puts(str)

#endif

#endif /* */
