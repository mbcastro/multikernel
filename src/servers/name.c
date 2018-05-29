/*
 * Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <mppa/osconfig.h>
#include <nanvix/hal.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>
#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#ifdef DEBUG
#include <stdio.h>
#endif

/**
 * @brief Number of process.
 */
static int nprocess = 0;

/**
 * @brief Lookup table of cluster names.
 */
static struct {
	int id;     				/**< Cluster ID.  */
	int dma;    				/**< DMA channel. */
	char *name; 				/**< Portal name. */
	char *process_name;	/**< Process name. */
} names[NR_DMA] = {
	{ CCLUSTER0,  CCLUSTER0,      "/cpu0",	""  },
	{ CCLUSTER1,  CCLUSTER1,      "/cpu1",	""  },
	{ CCLUSTER2,  CCLUSTER2,      "/cpu2",	""  },
	{ CCLUSTER3,  CCLUSTER3,      "/cpu3",	""  },
	{ CCLUSTER4,  CCLUSTER4,      "/cpu4",	""  },
	{ CCLUSTER5,  CCLUSTER5,      "/cpu5",	""  },
	{ CCLUSTER6,  CCLUSTER6,      "/cpu6",	""  },
	{ CCLUSTER7,  CCLUSTER7,      "/cpu7",	""  },
	{ CCLUSTER8,  CCLUSTER8,      "/cpu8",	""  },
	{ CCLUSTER9,  CCLUSTER9,      "/cpu9",	""  },
	{ CCLUSTER10, CCLUSTER10,     "/cpu10",	""  },
	{ CCLUSTER11, CCLUSTER11,     "/cpu11",	""  },
	{ CCLUSTER12, CCLUSTER12,     "/cpu12",	""  },
	{ CCLUSTER13, CCLUSTER13,     "/cpu13",	""  },
	{ CCLUSTER14, CCLUSTER14,     "/cpu14",	""  },
	{ CCLUSTER15, CCLUSTER15,     "/cpu15",	""  },
	{ IOCLUSTER0, IOCLUSTER0 + 0, "/io0"  , ""  },
	{ IOCLUSTER0, IOCLUSTER0 + 1, "/io1"  ,	""  },
	{ IOCLUSTER0, IOCLUSTER0 + 2, "/io2"  ,	""  },
	{ IOCLUSTER0, IOCLUSTER0 + 3, "/io3"  ,	""  },
	{ IOCLUSTER1, IOCLUSTER1 + 0, "/rmem0",	""  },
	{ IOCLUSTER1, IOCLUSTER1 + 1, "/rmem1",	""  },
	{ IOCLUSTER1, IOCLUSTER1 + 2, "/rmem2",	""  },
	{ IOCLUSTER1, IOCLUSTER1 + 3, "/rmem3",	""  }
};

/*=======================================================================*
 * name_cluster_id()                                                     *
 *=======================================================================*/

/**
 * @brief Converts a pathnamel name into a cluster ID.
 *
 * @param name Target pathnamel name.
 *
 * @returns Upon successful completion the cluster ID whose name is @p
 * name is returned. Upon failure, a negative error code is returned
 * instead.
 */
int name_cluster_id(const char *name)
{
	/* Search for portal name. */
	for (int i = 0; i < NR_DMA; i++)
	{
		/* Found. */
		if (!strcmp(name, names[i].name))
			return (names[i].id);
	}

	return (-ENOENT);
}

/*=======================================================================*
 * name_cluster_dma()                                                    *
 *=======================================================================*/

/**
 * @brief Converts a pathnamel name into a DMA channel id.
 *
 * @param name Target pathnamel name.
 *
 * @returns Upon successful completion the DMA ID whose name is @p
 * name is returned. Upon failure, a negative error code is returned
 * instead.
 */
int name_cluster_dma(const char *name)
{
	/* Search for portal name. */
	for (int i = 0; i < NR_DMA; i++)
	{
		/* Found. */
		if (!strcmp(name, names[i].name))
			return (names[i].dma);
	}

	return (-ENOENT);
}

/*=======================================================================*
 * name_lookdown()                                                       *
 *=======================================================================*/

/**
 * @brief Converts a cluster ID into a pathname.
 *
 * @param name Target pathnamel name.
 *
 * @returns Upon successful completion the pathname that matches the cluster ID
 * @p clusterid is returned. Upon failure, NULL is returned instead.
 */
const char *name_cluster_name(int clusterid)
{
	/* Search for portal name. */
	for (int i = 0; i < NR_DMA; i++)
	{
		/* Found. */
		if (names[i].id == clusterid)
			return (names[i].name);
	}

	return (NULL);
}

/*=======================================================================*
 * name_process_name()                                                      *
 *=======================================================================*/

/**
 * @brief Converts a cluster ID into a process name.
 *
 * @param name Target process name.
 *
 * @returns Upon successful completion the process name that matches the cluster ID.
 * Upon failure, NULL is returned instead.
 */
const char *name_process_name(int clusterid)
{
	/* Search for process name. */
	for (int i = 0; i < NR_DMA; i++)
	{
		/* Found. */
		if (names[i].id == clusterid)
			return (names[i].process_name);
	}

	return (NULL);
}

/*=======================================================================*
 * register_name()                                                        *
 *=======================================================================*/

/**
 * @brief Register a process name
 *
 * @param id		Cluster ID.
 * @param DMA		DMA channel.
 * @param name	Portal name.
 * @param process_name	Process name.
 */
int register_name(int id, int dma, const char *name, const char *process_name)
{
	if(nprocess >= NR_DMA)
		return -1;
	names[nprocess].id = id;
	names[nprocess].dma = dma;
	sprintf(names[nprocess].name, "%s", name);
	sprintf(names[nprocess].process_name, "%s", process_name);
	return ++nprocess;
}

/*=======================================================================*
 * remove_name()                                                        *
 *=======================================================================*/

/**
 * @brief Remove a process name
 *
 * @param name	Portal name.
 */
void remove_name(const char *name)
{
	/* Search for portal name. */
	int i = 0;

	while(i < NR_DMA && strcmp(name, names[i].name))
	{
		i++;
	}

	if(i < NR_DMA)
	{
		for(int j = i; j < nprocess - 1; j++)
		{
			names[j].id = names[j+1].id;
			names[j].dma = names[j+1].dma;
			sprintf(names[j].name, "%s", names[j+1].name);
			sprintf(names[j].process_name, "%s", names[j+1].process_name);
		}
		nprocess--;
	}

	return;
}

static pthread_barrier_t barrier;

static pthread_mutex_t lock;

/*===================================================================*
 * name_server()                                                     *
 *===================================================================*/

/**
 * @brief Handles remote name requests.
 *
 * @param args Server arguments.
 *
 * @returns Always returns NULL.
 */
static void *name_server(void *args)
{
	int dma;           /* DMA channel to use.         */
	int inbox;         /* Mailbox for small messages. */
	char pathname[16]; /* name bank.                  */

	dma = ((int *)args)[0];

	sprintf(pathname, "/io%d", dma);
	pthread_mutex_lock(&lock);
		inbox = mailbox_create(pathname);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	while(1)
	{
		struct name_message msg;

		mailbox_read(inbox, &msg);

		/* handle name query. */
		switch (msg.op)
		{
			/* Query name. */
			case NAME_QUERY:
				msg.id = name_cluster_id(msg.name);
				msg.dma = name_cluster_dma(msg.name);
				sprintf(msg.process_name, "%s", name_process_name(msg.id));
				mailbox_write(msg.source, &msg);
				break;

			/* Add name. */
			case NAME_ADD:
				assert(register_name(msg.id, msg.dma, msg.name, msg.process_name) != -1);
				break;

      /* Remove name. */
			case NAME_REMOVE:
				remove_name(msg.name);
				break;

			/* Should not happen. */
			default:
				break;
		}
	}

	/* House keeping. */
	pthread_mutex_lock(&lock);
		mailbox_unlink(inbox);
	pthread_mutex_unlock(&lock);

	return (NULL);
}

/*===================================================================*
 * main()                                                            *
 *===================================================================*/

/**
 * @brief Remote name server.
 */
int main(int argc, char **argv)
{
	int global_barrier;               /* System barrier. */
	pthread_t tid; 										/* Thread ID .     */

	((void) argc);
	((void) argv);

#ifdef DEBUG
	printf("[NAME_RESOLUTION] booting up server\n");
#endif

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, 2);

	/* Spawn name server thread. */
	int dma = 0;
	assert((pthread_create(&tid,
		NULL,
		name_server,
		&dma)) == 0
	);

	pthread_barrier_wait(&barrier);

	/* Release master IO cluster. */
	global_barrier = barrier_open(1);
	barrier_wait(global_barrier);

#ifdef DEBUG
	printf("[NAME_RESOLUTION] server alive\n");
#endif

	/* Wait for name server threads. */
	pthread_join(tid, NULL);

	/* House keeping. */
	barrier_close(global_barrier);

	return (EXIT_SUCCESS);
}
