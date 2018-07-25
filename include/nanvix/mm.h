/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef NANVIX_MM_H_
#define NANVIX_MM_H_
	
	#include <inttypes.h>
	#include <stddef.h>
	
	/**
	 * @brief Remote memory block size (in bytes).
	 */
	#define RMEM_BLOCK_SIZE (1024*1024)
	
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
	extern int meminit(void);
	extern int memwrite(uint64_t, const void *, size_t);
	extern int memread(uint64_t, void *, size_t);

#endif /* _MAILBOX_H_ */
