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
	#define BDEV_NAME "bdev"

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

	#define bdev_msg_build_writeblk_request(msg,dev,blknum,data)  \
	{                                                             \
		msg.type = BDEV_MSG_WRITEBLK_REQUEST;                     \
		msg.content.writeblk_req.dev = dev;                       \
		msg.content.writeblk_req.blknum = blknum;                 \
		kmemcpy(msg.content.writeblk_req.data, data, BLOCK_SIZE); \
	}

	#define bdev_msg_extract_writeblk_request(msg,dev,blknum,data) \
	{                                                              \
		dev = msg.content.writeblk_req.dev;                        \
		blknum = msg.content.writeblk_req.blknum;                  \
		kmemcpy(data, msg.content.writeblk_req.data, BLOCK_SIZE);  \
	}
	
	#define bdev_msg_build_writeblk_reply(msg,n) \
	{                                            \
		msg.type = BDEV_MSG_WRITEBLK_REPLY;      \
		msg.content.writeblk_rep.n = n;          \
	}

	#define bdev_msg_extract_writeblk_reply(msg,n) \
	{                                              \
		n = msg.content.writeblk_rep.n;            \
	}

	#define bdev_msg_build_readblk_request(msg,dev,blknum) \
	{                                                      \
		msg.type = BDEV_MSG_READBLK_REQUEST;               \
		msg.content.readblk_req.dev = dev;                 \
		msg.content.readblk_req.blknum = blknum;           \
	}

	#define bdev_msg_extract_readblk_request(msg,dev,blknum) \
	{                                                        \
		dev = msg.content.readblk_req.dev;                   \
		blknum = msg.content.readblk_req.blknum;             \
	}

	#define bdev_msg_build_readblk_reply(msg,data,n)             \
	{                                                            \
		msg.type = BDEV_MSG_READBLK_REPLY;                       \
		msg.content.readblk_rep.n = n;                           \
		kmemcpy(msg.content.readblk_rep.data, data, BLOCK_SIZE); \
	}

	#define bdev_msg_extract_readblk_reply(msg,data,n)           \
	{                                                            \
		n = msg.content.readblk_rep.n;                           \
		kmemcpy(data, msg.content.readblk_rep.data, BLOCK_SIZE); \
	}
	
	#define bdev_msg_build_error_rep(msg,code) \
	{                                          \
		msg.content.error_rep = code;          \
	}

	#define bdev_msg_extract_error_rep(msg,code) \
	{                                            \
		code = msg.content.error_rep;            \
	}

#endif /* DEV_H_ */
