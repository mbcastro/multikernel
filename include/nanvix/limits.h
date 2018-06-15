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

#ifndef NANVIX_LIMITS_H_
#define NANVIX_LIMITS_H_

	#include <nanvix/hal.h>

	/**
	 * @brief Maximum length of a process name.
	 *
	 * @note The null character is included.
	 */
	#define NANVIX_PROC_NAME_MAX 50

	/**
	 * @brief Maximum number of processes.
	 */
	#define NANVIX_PROC_MAX HAL_NR_CLUSTERS

#endif /* NANVIX_LIMITS_H_ */

