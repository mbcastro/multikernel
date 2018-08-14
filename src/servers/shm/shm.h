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

#ifndef _SHM_H_
#define _SHM_H_

	#include <sys/types.h>
	#include <stdint.h>

	#include <nanvix/klib.h>

#ifdef DEBUG_SHM
	#define shm_debug(fmt, ...) debug("shm", fmt, __VA_ARGS__)
#else
	#define shm_debug(fmt, ...) { }
#endif

	/* Forward definitions. */
	extern void buffer_init(void);
	extern int buffer_put(int, const void *);
	extern int buffer_get(int, void *);
	extern int shm_is_used(int);
	extern int shm_is_remove(int);
	extern int shm_is_owner(int, int);
	extern uint64_t shm_get_base(int);
	extern size_t shm_get_size(int);
	extern void shm_set_remove(int);
	extern void shm_set_perm(int, int, mode_t);
	extern void shm_set_name(int, const char *);
	extern void shm_set_base(int, uint64_t);
	extern void shm_set_size(int, size_t);
	extern int shm_alloc(void);
	extern int shm_get(const char *);
	extern void shm_put(int);
	extern void shm_init(void);

#endif /* _SHM_H_ */
