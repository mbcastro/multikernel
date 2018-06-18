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

#ifndef _MASTER_H_
#define _MASTER_H_

	#include <nanvix/arch/mppa.h>
	#include <stddef.h>

	/*===============================================================*
	 * IPC                                                           *
	 *===============================================================*/

	/**
	 * @brief Binary name of slave process.
	 */
	#define KM_SLAVE_BINARY "km-portal-slave"

	/* Forward definitions. */
	extern void spawn_slaves(void);
	extern void join_slaves(void);
	extern void open_noc_connectors(void);
	extern void close_noc_connectors(void);
	extern void data_send(int, const void *, size_t);
	extern void data_receive(int, int, void *, size_t);

	/* Forward definitions. */
	extern int nclusters;

	/* Forward definitions. */
	extern int infd;
	extern int outfd[NR_CCLUSTER];

	/*===============================================================*
	 * Utility                                                       *
	 *===============================================================*/

	/* Forward definitions. */
	extern void error(const char *);
	extern void *scalloc(size_t, size_t);
	extern void *smalloc(size_t);
	extern void srandnum(int);
	extern unsigned randnum(void);

	/*===============================================================*
	 * Vector                                                        *
	 *===============================================================*/
	
	/*
	 * Opaque pointer to a vector.
	 */
	typedef struct vector * vector_t;
	
	/*
	 * Opaque pointer to a constant vector.
	 */
	typedef const struct vector * const_vector_t;

	/* Forward definitions. */
	extern vector_t vector_create(int);
	extern void vector_destroy(vector_t);
	extern vector_t vector_random(vector_t);
	extern int vector_size(vector_t);
	extern float *vector_get(vector_t);

	/*===============================================================*
	 * Kernel                                                        *
	 *===============================================================*/

	/**
	 * @brief Defaultkernel parameters.
	 */
	#define KM_SEED 0 /**< Default seed value. */

	/* Forward definitions. */
	extern int *kmeans(vector_t *, int, int, float);

#endif /* _MASTER_H_ */
