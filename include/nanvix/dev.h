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

#ifndef DEV_H_
#define DEV_H_

	#define RAMDISK_MAJOR 0x0 /**< RAM Disk device. */

	/**
	 * @brief Returns the major number of a device.
	 */
	#define MAJOR(dev) \
		(((dev) >> 8) & 0xf)
	
	/**
	 * @brief Returns the minor number of a device.
	 */
	#define MINOR(dev) \
		(((dev) >> 4) & 0xf)

#endif /* DEV_H_ */
