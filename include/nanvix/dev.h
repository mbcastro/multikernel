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
	#define RMEM_MSG_ERROR            1 /**< Error.               */
	#define RMEM_MSG_WRITEBLK_REQUEST 2 /**< Write block request. */
	#define RMEM_MSG_WRITEBLK_REPLY   3 /**< Write block reply.   */
	#define RMEM_MSG_READBLK_REQUEST  4 /**< Read block request.  */
	#define RMEM_MSG_READBLK_REPLY    5 /**< Read block reply.    */
	/**@}*/

	/**
	 * @brief Remote memory message header.
	 */
	struct rmem_msg_header
	{
		/**
		 * @brief Operation code.
		 */
		int opcode;

		/**
		 * @brief Operation parameters.
		 */
		union
		{
			/**
			 * @brief Read/write.
			 */
			struct
			{
				dev_t dev;       /**< Device number. */
				unsigned blknum; /**< Block number.  */
			} rw;

			/**
			 * @brief Error
			 */
			struct
			{
				int num; /**< Error number. */
			} err;
		} param;
	};

	/**
	 * Remote memory message payload.
	 */
	struct rmem_msg_payload
	{
		char data[BLOCK_SIZE]; /**< Data. */
	};

#endif /* DEV_H_ */
