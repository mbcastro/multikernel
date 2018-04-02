/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * master.h -  Private master library.
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
	extern void sync_slaves(void);
	extern void open_noc_connectors(void);
	extern void close_noc_connectors(void);
	extern void data_send(int, void *, size_t);
	extern void data_receive(int, void *, size_t);

	/* Forward definitions. */
	extern int infd[NR_CCLUSTER];
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

	/* Forward definitions. */
	extern int *kmeans(vector_t *, int, int, float);

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
