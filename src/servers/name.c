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
#include <nanvix/klib.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#ifdef DEBUG
#include <stdio.h>
#endif

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
	int id;     						/**< Cluster ID.  */
	int dma;    						/**< DMA channel. */
	char name[PROC_NAME_MAX];			/**< Portal name. */
} names[NR_DMA] = {
	{ CCLUSTER0,  CCLUSTER0,      "\0"  },
	{ CCLUSTER1,  CCLUSTER1,      "\0"  },
	{ CCLUSTER2,  CCLUSTER2,      "\0"  },
	{ CCLUSTER3,  CCLUSTER3,      "\0"  },
	{ CCLUSTER4,  CCLUSTER4,      "\0"  },
	{ CCLUSTER5,  CCLUSTER5,      "\0"  },
	{ CCLUSTER6,  CCLUSTER6,      "\0"  },
	{ CCLUSTER7,  CCLUSTER7,      "\0"  },
	{ CCLUSTER8,  CCLUSTER8,      "\0"  },
	{ CCLUSTER9,  CCLUSTER9,      "\0"  },
	{ CCLUSTER10, CCLUSTER10,     "\0"  },
	{ CCLUSTER11, CCLUSTER11,     "\0"  },
	{ CCLUSTER12, CCLUSTER12,     "\0"  },
	{ CCLUSTER13, CCLUSTER13,     "\0"  },
	{ CCLUSTER14, CCLUSTER14,     "\0"  },
	{ CCLUSTER15, CCLUSTER15,     "\0"  },
	{ IOCLUSTER0, IOCLUSTER0 + 0, "/io0"},
	{ IOCLUSTER0, IOCLUSTER0 + 1, "\0"  },
	{ IOCLUSTER0, IOCLUSTER0 + 2, "\0"  },
	{ IOCLUSTER0, IOCLUSTER0 + 3, "\0"  },
	{ IOCLUSTER1, IOCLUSTER1 + 0, "\0"  },
	{ IOCLUSTER1, IOCLUSTER1 + 1, "\0"  },
	{ IOCLUSTER1, IOCLUSTER1 + 2, "\0"  },
	{ IOCLUSTER1, IOCLUSTER1 + 3, "\0"  }
};

/*=======================================================================*
 * _name_lookup_id()                                                     *
 *=======================================================================*/

/**
 * @brief Resolves a process name into a cluster ID.
 *
 * @param name Target process name.
 *
 * @returns Upon successful completion, the cluster ID whose name is
 * @p name is returned. Upon failure, a negative error code is
 * returned instead.
 */
static int _name_lookup_id(const char *name)
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
 * _name_lookup_dma()                                                    *
 *=======================================================================*/

/**
 * @brief Converts a pathname name into a DMA channel id.
 *
 * @param name Target pathname name.
 *
 * @returns Upon successful completion the DMA ID whose name is @p
 * name is returned. Upon failure, a negative error code is returned
 * instead.
 */
static int _name_lookup_dma(const char *name)
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
 * _name_lookup_pathname()                                               *
 *=======================================================================*/

/**
 * @brief Converts a cluster ID into a pathname.
 *
 * @param name Target pathname name.
 *
 * @returns Upon successful completion the pathname that matches the cluster ID
 * @p clusterid is returned. Upon failure, NULL is returned instead.
 */
static char *_name_lookup_pathname(int clusterid)
{
	/* Search for portal name. */
	for (int i = 0; i < NR_DMA; i++)
	{
		/* Found. */
		if (names[i].dma == clusterid)
			return (names[i].name);
	}

	return ("\0");
}

/*=======================================================================*
 * _name_link()                                                          *
 *=======================================================================*/

/**
 * @brief Register a process name
 *
 * @param DMA			DMA channel.
 * @param name			Portal name.
 *
 * @returns Upon successful registration the number of name is returned.
 * Upon failure, a negative error code is returned instead.
 */
static int _name_link(int dma, char *name)
{
	int index = -1;

#ifdef DEBUG
	printf("Entering NAME_ADD case... [dma: %d, name: %s].\n",
	                                               dma, name);
#endif

	/* No DMA available. */
	if (nr_cluster >= NR_DMA)
		return (-EINVAL);

	/* Compute index registration */
	if (dma >= 0 && dma < NR_CCLUSTER)
		index = dma;
	else if (dma >= IOCLUSTER0 && dma <= IOCLUSTER0 + 3)
	 	index = NR_CCLUSTER + dma%IOCLUSTER0;
	else if (dma >= IOCLUSTER1 && dma <= IOCLUSTER1 + 3)
	 	index = NR_CCLUSTER + NR_IOCLUSTER_DMA + dma%IOCLUSTER1;
	else
		return (-EINVAL);

	/* DMA channel not available */
	if (strcmp(names[index].name, "\0"))
		return (-3);

#ifdef DEBUG
	printf("writing [name: %s] at index %d.\n", name, index);
#endif

	snprintf(names[index].name, ARRAY_LENGTH(names[index].name), "%s", name);

	return (++nr_cluster);
}

/*=======================================================================*
 *_name_unlink()                                                         *
 *=======================================================================*/

/**
 * @brief Remove a name
 *
 * @param name	Portal name.
 */
static void _name_unlink(char *name)
{
	/* Search for portal name. */
	int i = 0;

#ifdef DEBUG
	printf("Entering NAME_REMOVE case... name: %s.\n", msg);
#endif

	while (i < NR_DMA && strcmp(name, names[i].name))
	{
		i++;
	}

	if (i < NR_DMA)
	{
		strcpy(names[i].name, "\0");
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
	int dma;   /* DMA channel to use.         */
	int inbox; /* Mailbox for small messages. */

	dma = ((int *)args)[0];

	pthread_mutex_lock(&lock);
		inbox = _mailbox_create(IOCLUSTER0 + dma, NAME);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	while(1)
	{
		struct name_message msg;

		assert(mailbox_read(inbox, &msg) == 0);

		/* handle name query. */
		switch (msg.op)
		{
			/* Lookup. */
			case NAME_QUERY:
				if (msg.id == -1)
				{
					/* ID query. */
					#ifdef DEBUG
						printf("Entering NAME_QUERY case... name provided:%s.\n"
						                                            , msg.name);
					#endif
					msg.id = _name_lookup_id(msg.name);
				}
				else
				{
					/* Name query. */
					#ifdef DEBUG
						printf("Entering NAME_QUERY case... id provided:%d.\n"
						                                            , msg.id);
					#endif
					snprintf(msg.name, ARRAY_LENGTH(msg.name), "%s",
					                 _name_lookup_pathname(msg.id));
				}
				msg.dma = _name_lookup_dma(msg.name);

				/* Send response. */
				int source = _mailbox_open(msg.source, NAME);
				assert(source >= 0);
				assert(mailbox_write(source, &msg) == 0);
				assert(mailbox_close(source) == 0);
				break;

			/* Add name. */
			case NAME_ADD:
				assert(_name_link(msg.dma, msg.name) > 0);
				break;

			/* Remove name. */
			case NAME_REMOVE:
				_name_unlink(msg.name);
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

	return ("\0");
}

/*===================================================================*
 * main()                                                            *
 *===================================================================*/

/**
 * @brief DMA channel.
 */
#define NAME_DMA 0

/**
 * @brief Resolves process names.
 */
int main(int argc, char **argv)
{
	int dma;            /* DMA_channel to use. */
	int global_barrier; /* System barrier.     */
	pthread_t tid;      /* Thread ID.          */

	((void) argc);
	((void) argv);

#ifdef DEBUG
	printf("[NAME] booting up server\n");
#endif

	/* Spawn name server thread. */
	dma = NAME_DMA;
	assert((pthread_create(&tid,
		NULL,
		name_server,
		&dma)) == 0
	);

	/* Release master IO cluster. */
	global_barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(global_barrier);

#ifdef DEBUG
	printf("[NAME] server alive\n");
#endif

	/* Wait for name server thread. */
	pthread_join(tid, NULL);

	/* House keeping. */
	barrier_close(global_barrier);

	return (EXIT_SUCCESS);
}
