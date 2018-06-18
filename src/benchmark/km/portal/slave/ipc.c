/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
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
	portal_write(fd, data, n);
}

/*
 * Receives data.
 */
void data_receive(int fd, void *data, size_t n)
{	
	portal_allow(fd, IOCLUSTER0);
	portal_read(fd, data, n);
}
