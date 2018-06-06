/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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
#include <nanvix/arch/mppa.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>
#include <assert.h>
#include <stdlib.h>
#include "kernel.h"

#ifdef DEBUG
#include <stdio.h>
#endif

/**
 * @brief ID of slave processes.
 */
static int pids[NR_CCLUSTER];

/**
 * @brief ID of name server thread.
 */
static pthread_t tid_name_server;

static pthread_barrier_t barrier;

static pthread_mutex_t lock;

/**
 * @brief Number of cluster name registered.
 */
static int nr_cluster = 0;

/**
 * @brief Lookup table of cluster names.
 */
static struct {
	int id;     				/**< Cluster ID.  */
	int dma;    				/**< DMA channel. */
	char name[50]; 				/**< Portal name. */
	char process_name[50];	/**< Process name. */
} names[NR_DMA] = {
	{ CCLUSTER0,  CCLUSTER0,      " ",	" "  },
	{ CCLUSTER1,  CCLUSTER1,      " ",	" "  },
	{ CCLUSTER2,  CCLUSTER2,      " ",	" "  },
	{ CCLUSTER3,  CCLUSTER3,      " ",	" "  },
	{ CCLUSTER4,  CCLUSTER4,      " ",	" "  },
	{ CCLUSTER5,  CCLUSTER5,      " ",	" "  },
	{ CCLUSTER6,  CCLUSTER6,      " ",	" "  },
	{ CCLUSTER7,  CCLUSTER7,      " ",	" "  },
	{ CCLUSTER8,  CCLUSTER8,      " ",	" "  },
	{ CCLUSTER9,  CCLUSTER9,      " ",	" "  },
	{ CCLUSTER10, CCLUSTER10,     " ",	" "  },
	{ CCLUSTER11, CCLUSTER11,     " ",	" "  },
	{ CCLUSTER12, CCLUSTER12,     " ",	" "  },
	{ CCLUSTER13, CCLUSTER13,     " ",	" "  },
	{ CCLUSTER14, CCLUSTER14,     " ",	" "  },
	{ CCLUSTER15, CCLUSTER15,     " ",	" "  },
	{ IOCLUSTER0, IOCLUSTER0 + 0, "/io0",	"name-server"  },
	{ IOCLUSTER0, IOCLUSTER0 + 1, " ",	" "  },
	{ IOCLUSTER0, IOCLUSTER0 + 2, " ",	" "  },
	{ IOCLUSTER0, IOCLUSTER0 + 3, " ",	" "  },
	{ IOCLUSTER1, IOCLUSTER1 + 0, " ",	" "  },
	{ IOCLUSTER1, IOCLUSTER1 + 1, " ",	" "  },
	{ IOCLUSTER1, IOCLUSTER1 + 2, " ",	" "  },
	{ IOCLUSTER1, IOCLUSTER1 + 3, " ",	" "  }
};

/*=======================================================================*
 * server_name_cluster_id()                                                     *
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
int server_name_cluster_id(const char *name)
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
 * server_name_cluster_dma()                                                    *
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
int server_name_cluster_dma(const char *name)
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
const char *server_id_cluster_name(int clusterid)
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
 * server_id_process_name()                                                      *
 *=======================================================================*/

/**
 * @brief Converts a cluster ID into a process name.
 *
 * @param name Target process name.
 *
 * @returns Upon successful completion the process name that matches the cluster ID.
 * Upon failure, NULL is returned instead.
 */
const char *server_id_process_name(int clusterid)
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
 * server_register_name()                                                        *
 *=======================================================================*/

/**
 * @brief Register a process name
 *
 * @param DMA		DMA channel.
 * @param name	Portal name.
 * @param process_name	Process name.
 *
 * @returns Upon successful registration the number of name is returned.
 * Upon failure, a negative error code is returned instead.
 */
int server_register_name(int dma, const char *name, const char *process_name)
{
	int index;

	/* No DMA available. */
	if(nr_cluster >= NR_DMA)
		return (-1);

	/* Compute index registration */
	if(dma >= 0 && dma < NR_CCLUSTER)
		index = dma;
	else if(dma >= IOCLUSTER0 && dma <= IOCLUSTER0 + 3)
	 	index = NR_CCLUSTER + dma%IOCLUSTER0;
	else if(dma >= IOCLUSTER1 && dma <= IOCLUSTER1 + 3)
	 	index = NR_CCLUSTER + NR_IOCLUSTER_DMA + dma%IOCLUSTER1;
	else
		return (-EINVAL);

	/* DMA channel not available */
	if(strcmp(names[index].name, " "))
		return (-3);

	#ifdef DEBUG
		printf("Writing %s, %s at index %d.\n", name, process_name, index);
	#endif

	sprintf(names[index].name, "%s", name);
	sprintf(names[index].process_name, "%s", process_name);

	return ++nr_cluster;
}

/*=======================================================================*
 * server_remove_name()                                                        *
 *=======================================================================*/

/**
 * @brief Remove a process name
 *
 * @param name	Portal name.
 */
void server_remove_name(const char *name)
{
	/* Search for portal name. */
	int i = 0;

	while(i < NR_DMA && strcmp(name, names[i].name))
	{
		i++;
	}

	if(i < NR_DMA)
	{
		sprintf(names[i].name, " ");
		sprintf(names[i].process_name, " ");
		nr_cluster--;
	}

	return;
}

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

	dma = ((int *)args)[0];

	pthread_mutex_lock(&lock);
		inbox = mailbox_create(IOCLUSTER0 + dma, NAME);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	while(1)
	{
		struct name_message msg;

		assert(mailbox_read(inbox, &msg) == STD);

		/* handle name query. */
		switch (msg.op)
		{
			/* Lookup */
			case NAME_QUERY:
				if(msg.id == -1){
					/* ID query */
					#ifdef DEBUG
						printf("Entering NAME_QUERY case... name provided: %s.\n", msg.name);
					#endif
					msg.id = server_name_cluster_id(msg.name);
				}else{
					/* name query */
					#ifdef DEBUG
						printf("Entering NAME_QUERY case... id provided: %d.\n", msg.id);
					#endif
					sprintf(msg.name, "%s", server_id_cluster_name(msg.id));
				}
				msg.dma = server_name_cluster_dma(msg.name);
				sprintf(msg.process_name, "%s", server_id_process_name(msg.id));

				/* Send response */
				int source = mailbox_open(msg.source, NAME);
				assert(source >= 0);
				assert(mailbox_write(source, &msg) == 0);
				assert(mailbox_close(source) == 0);
				break;

			/* Add name. */
			case NAME_ADD:
				#ifdef DEBUG
					printf("Entering NAME_ADD case... dma: %d, name: %s, process name: %s.\n", msg.dma, msg.name, msg.process_name);
				#endif

				assert(server_register_name(msg.dma, msg.name, msg.process_name) > 0);
				break;

			/* Remove name. */
			case NAME_REMOVE:
				#ifdef DEBUG
					printf("Entering NAME_REMOVE case... name: %s.\n", msg.name);
				#endif

				server_remove_name(msg.name);
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

/**
 * @brief Spawns slave processes.
 *
 * @param nclusters Number of clusters to spawn.
 * @param args      Cluster arguments.
 */
static void spawn_slaves(int nclusters, char **args)
{
	const char *argv[] = {
		"rmem-slave",
		args[1],
		args[2],
		args[3],
		NULL
	};

	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);
}

	// /**
	//  * @brief Wait for slaves to complete.
	//  *
	//  * @param nclusters Number of slaves to wait.
	//  */
	// static void join_slaves(int nclusters)
	// {
	// 	for (int i = 0; i < nclusters; i++)
	// 		assert(mppa_waitpid(pids[i], NULL, 0) != -1);
	// }

/*===================================================================*
 * Kernel                                                            *
 *===================================================================*/

/**
 * @brief Benchmarks write operations on the remote memory.
 */
int main(int argc, char **argv)
{
	int size;
	int global_barrier;
	int nclusters;

	assert(argc == 4);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[2]);
	assert((size = atoi(argv[3])) <= RMEM_BLOCK_SIZE);

	/* Deploy name server */

#ifdef DEBUG
	printf("[NAME_RESOLUTION] booting up server\n");
#endif

	/* Spawn name server thread. */
	int dma = 0;
	assert((pthread_create(&tid_name_server,
		NULL,
		name_server,
		&dma)) == 0
	);

#ifdef DEBUG
	printf("[NAME_RESOLUTION] server alive\n");
#endif

	/* Wait RMEM server. */
	global_barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(global_barrier);

#ifdef DEBUG
	printf("[SPAWNER] server alive\n");
#endif

#ifdef DEBUG
	printf("[SPAWNER] spawning kernels\n");
#endif

	spawn_slaves(nclusters, argv);

#ifdef DEBUG
	printf("[SPAWNER] waiting kernels\n");
#endif

	// join_slaves(nclusters);

	/* Wait for name server thread. */
	pthread_join(tid_name_server, NULL);

	/* House keeping. */
	barrier_close(global_barrier);

	return (EXIT_SUCCESS);
}
