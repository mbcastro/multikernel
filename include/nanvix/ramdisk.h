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

#ifndef RAMDISK_H_
#define RAMDISK_H_

	#include <sys/types.h>

	/**
	 * @brief RAM Disk size (in bytes).
	 */
	#define RAMDISK_SIZE 4096

	/**
	 * @brief RAM Disk device driver service name.
	 */
	#define RAMDISK_NAME "/tmp/ramdisk"

	/**
	 * @brief RAM Disk message buffer size (in bytes).
	 */
	#define RAMDISK_MSG_BUF_SIZE 512

	/**
	 * @brief RAM Disk message types.
	 */
	/**@{*/
	#define RAMDISK_MSG_ERROR         1 /**< Error.         */
	#define RAMDISK_MSG_WRITE_REQUEST 2 /**< Write request. */
	#define RAMDISK_MSG_WRITE_REPLY   3 /**< Write reply.   */
	#define RAMDISK_MSG_READ_REQUEST  4 /**< Read request.  */
	#define RAMDISK_MSG_READ_REPLY    5 /**< Read reply.    */
	/**@}*/
	/**
	 * @brief RAM Disk message.
	 */
	struct ramdisk_message
	{
		/**
		 * @brief Message type.
		 */
		int type;

		/**
		 * @brief Message content.
		 */
		union
		{
			/**
			 * @brief Write request.
			 */
			struct
			{
				unsigned minor;                  /**< Minor device number. */
				unsigned blknum;                 /**< Block number.        */
				char data[RAMDISK_MSG_BUF_SIZE]; /**< Data.                */
			} write_req;

			/**
			 * @brief Write reply.
			 */
			struct
			{
				ssize_t n; /* Number of bytes written. */
			} write_rep;

			/**
			 * @brief Read request.
			 */
			struct
			{
				unsigned minor;  /**< Minor device number. */
				unsigned blknum; /**< Block number.        */
			} read_req;

			/**
			 * @brief Read reply.
			 */
			struct
			{
				char data[RAMDISK_MSG_BUF_SIZE]; /**< Data.                 */
				ssize_t n;                       /**< Number of bytes read. */
			} read_rep;
		} content;
	};

#endif /* RAMDISK_H_ */
