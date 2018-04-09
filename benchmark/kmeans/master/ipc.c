/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <stdio.h>
#include "master.h"

/**
 * @brief Processes IDs.
 */
static mppa_pid_t pids[NR_CCLUSTER];

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
	for (int i = 0; i < nclusters; i++)
		mppa_waitpid(pids[i], NULL, 0);
}
