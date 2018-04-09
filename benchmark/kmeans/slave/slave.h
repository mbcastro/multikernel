/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * slave.h -  Private slave library.
 */

#ifndef _SLAVE_H_
#define _SLAVE_H_

	#include <stddef.h>

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

	/*===============================================================*
	 * Vector                                                        *
	 *===============================================================*/
	
	/* Forward definitions. */
	extern float vector_distance(float *, float *);
	extern float *vector_add(float *, const float *);
	extern float *vector_mult(float *, float);
	extern float *vector_assign(float *, const float *);
	extern int vector_equal(const float *, const float *);

	/* Forward definitions. */
	extern int dimension;

	/* Forward definitions. */
	extern long total;
	
#endif /* _SLAVE_H_ */
