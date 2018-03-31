/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <mppaipc.h>
#include "../global.h"
#include "../util.h"

/* Interprocess communication. */
int infd[NR_CCLUSTER];               /* Input channels.  */
int outfd[NR_CCLUSTER];              /* Output channels. */
static mppa_pid_t pids[NR_CCLUSTER]; /* Processes IDs.   */

/*
 * Spwans slave processes.
 */
void spawn_slaves(void)
{
	int i;          /* Loop index. */
	char arg0[4];   /* Argument 0. */
	char *args[2];  /* Arguments.  */

	/* Spawn slaves. */
	args[1] = NULL;
	for (i = 0; i < nclusters; i++)
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
	int i;
	
	/* Join slaves. */
	for (i = 0; i < nclusters; i++)
	{
		data_receive(infd[i], &slave[i], sizeof(uint64_t));
		mppa_waitpid(pids[i], NULL, 0);
	}
}

/*
 * Open NoC connectors.
 */
void open_noc_connectors(void)
{
	int i;          /* Loop index.     */
	char path[35];  /* Connector path. */

	/* Open channels. */
	for (i = 0; i < nclusters; i++)
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
	int i;
	
	/* Close channels. */
	for (i = 0; i < nclusters; i++)
	{
		mppa_close(outfd[i]);
		mppa_close(infd[i]);
	}
}

