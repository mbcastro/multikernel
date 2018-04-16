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
