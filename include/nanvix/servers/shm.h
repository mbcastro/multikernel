/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
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

#ifndef NANVIX_SERVERS_SHM_H_
#define NANVIX_SERVERS_SHM_H_

	#if !defined(__SHM_SERVICE) && !defined(__SHM_SERVER)
	#error "do not include this file"
	#endif

	#include <nanvix/servers/message.h>
	#include <nanvix/limits/shm.h>
	#include <posix/sys/types.h>
	#include <nanvix/sys/page.h>
	#include <posix/stdint.h>
	#include <posix/stddef.h>

	/**
	 * @brief Node number for SHM Server.
	 */
	#define SHM_SERVER_NAME "/shm"

	/**
	 * @brief Port number of SHMem server.
	 */
	#define SHM_SERVER_PORT_NUM 2

	/**
	 * @bried Shared memory region operations.
	 */
	/**@{*/
	#define SHM_EXIT         0  /**< Exit Request.     */
	#define SHM_OPEN         1  /**< Open.             */
	#define SHM_CREATE       2  /**< Create.           */
	#define SHM_CREATE_EXCL  3  /**< Exclusive create. */
	#define SHM_UNLINK       4  /**< Unlink.           */
	#define SHM_MAP          5  /**< Map.              */
	#define SHM_UNMAP        6  /**< Unmap.            */
	#define SHM_TRUNCATE     7  /**< Truncate.         */
	#define SHM_SUCCESS      8  /**< Success.          */
	#define SHM_FAIL         9  /**< Failure.          */
	/**@}*/

	/**
	 * @brief Shared Memory Region message.
	 */
	struct shm_message
	{
		message_header header; /**< Message header.  */

		/* Operation-specific fields. */
		union
		{
			/* Create message 1. */
			struct {
				char name[SHM_NAME_MAX]; /**< Shared Memory Region name. */
				mode_t mode;             /**< Access permission.         */
				int excl;                /**< Exclusive creation?        */
				int rw;                  /**< Read write?                */
				int truncate;            /**< Truncate?                  */
			} create;

			/* Open message. */
			struct {
				char name[SHM_NAME_MAX]; /**< Shared Memory Region name. */
				int rw;                  /**< Read write?                */
				int truncate;            /**< Truncate?                  */
			} open;

			/* Unlink message. */
			struct {
				char name[SHM_NAME_MAX]; /**< Shared Memory Region name. */
			} unlink;

			/* Map message. */
			struct {
				int shmid;    /**< Target shared memory region. */
				size_t size;  /**< Mapping size.                */
				int writable; /**< Writable mapping?            */
				int shared;   /**< Shared mapping?              */
				off_t off;    /**< Map offset.                  */
			} map;

			/* Unmap message. */
			struct {
				int shmid;   /**< Target shared memory region. */
				size_t size; /**< Mapping size.                */
			} unmap;

			/**
			 * Truncate message.
			 */
			struct {
				int shmid;   /**< Target shared memory region. */
				size_t size; /**< Size (in bytes).             */
			} truncate;

			/* Return message. */
			union
			{
				int shmid;       /**< ID of shared memory region.  */
				int status;      /**< Status code.                 */
				uint64_t mapblk; /**< Mapped remote address.       */
			} ret;
		} op;
	};


#ifdef __SHM_SERVER

	/**
	 * @brief Debug SHM Server?
	 */
	#define __DEBUG_SHM 0

	#if (__DEBUG_SHM)
	#define shm_debug(fmt, ...) uprintf(fmt, __VA_ARGS__)
	#else
	#define shm_debug(fmt, ...) { }
	#endif

	/**
	 * @brief Creates a shared memory region
	 *
	 * @param owner    ID of owner process.
	 * @param name     Name of the targeted shm.
	 * @param writable Writable? Else read-only.
	 * @param mode     Access permissions.
	 *
	 * @returns Upon successful completion, the ID of the newly created
	 * opened shared memory region is returned. Upon failure, a negative
	 * error code is returned instead.
	 */
	extern int __do_shm_create(int owner, const char *name, int writable, mode_t mode);

	/**
	 * @brief Open a shared memory region with existence check
	 *
	 * @param owner    ID of owner process.
	 * @param name     Name of the targeted shared memory region.
	 * @param writable Writable? Else read-only.
	 * @param mode     Access permissions.
	 *
	 * @returns Upon successful completion, the newly created shared
	 * memory region ID is returned. Upon failure, a negative error code
	 * is returned instead.
	 */
	extern int __do_shm_create_exclusive(int owner, char *name, int writable, mode_t mode);

	/**
	 * @brief Opens a shared memory region
	 *
	 * @param node     ID of opening process.
	 * @param name     Name of the targeted shared memory region.
	 * @param writable Writable? Else read-only.
	 * @param truncate Truncate? Else not.
	 *
	 * @returns Upon successful completion, the shared memory region ID is
	 * returned. Upon failure, a negative error code is returned instead.
	 */
	extern int __do_shm_open(int node, const char *name, int writable, int truncate);

	/**
	 * @brief Close a opened shared memory region
	 *
	 * @param node   ID of opening process.
	 * @param oshmid Opened shared memory region id.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __do_shm_close(int node, int oshmid);

	/**
	 * @brief Unlink a shared memory region
	 *
	 * @param node ID the calling process.
	 * @param name Name of the targeted shm.
	 *
	 * @returns Upon successful completion, oshmid is returned.
	 * Upon failure, a negative error code is returned instead.
	 */
	extern int __do_shm_unlink(int node, const char *name);

	/**
	 * @brief Truncates a shared memory region to a specified size.
	 *
	 * @param node   ID of opening process.
	 * @param oshmid ID of the opened shared memory region.
	 * @param length Shared memory region size (in bytes).
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, -1 is returned instead, and errno is set to indicate the
	 * error.
	 */
	extern int __do_shm_truncate(int node, int oshmid, size_t size);

	/**
	 * @brief Maps a shared memory region.
	 *
	 * @param node     ID of the calling node.
	 * @param oshmid   ID of the opened shared memory region.
	 * @param size     Size of mapping.
	 * @param writable Writable mapping? Else read-only.
	 * @param shared   Shared mapping? Else private.
	 * @param off      Offset within shared memory region.
	 * @param mapblk   Place which the mapping address should be stored.
	 *
	 * @returns Upon successful completion, zero is returned and the
	 * mapping address is stored into @p mapblk. Upon failure, a negative
	 * error code is returned instead.
	 */
	extern int __do_shm_map(
		int node,
		int oshmid,
		size_t size,
		int writable,
		int shared,
		off_t off,
		uint64_t *mapblk);

	/**
	 * @brief Unmaps a shared memory region.
	 *
	 * @param node   ID of the calling node.
	 * @param oshmid ID of the opened shared memory region.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __do_shm_unmap(int node, int oshmid);

	extern int shm_is_writable(int shmid);
	extern int shm_get(const char *name);
	extern int shm_is_used(int shmid);
	extern int shm_is_owner(int shmid, int node);
	extern void shm_set_perm(int shmid, int owner, mode_t mode);
	extern void shm_set_name(int shmid, const char *name);
	extern int shm_alloc(void);
	extern size_t shm_get_size(int shmid);
	extern int shm_is_remove(int shmid);
	extern void shm_set_size(int shmid, size_t size);
	extern void shm_set_remove(int shmid);
	extern void shm_put(int shmid);
	extern int shm_is_readable(int shmid);
	extern uint64_t shm_get_base(int shmid);
	extern void shm_init(void);
	extern void shm_set_base(int shmid, uint64_t base);

#endif /*__SHM_SERVER */

#endif /* _MAILBOX_H_ */
