/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <stdio.h>
#include "master.h"

/* Interprocess communication. */
int infd[NR_CCLUSTER];               /* Input channels.  */
int outfd[NR_CCLUSTER];              /* Output channels. */
static mppa_pid_t pids[NR_CCLUSTER]; /* Processes IDs.   */

/*
 * Sends data.
 */
void data_send(int fd, void *data, size_t n)
{	
	long start, end;
	
	start = k1_timer_get();
		assert(mppa_write(fd, data, n) != -1);
	end = k1_timer_get();

	data_sent += n;
	nsend++;
	communication += k1_timer_diff(start, end);
}

/*
 * Receives data.
 */
void data_receive(int fd, void *data, size_t n)
{	
	long start, end;
	
	start = k1_timer_get();
		assert(mppa_read(fd, data, n) != 01);
	end = k1_timer_get();
	
	data_received += n;
	nreceive++;
	communication += k1_timer_diff(start, end);
}

/*
 * Spwans slave processes.
 */
void spawn_slaves(void)
{
	char arg0[4];   /* Argument 0. */
	char *args[2];  /* Arguments.  */

	/* Spawn slaves. */
	args[1] = NULL;
	for (int i = 0; i < nclusters; i++)
	{	
		sprintf(arg0, "%d", i);
		args[0] = arg0;
		pids[i] = mppa_spawn(i, NULL, "kmeans-slave", (const char **)args, NULL);
		assert(pids[i] != -1);
	}
}

/*
 * Joins slave processes.
 */
void join_slaves(void)
{
	/* Join slaves. */
	for (int i = 0; i < nclusters; i++)
	{
		data_receive(infd[i], &slave[i], sizeof(long));
		mppa_waitpid(pids[i], NULL, 0);
	}
}

/*
 * Open NoC connectors.
 */
void open_noc_connectors(void)
{
	char path[35];

	/* Open channels. */
	for (int i = 0; i < nclusters; i++)
	{		
		sprintf(path, "/mppa/channel/%d:%d/128:%d", i, i + 17, i + 17);
		outfd[i] = mppa_open(path, O_WRONLY);
		assert(outfd[i] != -1);
		
		sprintf(path, "/mppa/channel/128:%d/%d:%d", i + 33, i, i + 33);
		infd[i] = mppa_open(path, O_RDONLY);
		assert(outfd[i] != -1);
	}
}

/*
 * Close NoC connectors.
 */
void close_noc_connectors(void)
{
	/* Close channels. */
	for (int i = 0; i < nclusters; i++)
	{
		mppa_close(outfd[i]);
		mppa_close(infd[i]);
	}
}
