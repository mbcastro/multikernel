/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <mppaipc.h>


/* Inter process communication. */
int rank;  /* Process rank.   */
int infd;  /* Input channel.  */
int outfd; /* Output channel. */

/*
 * Opens NoC connectors.
 */
void open_noc_connectors(void)
{
	char path[35];
	
	/* Open input channel. */
	sprintf(path, "/mppa/channel/%d:%d/128:%d", rank, rank + 17, rank + 17);
	infd = mppa_open(path, O_RDONLY);
	assert(infd != -1);
	sprintf(path, "/mppa/channel/128:%d/%d:%d", rank + 33, rank, rank + 33);
	outfd = mppa_open(path, O_WRONLY);
	assert(outfd != -1);
}

/*
 * Closes NoC connectors.
 */
void close_noc_connectors(void)
{
	/* Close channels. */
	mppa_close(infd);
	mppa_close(outfd);
}

