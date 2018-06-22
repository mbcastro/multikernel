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
#include <assert.h>
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
 * @brief Mailboxes for small messages.
 */
/**@{*/
static int server;
static int client;
/**@}*/

/**
 * @brief Lock for critical sections.
 */
static pthread_mutex_t lock;

/**
 * @brief Is the name service initialized ?
 */
static int initialized = 0;

/*============================================================================*
 * name_get_inbox()                                                                *
 *============================================================================*/

/**
 * @brief Get the input mailbox NoC connector of the client.
 *
 * @returns Upon successful completion, the input mailbox
 * is returned. Upon failure, a negative error code is returned instead.
 */
int name_get_inbox(void)
{
	if (initialized)
		return (client);
	else
		return (-EAGAIN);
}

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
	client = hal_mailbox_create(hal_get_node_id());
	server = hal_mailbox_open(hal_noc_nodes[NAME_SERVER_NODE]);

	if ((client >= 0) && (server >= 0))
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

	assert(hal_mailbox_close(server) == 0);
	assert(hal_mailbox_close(client) == 0);

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

	assert(name_init() == 0);

	/* Build operation header. */
	msg.source = hal_get_node_id();
	msg.op = NAME_LOOKUP;
	msg.nodeid = -1;
	strcpy(msg.name, name);

	/* Send name request. */
	#ifdef DEBUG
		printf("Sending request for name: %s...\n", msg.name);
	#endif

	assert(hal_mailbox_write(server, &msg, HAL_MAILBOX_MSG_SIZE)
	                                   == HAL_MAILBOX_MSG_SIZE);

	while(msg.nodeid == -1)
	{
		assert(hal_mailbox_read(client, &msg, HAL_MAILBOX_MSG_SIZE)
		                                 == HAL_MAILBOX_MSG_SIZE);
	}

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
	struct name_message msg_link;

	/* Sanity check. */
	if ((name == NULL) || (strlen(name) >= (NANVIX_PROC_NAME_MAX - 1))
	                || (nodeid < 0) || (strcmp(name, "") == 0))
		return (-EINVAL);

	if (name_init() != 0)
		return (-EAGAIN);

	/* Build operation header. */
	msg_link.source = hal_get_node_id();
	msg_link.op = NAME_LINK;
	msg_link.nodeid = nodeid;
	strcpy(msg_link.name, name);
	printf("send %d\n", hal_get_node_id());
	/* Send link request. */
	if (hal_mailbox_write(server, &msg_link, HAL_MAILBOX_MSG_SIZE)
	                                 != HAL_MAILBOX_MSG_SIZE)
		return (-EAGAIN);

	/* Wait server response */
	while(msg_link.op == NAME_LINK){
		pthread_mutex_lock(&lock);
		printf("read: %d\n", hal_mailbox_read(client, &msg_link, HAL_MAILBOX_MSG_SIZE));
		pthread_mutex_unlock(&lock);
	}
	printf("receive %d from %d\n", msg_link.op, hal_get_node_id());

	if ((msg_link.op != NAME_SUCCESS) && (msg_link.op != NAME_FAIL))
		return (-EAGAIN);


	if (msg_link.op == NAME_SUCCESS)
	{
		return (0);
	}
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
	/* Sanity check. */
	if ((name == NULL) ||(strlen(name) >= (NANVIX_PROC_NAME_MAX - 1))
	                               || (strcmp(name, "") == 0))
		return (-EINVAL);

	#ifdef DEBUG
		printf("name_unlink(%s): called from node %d...\n",
		                          name, hal_get_node_id());
	#endif

	assert(name_init() == 0);

	/* Build operation header. */
	msg.source = hal_get_node_id();
	msg.op = NAME_UNLINK;
	msg.nodeid = -1;
	strcpy(msg.name, name);

	/* Send name request. */
	#ifdef DEBUG
		printf("Sending remove request for name: %s...\n", msg.name);
	#endif

	assert(hal_mailbox_write(server, &msg, HAL_MAILBOX_MSG_SIZE)
	                                    == HAL_MAILBOX_MSG_SIZE);

	/* Wait server response */
	while(msg.op == NAME_UNLINK){
		assert(hal_mailbox_read(client, &msg, HAL_MAILBOX_MSG_SIZE)
	                                      == HAL_MAILBOX_MSG_SIZE);
	}

	assert((msg.op == NAME_SUCCESS) || (msg.op == NAME_FAIL));

	if (msg.op == NAME_SUCCESS)
	{
		return (0);
	}
	return (-1);
}
