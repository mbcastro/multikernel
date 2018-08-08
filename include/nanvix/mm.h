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
	
	#include <sys/types.h>
	#include <stdint.h>
	#include <stddef.h>

/*============================================================================*
 * Remote Memory Service                                                      *
 *============================================================================*/

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

/*============================================================================*
 * Shared Memory Region Service                                               *
 *============================================================================*/

	/**
	 * @brief Maximum number of opened shared memory regions.
	 */
	#define SHM_OPEN_MAX 8

	/**
	 * @brief Maximum mapping size (in bytes).
	 */
	#define SHM_MAP_SIZE_MAX (512*1024)

	/**
	 * @brief Maximum length for a shared memory region name.
	 */
	#define SHM_NAME_MAX 55

	/**
	 * @bried Shared memory region operations.
	 */
	/**@{*/
	#define SHM_OPEN        1 /**< Open.             */
	#define SHM_CREATE      2 /**< Create.           */
	#define SHM_CREATE_EXCL 3 /**< Exclusive create. */
	#define SHM_UNLINK      4 /**< Unlink.           */
	#define SHM_MAP         5 /**< Map.              */
	#define SHM_UNMAP       6 /**< Unmap.            */
	#define SHM_TRUNCATE    7 /**< Truncate.         */
	#define SHM_RETURN      8 /**< Return.           */
	#define SHM_FAILED      9 /**< Return.           */
	/**@}*/

	/**
	 * @brief Shared Memory Region message.
	 */
	struct shm_message
	{
		uint16_t source; /**< Source cluster.                 */
		int8_t opcode;   /**< Shared Memory Region operation. */
		uint16_t seq;    /**< Sequence number.                */

		/* Operation-specific fields. */
		union 
		{
			/* Create message 1. */
			struct {
				char name[SHM_NAME_MAX]; /**< Shared Memory Region name. */
			} create1;

			/* Create message 2. */
			struct {
				mode_t mode;  /**< Access permissions. */
				int excl;     /**< Exclusive creation? */
				int rw;       /**< Read write?         */
				int truncate; /**< Truncate?           */
			} create2;

			/* Open message 1. */
			struct {
				char name[SHM_NAME_MAX]; /**< Shared Memory Region name. */
			} open1;

			/* Open message 2. */
			struct {
				int rw;       /**< Read write? */
				int truncate; /**< Truncate?   */
			} open2;

			/* Unlink message. */
			struct {
				char name[SHM_NAME_MAX]; /**< Shared Memory Region name. */
			} unlink;

			/* Map message. */
			struct {
				int shmid;    /**< Target shared memory region.           */
				size_t size;  /**< Mapping size.                          */
				int writable; /**< Writable mapping?                      */
				int shared;   /**< Shared mapping?                        */
				off_t off;    /**< Offset in target shared memory region. */
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
				int status;      /**< Status code.           */
				uint64_t mapblk; /**< Mapped remote address. */
			} ret;
		} op;
	};

	/* Forward definitions. */
	extern int nanvix_shm_init(void);
	extern void nanvix_shm_finalize(void);
	extern int nanvix_shm_create(const char *, int, int, mode_t);
	extern int nanvix_shm_create_excl(const char *, int, mode_t);
	extern int nanvix_shm_open(const char *, int, int);
	extern int nanvix_shm_unlink(const char *);
	extern void *nanvix_mmap(size_t, int, int, int, off_t);
	extern int nanvix_munmap(void *, size_t);
	extern int nanvix_msync(void *, size_t, int, int);
	extern int nanvix_mtruncate(int, size_t);

#endif /* _MAILBOX_H_ */
