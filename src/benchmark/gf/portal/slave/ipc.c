/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <stdio.h>
#include "slave.h"

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

	sprintf(path, "/cpu%d", rank);
	infd = portal_create(path);

	outfd = portal_open("/io0");
}

/*
 * Closes NoC connectors.
 */
void close_noc_connectors(void)
{
	portal_unlink(infd);
	portal_close(outfd);
}

/*
 * Sends data.
 */
void data_send(int fd, void *data, size_t n)
{	
	long start, end;
	
	start = k1_timer_get();
		portal_write(fd, data, n);
	end = k1_timer_get();
	
	total += k1_timer_diff(start, end);
}

/*
 * Receives data.
 */
void data_receive(int fd, void *data, size_t n)
{	
	long start, end;
	
	start = k1_timer_get();
		portal_allow(fd, IOCLUSTER0);
		portal_read(fd, data, n);
//	k1_dcache_invalidate_mem_area(data, n);
	end = k1_timer_get();
	
	total += k1_timer_diff(start, end);
}
