/*
 *  * Copyright (C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *   */

#ifndef SLAVE_H_
#define SLAVE_H_

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

#endif /* SLAVE_H_ */
