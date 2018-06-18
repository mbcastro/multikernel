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
	
	/* Forward definitions. */
	extern void spawn_slaves(void);
	extern void join_slaves(void);

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

	/* Forward definitions. */
	extern float * vector_random(float *, int);

	/*===============================================================*
	 * Kernel                                                        *
	 *===============================================================*/

	/* Forward definitions. */
	extern void kmeans(const float *, int, int, int, float);

	/* Forward definitions. */
	extern long master;
	extern long slave[NR_CCLUSTER];
	extern long communication;
	extern size_t data_sent;
	extern size_t data_received;
	extern unsigned nsend;
	extern unsigned nreceive;
	extern int nclusters;

#endif /* _MASTER_H_ */
