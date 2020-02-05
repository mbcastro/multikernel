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

	#include <nanvix/kernel/kernel.h>

#if defined(__NEED_RMEM_CLIENT) || defined(__RMEM_SERVICE)

	#include <nanvix/servers/message.h>
	#include <nanvix/servers/spawn.h>
	#include <stdint.h>
	#include <stddef.h>

#endif /* __RMEM_SERVICE */

	/**
	 * @brief Null remote address.
	 */
	#define RMEM_NULL 0

	/**
	 * @brief Remote memory shift.
	 */
	#define RMEM_BLOCK_SHIFT PAGE_SHIFT

	/**
	 * @brief Remote memory block size (in bytes).
	 */
	#define RMEM_BLOCK_SIZE 4096

	/**
	 * @brief Remote memory size (in bytes).
	 */
	#define RMEM_SIZE ((65536*1024)/2)

	/**
	 * @brief Number of remote memory blocks.
	 */
	#define RMEM_NUM_BLOCKS (RMEM_SIZE/RMEM_BLOCK_SIZE)

	/**
	 * @name Shifts for remote addresses.
	 */
	/**@{*/
	#define RMEM_BLOCK_NUM_SHIFT    0ULL
	#define RMEM_BLOCK_SERVER_SHIFT 24ULL
	/**@}*/

	/**
	 * @name Masks for remote addresses.
	 */
	/**@{*/
	#define RMEM_BLOCK_NUM_MASK    (0xffffff << RMEM_BLOCK_NUM_SHIFT)    /**< Block Number  */
	#define RMEM_BLOCK_SERVER_MASK (    0xff << RMEM_BLOCK_SERVER_SHIFT) /**< Server Number */
	/**@}*/

	/**
	 * @brief Gets block number.
	 *
	 * The @p RMEM_BLOCK_NUM macro returns the number of the remote
	 * memory block encoded in a remote memory address.
	 *
	 * @param x Target remote address.
	 */
	#define RMEM_BLOCK_NUM(x) \
		(((x) & RMEM_BLOCK_NUM_MASK) >> RMEM_BLOCK_NUM_SHIFT)

	/**
	 * @brief Gets server number.
	 *
	 * The @p RMEM_BLOCK_SERVER macro returns the number of the remote
	 * memory server encoded in a remote memory address.
	 *
	 * @param x Target remote address.
	 */
	#define RMEM_BLOCK_SERVER(x) \
		(((x) & RMEM_BLOCK_SERVER_MASK) >> RMEM_BLOCK_SERVER_SHIFT)


	/**
	 * @brief Builds a remote memory address.
	 */
	#define RMEM_BLOCK(server, num) ( \
		(((word_t)(server) << RMEM_BLOCK_SERVER_SHIFT) & RMEM_BLOCK_SERVER_MASK) | \
		(((word_t)(num) << RMEM_BLOCK_NUM_SHIFT) & RMEM_BLOCK_NUM_MASK)            \
	)

#if defined(__NEED_RMEM_CLIENT) || defined(__RMEM_SERVICE)

	/**
	 * @brief Remote page number.
	 */
	typedef word_t rpage_t;

	/**
	 * @brief Remote address.
	 */
	typedef word_t raddr_t;

#endif /* __NEED_RMEM_CLIENT || __RMEM_SERVICE */

#if defined(__RMEM_SERVICE)

	/**
	 * @brief Operations on remote memory.
	 */
	/**@{*/
	#define RMEM_EXIT    0 /**< Exit        */
	#define RMEM_READ    1 /**< Read        */
	#define RMEM_WRITE   2 /**< Write       */
	#define RMEM_ALLOC   3 /**< Alloc       */
	#define RMEM_MEMFREE 4 /**< Free        */
	#define RMEM_ACK     5 /**< Acknowledge */
	/**@}*/

	/**
	 * @brief Remote memory message.
	 */
	struct rmem_message
	{
		message_header header; /**< Message header. */
		rpage_t blknum;        /**< Block number.   */
		int errcode;           /**< Error code.     */
	};

	/**
	 * @brief Table of RMem Servers.
	 */
	extern struct rmem_servers_info
	{
		int nodenum;
		const char *name;
	} rmem_servers[RMEM_SERVERS_NUM];

#endif /* __RMEM_SERVICE */

#if defined(__NEED_RMEM_CLIENT)

	/**
	 * @brief Allocates a remote memory block.
	 *
	 * @returns Upon successful completion, the number of the newly
	 * allocated block in the remote memory is returned. Upon failure,
	 * a negative error code is returned instead.
	 */
	extern rpage_t nanvix_rmem_alloc(void);

	/**
	 * @brief Frees a remote memory block.
	 *
	 * @param blknum Number of the target block.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_rmem_free(rpage_t blknum);

	/**
	 * @brief Reads data from the remote memory.
	 *
	 * @param blknum Number of the target block.
	 * @param buf    Location where the data should be written to.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern size_t nanvix_rmem_read(rpage_t blknum, void *buf);

	/**
	 * @brief Writes data to the remote memory.
	 *
	 * @param blknum Number of the target block.
	 * @param buf    Location where the data should be read from.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern size_t nanvix_rmem_write(rpage_t blknum, const void *buf);

	/**
	 * @brief Shutdowns a remote memory server.
	 *
	 * @param serverid ID of the target server.
	 *
	 * @returns Upon successful completion 0 is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_rmem_shutdown(int serverid);

#endif /* __NEED_RMEM_CLIENT  */

#endif /* NANVIX_SERVERS_RMEM_H_ */
