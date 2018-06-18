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
#include "master.h"

/* Interprocess communication. */
int infd;                            /* Input channels.  */
int outfd[NR_CCLUSTER];              /* Output channels. */
static mppa_pid_t pids[NR_CCLUSTER]; /* Processes IDs.   */

/*
 * @brief Sends data.
 *
 * @param fd   Output NoC connector.
 * @param data Target buffer.
 * @param n    Number of bytes to send.
 */
void data_send(int fd, const void *data, size_t n)
{
	portal_write(fd, data, n);
}

/*
 * @brief Receives data.
 *
 * @param fd     Input NoC connector.
 * @param remote Remote cluster.
 * @param data   Target buffer.
 * @param n      Number of bytes to receive.
 */
void data_receive(int fd, int remote, void *data, size_t n)
{	
	portal_allow(fd, remote);
	portal_read(fd, data, n);
}

/**
 * @brief Spwans slave processes.
 */
void spawn_slaves(void)
{
	char arg0[4];  /* Argument 0. */
	char *args[2]; /* Arguments.  */

	/* Spawn slaves. */
	args[1] = NULL;
	for (int i = 0; i < nclusters; i++)
	{	
		sprintf(arg0, "%d", i);
		args[0] = arg0;
		pids[i] = mppa_spawn(i,
					NULL,
					KM_SLAVE_BINARY,
					(const char **)args,
					NULL
				);
		assert(pids[i] != -1);
	}
}

/*
 * @brief Wait slave processes to complete.
 */
void join_slaves(void)
{
	for (int i = 0; i < nclusters; i++)
		mppa_waitpid(pids[i], NULL, 0);
}

/**
 * @brief Open NoC connectors.
 */
void open_noc_connectors(void)
{
	char path[35];

	infd = portal_create("/io0");

	/* Open channels. */
	for (int i = 0; i < nclusters; i++)
	{		
		sprintf(path,"/cpu%d", i);
		outfd[i] = portal_open(path);
	}
}

/**
 * @brief Close NoC connectors.
 */
void close_noc_connectors(void)
{
	/* Close channels. */
	for (int i = 0; i < nclusters; i++)
		portal_close(outfd[i]);

	portal_unlink(infd);
}
