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

#ifndef KLIB_H_
#define KLIB_H_

	#include <stdarg.h>

	#include <nanvix/hal.h>

	/**
	 * @brief Success return code.
	 */
	#define NANVIX_SUCCESS 0

	/**
	 * @brief Failure return code.
	 */
	#define NANVIX_FAILURE 1

	/**
	 * @brief Kernel buffer size.
	 */
	#define KBUFFER_SIZE 4096

	/* Forward definitions. */
	extern void *kmemcpy (void* , const void *, size_t);
	extern void *kmemset(void *, int, size_t);
	extern int kstrcmp(const char *, const char *);
	extern char *kstrcpy(char *, const char *);
	extern size_t kstrlen(const char *);
	extern int kstrncmp(const char *, const char *, size_t);
	extern char *kstrncpy(char *, const char *, size_t);
	extern int kvsprintf(char *, const char *, va_list);
	extern void kprintf(const char *, ...);
	extern void kpanic(const char *, ...);
	extern void kdebug(const char *, ...);
	extern void *kmalloc(size_t);
	extern void kfree(void *);

#endif /* KLIB_H_ */
