/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <global.h> - Global variables.
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

	#include <nanvix/arch/mppa.h>
	#include <stddef.h>
	#include <stdint.h>

	/*
	 * Be verbose?
	 */
	extern int verbose;

	/*
	 * Number of threads.
	 */
	extern int nclusters;
	
	/* Timing statistics. */
	extern uint64_t master;
	extern uint64_t slave[NR_CCLUSTER];
	extern uint64_t communication;
	extern uint64_t total;
	extern size_t data_sent;
	extern size_t data_received;
	extern unsigned nsend;
	extern unsigned nreceive;

#endif /* GLOBAL_H_ */
