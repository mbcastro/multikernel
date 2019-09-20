/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef NANVIX_SERVERS_RMEM_H_
#define NANVIX_SERVERS_RMEM_H_

	#include <nanvix/servers/message.h>
	#include <nanvix/limits.h>
	#include <stdint.h>

	/**
	 * @brief Remote memory block size (in bytes).
	 */
	#define RMEM_BLOCK_SIZE 1024

	/**
	 * @brief Remote memory size (in bytes).
	 *
	 * @bug FIXME: This is platform-dependent.
	 */
	#define RMEM_SIZE (256*1024*1024)

	/**
	 * @brief Operations on remote memory.
	 */
	/**@{*/
	#define RMEM_EXIT     0 /**< Exit Request. */
	#define RMEM_READ     1 /**< Read.         */
	#define RMEM_WRITE    2 /**< Write.        */
	#define RMEM_MEMALLOC 3 /**< Alloc.        */
	#define RMEM_MEMFREE  4 /**< Free.        */
	/**@}*/

	/**
	 * @brief Remote memory message.
	 */
	struct rmem_message
	{
		message_header header; /**< Message header. */
		uint64_t blknum;       /**< Block number.   */
		uint32_t size;         /**< Size.           */
	};

	/**
	 * @brief Allocates a remote memory block.
	 *
	 * @returns Upon successful completion, the number of the newly
	 * allocated block in the remote memory is returned. Upon failure,
	 * a negative error code is returned instead.
	 */
	extern int nanvix_rmemalloc(void);

	/**
	 * @brief Frees a remote memory block.
	 *
	 * @param blknum Target remote memory block.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_rmemfree(uint64_t blknum);

	/**
	 * @brief Reads data from the remote memory.
	 *
	 * @param addr Remote address.
	 * @param bug  Location where the data should be written to.
	 * @param n    Number of bytes to read.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_rmemread(uint64_t addr, void *buf, size_t n);

	/**
	 * @brief Writes data to the remote memory.
	 *
	 * @param addr Remote address.
	 * @param bug  Location where the data should be read from.
	 * @param n    Number of bytes to write.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_rmemwrite(uint64_t addr, const void *buf, size_t n);

#endif /* NANVIX_SERVERS_RMEM_H_ */
