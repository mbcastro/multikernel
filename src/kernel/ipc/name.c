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
#include <string.h>
#include <pthread.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_MAILBOX_
#include <nanvix/config.h>
#include <nanvix/name.h>
#include <nanvix/pm.h>
#include <nanvix/hal.h>

/* Forward definitions. */
extern pthread_mutex_t kernel_lock;

/**
 * @brief Name server message.
 */
static struct name_message msg;

/**
 * @brief Mailboxe for small messages.
 */
static int server;

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

	server = hal_mailbox_open(hal_noc_nodes[NAME_SERVER_NODE]);

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

	hal_mailbox_close(server);

	initialized = 0;
}

/*============================================================================*
 * name_lookup()                                                              *
 *============================================================================*/

/**
 * @brief Internal name_lookup()
 */
static int _name_lookup(char *name)
{
	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Bad name. */
	if ((strlen(name) >= (NANVIX_PROC_NAME_MAX - 1)) || (!strcmp(name, "")))
		return (-EINVAL);

	/* Initilize name client. */
	if (name_init() != 0)
		return (-EAGAIN);

	/* Build operation header. */
	msg.source = hal_get_node_id();
	msg.op = NAME_LOOKUP;
	msg.nodeid = -1;
	strcpy(msg.name, name);

	if (hal_mailbox_write(server, &msg, HAL_MAILBOX_MSG_SIZE) != HAL_MAILBOX_MSG_SIZE)
		return (-EAGAIN);

	while (msg.nodeid == -1)
	{
		if (hal_mailbox_read(get_inbox(), &msg, HAL_MAILBOX_MSG_SIZE) != HAL_MAILBOX_MSG_SIZE)
			return (-EAGAIN);
	}

	return (msg.nodeid);
}

/**
 * @brief Converts a name into a NoC node ID.
 *
 * @param name Target name.
 *
 * @returns Upon successful completion the NoC node ID whose name is @p
 * name is returned. Upon failure, a negative error code is returned
 * instead.
 */
int name_lookup(char *name)
{
	int ret;

	pthread_mutex_lock(&kernel_lock);
		ret = _name_lookup(name);
	pthread_mutex_unlock(&kernel_lock);

	return (ret);
}

/*============================================================================*
 * name_link()                                                                *
 *============================================================================*/

/**
 * @brief Internal name_link()
 */
static int _name_link(int nodeid, const char *name)
{
	/* Invalid NoC node ID. */
	if (nodeid < 0)
		return (-EINVAL);

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Bad name. */
	if ((strlen(name) >= (NANVIX_PROC_NAME_MAX - 1)) || (!strcmp(name, "")))
		return (-EINVAL);

	/* Build operation header. */
	msg.source = hal_get_node_id();
	msg.op = NAME_LINK;
	msg.nodeid = nodeid;
	strcpy(msg.name, name);

	/* Send link request. */
	if (hal_mailbox_write(server, &msg, HAL_MAILBOX_MSG_SIZE) != HAL_MAILBOX_MSG_SIZE)
		return (-EAGAIN);

	/* Wait server response */
	while (msg.op == NAME_LINK)
	{
		if (hal_mailbox_read(get_inbox(), &msg, HAL_MAILBOX_MSG_SIZE) != HAL_MAILBOX_MSG_SIZE)
			return (-EAGAIN);
	}

	if ((msg.op != NAME_SUCCESS) && (msg.op != NAME_FAIL))
		return (-EAGAIN);

	if (msg.op == NAME_SUCCESS)
		return (0);

	return (-1);
}

/**
 * @brief link a process name.
 *
 * @param nodeid NoC node ID of the process to link.
 * @param name   Name of the process to link.
 *
 * @returns Upon successful completion 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
int name_link(int nodeid, const char *name)
{
	int ret;

	pthread_mutex_lock(&kernel_lock);
		ret = _name_link(nodeid, name);
	pthread_mutex_unlock(&kernel_lock);

	return (ret);
}

/*============================================================================*
 * name_unlink()                                                              *
 *============================================================================*/

/**
 * @brief Internal name_unlink()
 */
static int _name_unlink(const char *name)
{
	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Bad name. */
	if ((strlen(name) >= (NANVIX_PROC_NAME_MAX - 1)) || (!strcmp(name, "")))
		return (-EINVAL);

	if (name_init() != 0)
		return (-EAGAIN);

	/* Build operation header. */
	msg.source = hal_get_node_id();
	msg.op = NAME_UNLINK;
	msg.nodeid = -1;
	strcpy(msg.name, name);

	if (hal_mailbox_write(server, &msg, HAL_MAILBOX_MSG_SIZE) != HAL_MAILBOX_MSG_SIZE)
		return (-EAGAIN);

	/* Wait server response */
	while(msg.op == NAME_UNLINK)
	{
		if (hal_mailbox_read(get_inbox(), &msg, HAL_MAILBOX_MSG_SIZE) != HAL_MAILBOX_MSG_SIZE)
			return (-EAGAIN);
	}

	if ((msg.op != NAME_SUCCESS) && (msg.op != NAME_FAIL))
		return (-EAGAIN);

	if (msg.op == NAME_SUCCESS)
		return (0);

	return (-1);
}

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
	int ret;

	pthread_mutex_lock(&kernel_lock);
		ret = _name_unlink(name);
	pthread_mutex_unlock(&kernel_lock);

	return (ret);
}
