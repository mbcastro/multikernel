/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * slave.h -  Private slave library.
 */

#ifndef _SLAVE_H_
#define _SLAVE_H_

	#include <stdlib.h>

	#define PI 3.14159265359   /* pi                  */
    #define E 2.71828182845904 /* e                   */
    #define SD 0.8             /* Standard Deviation  */

	#define CHUNK_SIZE (1024)  /* Maximum chunk size. */
	#define MASK_SIZE (20)     /* Maximum mask size.  */
	#define IMG_SIZE (32768)   /* Maximum image size. */
	
    /* Type of messages. */
	#define MSG_CHUNK 1
	#define MSG_DIE   0


	
    /*===============================================================*
     * IPC                                                           *
     *===============================================================*/

    /* Forward definitions. */
    extern void open_noc_connectors(void);
    extern void close_noc_connectors(void);
    extern void data_receive(int, void *, size_t);
    extern void data_send(int, void *, size_t);
    extern void sync_master(void);

    /* Forward definitions. */
    extern int rank;
    extern int infd;
    extern int outfd;
	
#endif /* _SLAVE_H_ */
