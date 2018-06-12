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

static pthread_mutex_t lock;

/**
 * @brief Number of registration.
 */
static int nr_registration = 0;

/**
 * @brief Lookup table of process names.
 */
static struct {
	int nodeid;    						/**< NoC node ID.  */
	char name[PROC_NAME_MAX];			/**< Process name. */
} names[HAL_NR_NOC_NODES] = {
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
 * _name_lookup()                                                        *
 *=======================================================================*/

/**
 * @brief Converts a name into a NoC node ID.
 *
 * @param name 		Target name.
 *
 * @returns Upon successful completion the NoC node ID whose name is @p
 * name is returned. Upon failure, a negative error code is returned
 * instead.
 */
static int _name_lookup(const char *name)
{
	/* Search for portal name. */
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
	{
		/* Found. */
		if (!strcmp(name, names[i].name))
			return (names[i].nodeid);
	}

	return (-ENOENT);
}

/*=======================================================================*
 * _name_link()                                                          *
 *=======================================================================*/

/**
 * @brief Register a process name.
 *
 * @param nodeid		NoC node ID of the process to register.
 * @param name			Name of the process to register.
 *
 * @returns Upon successful registration the number of name registered
 * is returned. Upon failure, a negative error code is returned instead.
 */
static int _name_link(int nodeid, char *name)
{
	int index;          /* Index where the process will be stored. */

	/* No entry available. */
	if (nr_registration >= HAL_NR_NOC_NODES)
		return (-EINVAL);

	/* Check that the name is not already used */
	for (int i = 0; i < HAL_NR_NOC_NODES; i++){
		if (strcmp(names[i].name, name) == 0)
		{
			return (-EINVAL);
		}
	}

	/* Compute index registration */
	if (nodeid >= 0 && nodeid < NR_CCLUSTER)
		index = nodeid;
	else if (nodeid >= IOCLUSTER0 && nodeid <= IOCLUSTER0 + 3)
	 	index = NR_CCLUSTER + nodeid%IOCLUSTER0;
	else if (nodeid >= IOCLUSTER1 && nodeid <= IOCLUSTER1 + 3)
	 	index = NR_CCLUSTER + NR_IOCLUSTER_DMA + nodeid%IOCLUSTER1;
	else
		return (-EINVAL);

	/* Entry not available */
	if (strcmp(names[index].name, "\0"))
		return (-EINVAL);

#ifdef DEBUG
	printf("writing [nodeid ID:%d name: %s] at index %d.\n",
	                   names[index].nodeid, name, index);
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

	while (i < HAL_NR_NOC_NODES && strcmp(name, names[i].name))
	{
		i++;
	}

	if (i < HAL_NR_NOC_NODES)
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
	int dma;     /* DMA channel to use.         */
	int inbox;   /* Mailbox for small messages. */
	int source;  /* NoC node ID of the client   */
	int tmp;

	dma = ((int *)args)[0];

	/* Open server mailbox. */
	pthread_mutex_lock(&lock);
		inbox = hal_mailbox_create(IOCLUSTER0 + dma);
	pthread_mutex_unlock(&lock);

	while(1)
	{
		struct name_message msg;

		assert(hal_mailbox_read(inbox, &msg, HAL_MAILBOX_MSG_SIZE)
		                                  == HAL_MAILBOX_MSG_SIZE);

		/* Handle name requests. */
		switch (msg.op)
		{
			/* Lookup. */
			case NAME_LOOKUP:
#ifdef DEBUG
				printf("Entering NAME_LOOKUP case... name provided:%s.\n"
						                                     , msg.name);
#endif
				msg.nodeid = _name_lookup(msg.name);

				/* Send response. */
				source = hal_mailbox_open(msg.source);
				assert(source >= 0);
				assert(hal_mailbox_write(source, &msg, HAL_MAILBOX_MSG_SIZE)
				                                   == HAL_MAILBOX_MSG_SIZE);
				assert(hal_mailbox_close(source) == 0);
				break;

			/* Add name. */
			case NAME_ADD:
#ifdef DEBUG
				printf("Entering NAME_ADD case... [nodeid ID: %d, name: %s].\n"
				                                       , msg.nodeid, msg.name);
#endif
				tmp = nr_registration;

				if (_name_link(msg.nodeid, msg.name) == (tmp + 1))
				{
					msg.op = NAME_SUCCESS;
				}
				else
				{
					msg.op = NAME_FAIL;
				}

				/* Send acknowledgement. */
				source = hal_mailbox_open(msg.source);
				assert(source >= 0);
				assert(hal_mailbox_write(source, &msg, HAL_MAILBOX_MSG_SIZE)
				                                   == HAL_MAILBOX_MSG_SIZE);
				assert(hal_mailbox_close(source) == 0);

				break;

			/* Remove name. */
			case NAME_REMOVE:
#ifdef DEBUG
				printf("Entering NAME_REMOVE case... name: %s.\n", msg.name);
#endif
				assert(nr_registration > 0);

				tmp = nr_registration;

				if (_name_unlink(msg.name) == (tmp - 1))
				{
					msg.op = NAME_SUCCESS;
				}
				else
				{
					msg.op = NAME_FAIL;
				}

				/* Send acknowledgement. */
				source = hal_mailbox_open(msg.source);
				assert(source >= 0);
				assert(hal_mailbox_write(source, &msg, HAL_MAILBOX_MSG_SIZE)
				                                   == HAL_MAILBOX_MSG_SIZE);
				assert(hal_mailbox_close(source) == 0);

				break;

			/* Should not happen. */
			default:
				break;
		}
	}

	/* House keeping. */
	pthread_mutex_lock(&lock);
		hal_mailbox_unlink(inbox);
	pthread_mutex_unlock(&lock);

	return (NULL);
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
	/* printf("[NAME] booting up server\n"); */
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
	/* printf("[NAME] server alive\n"); */
#endif

	/* Wait for name server thread. */
	pthread_join(tid, NULL);

	/* House keeping. */
	barrier_close(global_barrier);

	return (EXIT_SUCCESS);
}
