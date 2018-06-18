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

#ifndef _SLAVE_H_
#define _SLAVE_H_
		
	#include "../../kernel.h"
	
	/*===============================================================*
	 * IPC                                                           *
	 *===============================================================*/

	/* Forward definitions. */
	extern void open_noc_connectors(void);
	extern void close_noc_connectors(void);
	extern void data_receive(int, void *, size_t);
	extern void data_send(int, void *, size_t);

	/* Forward definitions. */
	extern int rank;
	extern int infd;
	extern int outfd;

	/* Forward definitions. */
	extern long total;

	/*===============================================================*
	 * Utility                                                       *
	 *===============================================================*/

	/* Forward definitions. */
	extern void error(const char *);
	extern void *scalloc(size_t, size_t);
	extern void *smalloc(size_t);
	extern void srandnum(int);
	extern unsigned randnum(void);

#endif /* _SLAVE_H_ */
