/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of CAP Benchmarks.
 * 
 * CAP Benchmarks is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any
 * later version.
 * 
 * CAP Benchmarks is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * CAP Benchmarks. If not, see <http://www.gnu.org/licenses/>.
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
