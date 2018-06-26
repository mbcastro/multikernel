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

#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_MAILBOX_
#include <nanvix/config.h>
#include <nanvix/hal.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>

#ifdef DEBUG
#include <stdio.h>
#endif

/* Import definitions. */
extern pthread_mutex_t lock;
extern pthread_mutex_t barrier;

/**
 * @brief Number of registration.
 */
static int nr_registration = 0;

/**
 * @brief Lookup table of process names.
 */
static struct {
	int nodeid;                      /**< NoC node ID.  */
	char name[NANVIX_PROC_NAME_MAX]; /**< Process name. */
} names[HAL_NR_NOC_NODES];

/*===================================================================*
 * name_init()                                                       *
 *===================================================================*/

/**
 * @brief Initializes the name server.
 */
static void name_init(void)
{
	/* Initialize lookup table. */
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
	{
		names[i].nodeid = hal_noc_nodes[i];
		strcpy(names[i].name, "");
	}

	strcpy(names[hal_noc_nodes[NAME_SERVER_NODE]].name, "/io0");
}

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
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
	{
		if (strcmp(names[i].name, name) == 0)
			return (-EINVAL);
	}

	/* Find index. */
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
	{
		/* Found. */
		if (names[i].nodeid == nodeid)
		{
			index = i;
			goto found;
		}
	}

	return (-EINVAL);

found:

	/* Entry not available */
	if (strcmp(names[index].name, ""))
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
		strcpy(names[i].name, "");
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
void *name_server(void *args)
{
	int inbox;   /* Mailbox for small messages. */
	int source;  /* NoC node ID of the client   */
	int tmp;

	((void) args);

	hal_setup();

	name_init();

	/* Open server mailbox. */
	pthread_mutex_lock(&lock);
		inbox = hal_mailbox_create(hal_noc_nodes[NAME_SERVER_NODE]);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	while(1)
	{
		struct name_message msg;

		pthread_mutex_lock(&lock);
		assert(hal_mailbox_read(inbox, &msg, sizeof(struct name_message)) == sizeof(struct name_message));
		pthread_mutex_unlock(&lock);

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
				pthread_mutex_lock(&lock);
				source = hal_mailbox_open(msg.source);
				pthread_mutex_unlock(&lock);

				assert(source >= 0);

				pthread_mutex_lock(&lock);
				assert(hal_mailbox_write(source, &msg, sizeof(struct name_message)) == sizeof(struct name_message));
				pthread_mutex_unlock(&lock);

				pthread_mutex_lock(&lock);
				assert(hal_mailbox_close(source) == 0);
				pthread_mutex_unlock(&lock);

				break;

			/* Add name. */
			case NAME_LINK:
#ifdef DEBUG
				printf("Entering NAME_LINK case... [nodeid ID: %d, name: %s].\n"
														  msg.nodeid, msg.name);
#endif
				tmp = nr_registration;

				if (_name_link(msg.nodeid, msg.name) == (tmp + 1))
					msg.op = NAME_SUCCESS;
				else
					msg.op = NAME_FAIL;

				assert(nr_registration >= 0);

				/* Send acknowledgement. */
				pthread_mutex_lock(&lock);
				source = hal_mailbox_open(msg.source);
				pthread_mutex_unlock(&lock);

				assert(source >= 0);

				pthread_mutex_lock(&lock);
				assert(hal_mailbox_write(source, &msg, sizeof(struct name_message)) == sizeof(struct name_message));
				pthread_mutex_unlock(&lock);

				pthread_mutex_lock(&lock);
				assert(hal_mailbox_close(source) == 0);
				pthread_mutex_unlock(&lock);

				break;

			/* Remove name. */
			case NAME_UNLINK:
#ifdef DEBUG
				printf("Entering NAME_UNLINK case... name: %s.\n", msg.name);
#endif
				tmp = nr_registration;

				if ((tmp > 0) && (_name_unlink(msg.name) == (tmp - 1)))
					msg.op = NAME_SUCCESS;
				else
					msg.op = NAME_FAIL;

				assert(nr_registration >= 0);

				/* Send acknowledgement. */
				pthread_mutex_lock(&lock);
				source = hal_mailbox_open(msg.source);
				pthread_mutex_unlock(&lock);

				assert(source >= 0);

				pthread_mutex_lock(&lock);
				assert(hal_mailbox_write(source, &msg, sizeof(struct name_message)) == sizeof(struct name_message));
				pthread_mutex_unlock(&lock);

				pthread_mutex_lock(&lock);
				assert(hal_mailbox_close(source) == 0);
				pthread_mutex_unlock(&lock);

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

	hal_cleanup();
	return (NULL);
}
