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

#ifndef KLIB_H_
#define KLIB_H_

	#include <nanvix/hal.h>

	/**
	 * @brief Success return code.
	 */
	#define NANVIX_SUCCESS 0

	/**
	 * @brief Failure return code.
	 */
	#define NANVIX_FAILURE 1

	/**
	 * @brief Prints debug messages.
	 *
	 * @param str Debugging message.
	 */
	#define kdebug(str) kputs(str)

#endif /* KLIB_H_ */
