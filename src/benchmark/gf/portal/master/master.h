/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * master.h -  Private master library.
 */

#ifndef _MASTER_H_
#define _MASTER_H_

    #include <nanvix/arch/mppa.h>
	
	#define PI 3.14159265359 /* pi */
	#define E 2.71828182845904 /* e */
	#define SD 0.8 /* Standard deviation. */
	
	#define CHUNK_SIZE (1024)   /* Maximum chunk size. */
	#define MASK_SIZE  (15)     /* Maximum mask size. */
	#define IMG_SIZE   (32768)  /* Maximum image size. */
	
	/* Type of messages. */
	#define MSG_CHUNK 1
	#define MSG_DIE   0
	
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
	extern void data_receive(int, int, void *, size_t);

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
	 * Kernel                                                        *
	 *===============================================================*/

	/* Forward definitions. */
    extern void gauss_filter(unsigned char *, int, double *, int);

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
