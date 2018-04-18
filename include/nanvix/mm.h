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

#ifndef NANVIX_MM_H_
#define NANVIX_MM_H_
	
	#include <inttypes.h>
	#include <stddef.h>
	
	/**
	 * @brief Remote memory block size (in bytes).
	 */
	#define RMEM_BLOCK_SIZE (16*1024)
	
	/**
	 * @brief Remote memory size (in bytes).
	 */
	#define RMEM_SIZE ((1024 + 256)*1024*1024)

	/**
	 * @brief Operations on remote memory.
	 */
	/**@{*/
	#define RMEM_READ   0 /**< Read.   */
	#define RMEM_WRITE  1 /**< Write.  */
	/**@}*/

	/**
	 * @brief remote memory message.
	 */
	struct rmem_message
	{
		uint16_t source;     /**< Source cluster. */
		uint16_t op;         /**< Operation.      */
		uint64_t blknum;     /**< Block number.   */
		uint32_t size;       /**< Size.           */
		uint32_t unused[12]; /**< Not used.       */
	};

	/* Forward definitions. */
	extern void memwrite(uint64_t, const void *, size_t);
	extern void memread(uint64_t, void *, size_t);

#endif /* _MAILBOX_H_ */
