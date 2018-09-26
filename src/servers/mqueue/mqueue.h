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

#ifndef _MQUEUE_H_
#define _MQUEUE_H_

	#include <sys/types.h>

	#include <nanvix/klib.h>

#ifdef DEBUG_MQUEUE
	#define mqueue_debug(fmt, ...) debug("mqueue", fmt, __VA_ARGS__)
#else
	#define mqueue_debug(fmt, ...) { }
#endif

	/* Forward definitions. */
	extern void buffer_init(void);
	extern int buffer_put(int, const void *);
	extern int buffer_get(int, void *);
	extern int mqueue_is_used(int);
	extern int mqueue_is_remove(int);
	extern int mqueue_is_owner(int, int);
	extern int mqueue_is_readable(int);
	extern int mqueue_is_writable(int);
	extern size_t mqueue_get_size(int);
	extern void mqueue_set_remove(int);
	extern void mqueue_set_perm(int, int, mode_t);
	extern void mqueue_set_name(int, const char *);
	extern void mqueue_set_size(int, size_t);
	extern int mqueue_alloc(void);
	extern int mqueue_get(const char *);
	extern int mqueue_is_full(int);
	extern int mqueue_is_empty(int);
	extern void *mqueue_slot_alloc(int, unsigned);
	extern void mqueue_slot_free(int, void *);
	extern void *mqueue_get_first_slot(int, unsigned *);
	extern void mqueue_remove_first_slot(int);
	extern void mqueue_put(int);
	extern void mqueue_init(void);

#endif /* _MQUEUE_H_ */
