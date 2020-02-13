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

#ifndef NANVIX_RUNTIME_RMEM_H_
#define NANVIX_RUNTIME_RMEM_H_

	#include <stddef.h>
	#include <nanvix/sys/page.h>

#if defined(__NEED_RMEM_CACHE)

	#define __NEED_RMEM_CLIENT

	#include <nanvix/servers/rmem.h>

	/**
	 * @#brief Size of a block in the page cache.
	 */
	#ifndef __RMEM_CACHE_BLOCK_SIZE
	#define RMEM_CACHE_BLOCK_SIZE 1
	#endif

	/**
	 * @brief Length of the page cache.
	 */
	#ifndef __RMEM_CACHE_LENGTH
	#define RMEM_CACHE_LENGTH 32
	#endif

	/**
	 * @brief Size of the page cache.
	 */
	#define RMEM_CACHE_SIZE (RMEM_CACHE_BLOCK_SIZE*RMEM_CACHE_LENGTH)

	/**
	 * @name Page replacement policies.
	 */
	/**@{*/
	#define RMEM_CACHE_FIFO  0 /**< First In First Out  */
	#define RMEM_CACHE_LIFO  1 /**< Last In First Out   */
	#define RMEM_CACHE_LRU   2 /**< Least Recently Used */
	#define RMEM_CACHE_AGING 3 /**< Aging               */
	/**@}*/

	/**
	 * @name Page Writing Policies
	 */
	/**@{*/
	#define RMEM_CACHE_WRITE_BACK    0 /**< Write Back    */
	#define RMEM_CACHE_WRITE_THROUGH 1 /**< Write Through */
	/**@}*/

	/**
	 * @brief Allocates a remote page.
	 *
	 * @returns Upon successful completion, the number of the newly
	 * allocated remote page is returned. Upon failure, @p RMEM_NULL
	 * is returned instead.
	 */
	extern rpage_t nanvix_rcache_alloc(void);

	/**
	 * @brief Cleans the cache..
	 */
	extern void nanvix_rcache_clean(void);

	/**
	 * @brief Frees a remote page.
	 *
	 * @param pgnum Number of the target page.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_rcache_free(rpage_t pgnum);

	/**
	 * @brief Gets remote page.
	 *
	 * @param pgnum Number of the target page.
	 *
	 * @returns Upon successful completion, a pointer to a local
	 * mapping of the remote page is returned. Upon failure, a @p
	 * NULL pointer is returned instead.
	 */
	extern void *nanvix_rcache_get(rpage_t pgnum);

	/**
	 * @brief Puts remote page.
	 *
	 * @param pgnum Number of the target page.
	 *
	 * @returns Upon successful completion, zero is returned. Upon failure a
	 * negative error code is returned instead.
	 */
	extern int nanvix_rcache_put(rpage_t pgnum, int strike);

	/**
	 * @brief Flushes changes on a remote page.
	 *
	 * @param pgnum Number of the target page.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure a negative error code is returned instead.
	 */
	extern int nanvix_rcache_flush(rpage_t pgnum);

	/**
	 * @brief Selects the cache replacement_policy.
	 *
	 * @param num Number of the replacement policy.
	 */
	extern int nanvix_rcache_select_replacement_policy(int num);

	/**
	 * @brief Selects the write policy.
	 *
	 * @param num Number of the policy.
	 */
	extern int nanvix_rcache_select_write(int num);

#endif /* __NEED_RMEM_CACHE */

	/**
	 * @brief Allocates remote memory.
	 *
	 * @param n Number of bytes to allocate.
	 *
	 * @returns Upon successful completion, a pointer to the newly
	 * allocated remote memory area is returned. Upon failure, a null
	 * pointer is returned instead.
	 */
	extern void *nanvix_ralloc(size_t n);

	/**
	 * @brief Frees remote memory.
	 *
	 * @param ptr Target remote memory area.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_rfree(void *ptr);

	/**
	 * @brief Reads data from remote memory.
	 *
	 * @param buf Local buffer where data should be placed.
	 * @param ptr Target remote memory area.
	 * @param n   Number of bytes to read.
	 *
	 * @returns The number of bytes read from remote memory.
	 */
	extern size_t nanvix_rread(void *buf, const void *ptr, size_t n);

	/**
	 * @brief Writes data to remote memory.
	 *
	 * @param ptr Target remote memory area.
	 * @param buf Local buffer from where data should be retrieved.
	 * @param n   Number of bytes to write.
	 *
	 * @returns The number of bytes written to remote memory.
	 */
	extern size_t nanvix_rwrite(void *ptr, const void *buf, size_t n);

	/**
	 * @brief Handles a remote page fault.
	 *
	 * @param vaddr Faulting virtual address.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_rfault(vaddr_t vaddr);

#endif /* NANVIX_RUNTIME_RMEM_H_ */
