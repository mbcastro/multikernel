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

#ifndef NANVIX_RUNTIME_SHM_H_
#define NANVIX_RUNTIME_SHM_H_

#ifdef __SHM_SERVICE

	#include <nanvix/servers/shm.h>

#endif /* __SHM_SERVICE */

	#include <posix/sys/types.h>
	#include <posix/stdint.h>

#ifdef __SHM_SERVICE

	/**
	 * @brief Initializes the SHM Service.
	 *
	 * @returns Upon successful completion, zero is returned. Upon failure,
	 * a negative error code is returned instead.
	 */
	extern int __nanvix_shm_setup(void);

	/**
	 * @brief Shutdowns the SHM Service.
	 */
	extern int __nanvix_shm_cleanup(void);

#endif /* __SHM_SERVICE */

	/**
	 * @brief Shutdowns the SHM Service.
	 */
	extern int nanvix_shm_shutdown(void);

	/**
	 * @brief Creates a shared memory region.
	 *
	 * @param name     Name of the new shared memory region.
	 * @param rw       Read write? Otherwise read-only.
	 * @param truncate Truncate region?
	 * @param mode     Access permissions.
	 *
	 * @returns Upon successful completion, a descriptor for the newly
	 * created shared memory region is returned. Upon failure, a negative
	 * error code is returned instead.
	 */
	extern int __nanvix_shm_create(const char *name, int rw, int truncate, mode_t mode);

	/**
	 * @brief Creates an exclusive shared memory region.
	 *
	 * @param name Name of the new shared memory region.
	 * @param rw   Read write? Otherwise read-only.
	 * @param mode Access permissions.
	 *
	 * @returns Upon successful completion, a descriptor for the newly
	 * created shared memory region is returned. Upon failure, a
	 * negative error code is returned instead.
	 */
	extern int __nanvix_shm_create_excl(const char *name, int rw, mode_t mode);

	/**
	 * @brief Opens a shared memory region.
	 *
	 * @param name     Name of the new shared memory region.
	 * @param rw       Read write? Otherwise read-only.
	 * @param truncate Truncate region?
	 *
	 * @returns Upon successful completion, a descriptor for the target
	 * shared memory region is returned. Upon failure, a negative error
	 * code is returned instead.
	 */
	extern int __nanvix_shm_open(const char *name, int rw, int truncate);

	/**
	 * @brief Removes a shared memory region.
	 *
	 * @param name Name of the target existing shared memory region.
	 *
	 * @returns Upon successful completion, a descriptor for the target
	 * shared memory region is returned. Upon failure, a negative error
	 * code is returned instead.
	 */
	extern int __nanvix_shm_unlink(const char *name);

	/**
	 * @brief Maps pages of memory.
	 *
	 * @param mapblk   Location at mapped block should be stored.
	 * @param len      Length of mapping (in bytes).
	 * @param writable Writable? Else read-only.
	 * @param shared   Shared? Else private.
	 * @param fd       Target file descriptor.
	 * @param off      Offset within file.
	 *
	 * @retuns Upon successful completion, zero is returned. Upon
	 * failure,
	 * -1 is returned instead and errno is set to indicate the error.
	 */
	extern int __nanvix_map(
		uint64_t *mapblk,
		size_t len,
		int writable,
		int shared,
		int fd,
		off_t off
	);

	/**
	 * @brief Unmaps pages of memory.
	 *
	 * @param shmid ID of the target shared memory region.
	 * @param len   Length of mapping (in bytes).
	 *
	 * @retuns Upon successful completion, zero is returned. Otherwise,
	 * -1 is returned and errno is set to indicate the error.
	 */
	extern int __nanvix_unmap(int shmid, size_t len);

	/**
	 * @brief Truncates a shared memory region to a specified size.
	 *
	 * @param shmid  ID of the target shared memory region.
	 * @param length Shared memory region size (in bytes).
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, -1 is returned instead, and errno is set to indicate the
	 * error.
	 */
	extern int __nanvix_mtruncate(int shmid, size_t size);

	/**
	 * @brief Maps pages of memory.
	 *
	 * @param len      Length of mapping (in bytes).
	 * @param writable Writable? Else read-only.
	 * @param shared   Shared? Else private.
	 * @param fd       Target file descriptor.
	 * @param off      Offset within file.
	 *
	 * @retuns Upon successful completion, the address at which the
	 * mapping was placed is returned. Otherwise, NULL is returned
	 * and errno is set to indicate the error.
	 */
	extern void *__nanvix_mmap(size_t len, int writable, int shared, int fd, off_t off);

	/**
	 * @brief Unmaps pages of memory.
	 *
	 * @param addr  Mapping address.
	 * @param len   Length of mapping (in bytes).
	 *
	 * @retuns Upon successful completion, zero is returned. Otherwise, -1
	 * is returned and errno is set to indicate the error.
	 */
	extern int __nanvix_munmap(void *addr, size_t len);

	/**
	 * @brief Truncates a file to a specified length.
	 *
	 * @param fd     Target file descriptor.
	 * @param length File length (in bytes).
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, -1 is returned instead, and errno is set to indicate the
	 * error.
	 */
	extern int __nanvix_ftruncate(int fd, off_t length);

	/**
	 * @brief Synchronizes memory with physical storage.
	 *
	 * @param addr       Target local address.
	 * @param len        Number of bytes to synchronize.
	 * @param sync       Synchronous write? Else asynchronous.
	 * @param invalidate Invaldiate cached data? Else no.
	 *
	 * @para Upon successful completion, zero is returned. Upon failure,
	 * -1 is returned instead and errno is set to indicate the error.
	 */
	extern int __nanvix_msync(void *addr, size_t len, int sync, int invalidate);

#endif /* NANVIX_RUNTIME_SHM_H_ */
