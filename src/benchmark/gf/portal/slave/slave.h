/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * slave.h -  Private slave library.
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
