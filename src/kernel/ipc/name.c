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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <nanvix/config.h>
#include <nanvix/name.h>
#include <nanvix/pm.h>
#include <nanvix/hal.h>

/**
 * @brief Name server message.
 */
static struct name_message msg;

/**
 * @brief Mailboxe for small messages.
 */
static int server;

/**
 * @brief Lock for critical sections.
 */
static pthread_mutex_t lock;

/**
 * @brief Is the name service initialized ?
 */
static int initialized = 0;

/*============================================================================*
 * name_init()                                                                *
 *============================================================================*/

/**
 * @brief Initializes the naming client.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int name_init(void)
{
	/* Nothing to do. */
	if (initialized)
		return (0);

	pthread_mutex_init(&lock, NULL);

	pthread_mutex_lock(&lock);
	server = hal_mailbox_open(hal_noc_nodes[NAME_SERVER_NODE]);
	pthread_mutex_unlock(&lock);

	if (server >= 0)
	{
		initialized = 1;
		return (0);
	}

	return (-1);
}

/*============================================================================*
 * name_finalize()                                                            *
 *============================================================================*/

/**
 * @brief Closes the naming client.
 */
void name_finalize(void)
{
	/* Nothing to do. */
	if (!initialized)
		return;

	pthread_mutex_init(&lock, NULL);

	pthread_mutex_lock(&lock);
	hal_mailbox_close(server);
	pthread_mutex_unlock(&lock);

	initialized = 0;
}

/*============================================================================*
 * name_lookup()                                                              *
 *============================================================================*/

/**
 * @brief Converts a name into a NoC node ID.
 *
 * @param name 		Target name.
 *
 * @returns Upon successful completion the NoC node ID whose name is @p
 * name is returned. Upon failure, a negative error code is returned
 * instead.
 */
int name_lookup(char *name)
{
	/* Sanity check. */
	if ((name == NULL) ||(strlen(name) >= (NANVIX_PROC_NAME_MAX - 1))
										  || (strcmp(name, "") == 0))
		return (-EINVAL);

	#ifdef DEBUG
		printf("name_lookup(%s) called from node %d...\n", name,
											 hal_get_node_id());
	#endif

	if (name_init() != 0)
		return (-EAGAIN);

	pthread_mutex_init(&lock, NULL);

	pthread_mutex_lock(&lock);

	/* Build operation header. */
	msg.source = hal_get_node_id();
	msg.op = NAME_LOOKUP;
	msg.nodeid = -1;
	strcpy(msg.name, name);

	/* Send name request. */
	#ifdef DEBUG
		printf("Sending request for name: %s...\n", msg.name);
	#endif


	if (hal_mailbox_write(server, &msg, HAL_MAILBOX_MSG_SIZE)
									 != HAL_MAILBOX_MSG_SIZE)
	{
		pthread_mutex_unlock(&lock);
		return (-EAGAIN);
	}

	while(msg.nodeid == -1)
	{
		if (hal_mailbox_read(get_inbox(), &msg, HAL_MAILBOX_MSG_SIZE)
											 != HAL_MAILBOX_MSG_SIZE)
		{
			pthread_mutex_unlock(&lock);
			return (-EAGAIN);
		}
	}

	pthread_mutex_unlock(&lock);

	return (msg.nodeid);
}

/*============================================================================*
 * name_link()                                                                *
 *============================================================================*/

/**
 * @brief link a process name.
 *
 * @param nodeid    NoC node ID of the process to link.
 * @param name      Name of the process to link.
 *
 * @returns Upon successful completion 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
int name_link(int nodeid, const char *name)
{
	/* Sanity check. */
	if ((name == NULL) || (strlen(name) >= (NANVIX_PROC_NAME_MAX - 1))
						   || (nodeid < 0) || (strcmp(name, "") == 0))
		return (-EINVAL);

	if (name_init() != 0)
		return (-EAGAIN);

	pthread_mutex_init(&lock, NULL);

	pthread_mutex_lock(&lock);

	/* Build operation header. */
	msg.source = hal_get_node_id();
	msg.op = NAME_LINK;
	msg.nodeid = nodeid;
	strcpy(msg.name, name);

	/* Send link request. */
	if (hal_mailbox_write(server, &msg, HAL_MAILBOX_MSG_SIZE)
									 != HAL_MAILBOX_MSG_SIZE)
		return (-EAGAIN);

	/* Wait server response */
	while (msg.op == NAME_LINK)
	{
		if (hal_mailbox_read(get_inbox(), &msg, HAL_MAILBOX_MSG_SIZE) !=
												   HAL_MAILBOX_MSG_SIZE)
		{
			pthread_mutex_unlock(&lock);
			return (-EAGAIN);
		}
	}

	pthread_mutex_unlock(&lock);

	if ((msg.op != NAME_SUCCESS) && (msg.op != NAME_FAIL))
		return (-EAGAIN);

	if (msg.op == NAME_SUCCESS)
		return (0);

	return (-1);
}

/*============================================================================*
 * name_unlink()                                                              *
 *============================================================================*/

/**
 * @brief Unlink a process name.
 *
 * @param name	    Name of the process to unlink.
 *
 * @returns Upon successful completion 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
int name_unlink(const char *name)
{
	pthread_mutex_init(&lock, NULL);

	/* Sanity check. */
	if ((name == NULL) ||(strlen(name) >= (NANVIX_PROC_NAME_MAX - 1))
										  || (strcmp(name, "") == 0))
		return (-EINVAL);

	#ifdef DEBUG
		printf("name_unlink(%s): called from node %d...\n",
								  name, hal_get_node_id());
	#endif

	if (name_init() != 0)
		return (-EAGAIN);

	pthread_mutex_lock(&lock);

	/* Build operation header. */
	msg.source = hal_get_node_id();
	msg.op = NAME_UNLINK;
	msg.nodeid = -1;
	strcpy(msg.name, name);

	/* Send name request. */
	#ifdef DEBUG
		printf("Sending remove request for name: %s...\n", msg.name);
	#endif

	if (hal_mailbox_write(server, &msg, HAL_MAILBOX_MSG_SIZE)
									 != HAL_MAILBOX_MSG_SIZE)
	{
		pthread_mutex_unlock(&lock);
		return (-EAGAIN);
	}

	/* Wait server response */
	while(msg.op == NAME_UNLINK)
	{
		if (hal_mailbox_read(get_inbox(), &msg, HAL_MAILBOX_MSG_SIZE)
											 != HAL_MAILBOX_MSG_SIZE)
		{
			pthread_mutex_unlock(&lock);
			return (-EAGAIN);
		}
	}

	pthread_mutex_unlock(&lock);

	if ((msg.op != NAME_SUCCESS) && (msg.op != NAME_FAIL))
		return (-EAGAIN);

	if (msg.op == NAME_SUCCESS)
		return (0);

	return (-1);
}
