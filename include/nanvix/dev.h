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

	#include <sys/types.h>
	#include <nanvix/vfs.h>	

	/**
	 * @brief Block device server name.
	 */
	#define BDEV_NAME "/sys/bdev"

	/**
	 * @brief Block device message types.
	 */
	/**@{*/
	#define BDEV_MSG_ERROR            1 /**< Error.               */
	#define BDEV_MSG_WRITEBLK_REQUEST 2 /**< Write block request. */
	#define BDEV_MSG_WRITEBLK_REPLY   3 /**< Write block reply.   */
	#define BDEV_MSG_READBLK_REQUEST  4 /**< Read block request.  */
	#define BDEV_MSG_READBLK_REPLY    5 /**< Read block reply.    */
	/**@}*/

	/**
	 * @brief Block device message
	 */
	struct bdev_msg
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
			 * @brief Write block request.
			 */
			struct
			{
				dev_t dev;             /**< Device number. */
				unsigned blknum;       /**< Block number. */
				char data[BLOCK_SIZE]; /**< Data.         */
			} writeblk_req;

			/**
			 * @brief Write block reply.
			 */
			struct
			{
				ssize_t n; /* Number of bytes written. */
			} writeblk_rep;

			/**
			 * @brief Read block request.
			 */
			struct
			{
				dev_t dev;       /**< Device number. */
				unsigned blknum; /**< Block number.  */
			} readblk_req;

			/**
			 * @brief Read block reply.
			 */
			struct
			{
				char data[BLOCK_SIZE]; /**< Data.                 */
				ssize_t n;             /**< Number of bytes read. */
			} readblk_rep;
			
			/**
			 * @brief Error reply.
			 */
			struct
			{
				int code; /* Error code. */
			} error_rep;

		} content;
	};

#endif /* DEV_H_ */
