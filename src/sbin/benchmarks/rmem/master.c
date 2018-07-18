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

#include <mppa/osconfig.h>
#include <nanvix/syscalls.h>
#include <nanvix/mm.h>
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

static pthread_mutex_t lock;

/**
 * @brief Number of registration.
 */
static int nr_registration = 0;

/**
 * @brief Lookup table of process names.
 */
static struct {
	int core;                        /**< CPU ID.      */
	char name[NANVIX_PROC_NAME_MAX]; /**< Portal name. */
} names[NANVIX_NR_NODES] = {
	{ CCLUSTER0,      "\0"  },
	{ CCLUSTER1,      "\0"  },
	{ CCLUSTER2,      "\0"  },
	{ CCLUSTER3,      "\0"  },
	{ CCLUSTER4,      "\0"  },
	{ CCLUSTER5,      "\0"  },
	{ CCLUSTER6,      "\0"  },
	{ CCLUSTER7,      "\0"  },
	{ CCLUSTER8,      "\0"  },
	{ CCLUSTER9,      "\0"  },
	{ CCLUSTER10,     "\0"  },
	{ CCLUSTER11,     "\0"  },
	{ CCLUSTER12,     "\0"  },
	{ CCLUSTER13,     "\0"  },
	{ CCLUSTER14,     "\0"  },
	{ CCLUSTER15,     "\0"  },
	{ IOCLUSTER0 + 0, "/io0"},
	{ IOCLUSTER0 + 1, "\0"  },
	{ IOCLUSTER0 + 2, "\0"  },
	{ IOCLUSTER0 + 3, "\0"  },
	{ IOCLUSTER1 + 0, "\0"  },
	{ IOCLUSTER1 + 1, "\0"  },
	{ IOCLUSTER1 + 2, "\0"  },
	{ IOCLUSTER1 + 3, "\0"  }
};

/*=======================================================================*
 * _name_lookup()                                                    *
 *=======================================================================*/

/**
 * @brief Converts a name into a CPU ID.
 *
 * @param name 		Target name.
 *
 * @returns Upon successful completion the CPU ID whose name is @p
 * name is returned. Upon failure, a negative error code is returned
 * instead.
 */
static int _name_lookup(const char *name)
{
	/* Search for portal name. */
	for (int i = 0; i < NANVIX_NR_NODES; i++)
	{
		/* Found. */
		if (!strcmp(name, names[i].name))
			return (names[i].core);
	}

	return (-ENOENT);
}

/*=======================================================================*
 * _name_link()                                                          *
 *=======================================================================*/

/**
 * @brief Register a process name.
 *
 * @param core			CPU ID of the process to register.
 * @param name			Name of the process to register.
 *
 * @returns Upon successful registration the number of name registered
 * is returned. Upon failure, a negative error code is returned instead.
 */
static int _name_link(int core, char *name)
{
	int index;          /* Index where the process will be stored. */

	/* No entry available. */
	if (nr_registration >= NANVIX_NR_NODES)
		return (-EINVAL);

	/* Compute index registration */
	if (core >= 0 && core < NR_CCLUSTER)
		index = core;
	else if (core >= IOCLUSTER0 && core <= IOCLUSTER0 + 3)
	 	index = NR_CCLUSTER + core%IOCLUSTER0;
	else if (core >= IOCLUSTER1 && core <= IOCLUSTER1 + 3)
	 	index = NR_CCLUSTER + NR_IOCLUSTER_DMA + core%IOCLUSTER1;
	else
		return (-EINVAL);

	/* Entry not available */
	if (strcmp(names[index].name, "\0"))
		return (-EINVAL);

#ifdef DEBUG
	printf("writing [CPU ID:%d name: %s] at index %d.\n", names[index].core,
	                                                            name, index);
#endif

	strcpy(names[index].name, name);

	return (++nr_registration);
}

/*=======================================================================*
 *_name_unlink()                                                         *
 *=======================================================================*/

/**
 * @brief Remove a name
 *
 * @param name			Name of the process to unlink.
 *
 * @returns Upon successful registration the new number of name registered
 * is returned. Upon failure, a negative error code is returned instead.
 */
static int _name_unlink(char *name)
{
	/* Search for portal name. */
	int i = 0;

	while (i < NANVIX_NR_NODES && strcmp(name, names[i].name))
	{
		i++;
	}

	if (i < NANVIX_NR_NODES)
	{
		strcpy(names[i].name, "\0");
		return (--nr_registration);
	}

	return (-ENOENT);
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
	int dma;   /* DMA channel to use.         */
	int inbox; /* Mailbox for small messages. */

	dma = ((int *)args)[0];

	/* Open server mailbox. */
	pthread_mutex_lock(&lock);
		inbox = sys_mailbox_create(IOCLUSTER0 + dma);
	pthread_mutex_unlock(&lock);

	while(1)
	{
		struct name_message msg;

		assert(mailbox_read(inbox, &msg) == 0);

		/* Handle name requests. */
		switch (msg.op)
		{
			/* Lookup. */
			case NAME_LOOKUP:
#ifdef DEBUG
				printf("Entering NAME_LOOKUP case... name provided:%s.\n"
						                                     , msg.name);
#endif
				msg.core = _name_lookup(msg.name);

				/* Send response. */
				int source =sys_mailbox_open(msg.source);
				assert(source >= 0);
				assert(mailbox_write(source, &msg) == 0);
				assert(mailbox_close(source) == 0);
				break;

			/* Add name. */
			case NAME_ADD:
#ifdef DEBUG
				printf("Entering NAME_ADD case... [CPU ID: %d, name: %s].\n",
				                                          msg.core, msg.name);
#endif
				assert(_name_link(msg.core, msg.name) > 0);
				break;

			/* Remove name. */
			case NAME_REMOVE:
#ifdef DEBUG
				printf("Entering NAME_REMOVE case... name: %s.\n", msg.name);
#endif
				assert(_name_unlink(msg.name) >= 0);
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

/**
 * @brief Wait for slaves to complete.
 *
 * @param nclusters Number of slaves to wait.
 */
/*static void join_slaves(int nclusters)
{
	for (int i = 0; i < nclusters; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);
}*/

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

	/* Deploy name server. */
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
	barrier_close(global_barrier);

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

/* join_slaves(nclusters); */

	/* Wait for slaves. */
	global_barrier = barrier_open(nclusters);
	barrier_wait(global_barrier);

	printf("master crossed the barrier\n");

	/* House keeping. */
	barrier_close(global_barrier);

	return (EXIT_SUCCESS);
}
