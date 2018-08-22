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
#include <stdio.h>
#include <pthread.h>

#include <nanvix/const.h>
#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/pm.h>

/**
 * @brief Mailbox for small messages.
 */
static int server;

/**
 * @brief Is the name service initialized ?
 */
static int initialized = 0;

/**
* @brief Name linked to the process.
*/
static char process_name[HAL_NR_NOC_IONODES][NANVIX_PROC_NAME_MAX];

/**
 * @brief Mailbox module lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*============================================================================*
 * name_init()                                                                *
 *============================================================================*/

/**
 * @brief Initializes the naming client.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int name_init(void)
{
	/* Sanity check at compile time: Mailbox compliant */
	CHECK_MAILBOX_MSG_SIZE(struct name_message);

	/* Nothing to do. */
	if (initialized)
		return (0);

	server = sys_mailbox_open(NAME_SERVER_NODE);

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
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int name_finalize(void)
{
	/* Nothing to do. */
	if (!initialized)
		return (0);

	if (sys_mailbox_close(NAME_SERVER_NODE) < 0)
		return (-EAGAIN);

	initialized = 0;

	return (0);
}

/*============================================================================*
 * get_name()                                                                 *
 *============================================================================*/

/**
 * @brief Get the name of the running process.
 *
 * @param name Address where the name will be written.
 *
 * @returns Upon successful zero is returned.
 * Upon failure, a negative error code is returned instead.
 */
int get_name(char *name)
{
	int index;

	/* Sanity check. */
	if (name == NULL)
		return (-EINVAL);

	index = sys_get_core_id();

	/* Name was not initialized. */
	if (!strcmp(process_name[index], ""))
		return (-EAGAIN);

	strcpy(name, process_name[index]);

	return (0);
}

/*============================================================================*
 * name_lookup()                                                              *
 *============================================================================*/

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
	struct name_message msg;

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Bad name. */
	if ((strlen(name) >= (NANVIX_PROC_NAME_MAX - 1)) || (!strcmp(name, "")))
		return (-EINVAL);

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Build operation header. */
	msg.source = sys_get_node_num();
	msg.op = NAME_LOOKUP;
	msg.nodenum = -1;
	strcpy(msg.name, name);

	pthread_mutex_lock(&lock);

		if (sys_mailbox_write(server, &msg, sizeof(struct name_message)) != sizeof(struct name_message))
			goto error1;

		if (sys_mailbox_read(get_inbox(), &msg, sizeof(struct name_message)) != sizeof(struct name_message))
			goto error1;

	pthread_mutex_unlock(&lock);

	return (msg.nodenum);

error1:
	pthread_mutex_unlock(&lock);
	return (-EAGAIN);
}

/*============================================================================*
 * name_link()                                                                *
 *============================================================================*/

/**
 * @brief link a process name.
 *
 * @param nodenum NoC node ID of the process to link.
 * @param name   Name of the process to link.
 *
 * @returns Upon successful completion 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
int name_link(int nodenum, const char *name)
{
	struct name_message msg;

	/* Invalid NoC node ID. */
	if (nodenum < 0)
		return (-EINVAL);

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Bad name. */
	if ((strlen(name) >= (NANVIX_PROC_NAME_MAX - 1)) || (!strcmp(name, "")))
		return (-EINVAL);

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Build operation header. */
	msg.source = sys_get_node_num();
	msg.op = NAME_LINK;
	msg.nodenum = nodenum;
	strcpy(msg.name, name);

	pthread_mutex_lock(&lock);

		/* Send link request. */
		if (sys_mailbox_write(server, &msg, sizeof(struct name_message)) != sizeof(struct name_message))
			goto error1;

		/* Wait server response */
		if (sys_mailbox_read(get_inbox(), &msg, sizeof(struct name_message)) != sizeof(struct name_message))
			goto error1;

	pthread_mutex_unlock(&lock);

	if ((msg.op != NAME_SUCCESS) && (msg.op != NAME_FAIL))
		return (-EAGAIN);

	if (msg.op == NAME_SUCCESS)
	{
		strcpy(process_name[sys_get_core_id()], name);

		return (0);
	}

	return (-1);

error1:
	pthread_mutex_unlock(&lock);
	return (-EAGAIN);
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
	struct name_message msg;

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Bad name. */
	if ((strlen(name) >= (NANVIX_PROC_NAME_MAX - 1)) || (!strcmp(name, "")))
		return (-EINVAL);

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Build operation header. */
	msg.source = sys_get_node_num();
	msg.op = NAME_UNLINK;
	msg.nodenum = -1;
	strcpy(msg.name, name);

	pthread_mutex_lock(&lock);

		if (sys_mailbox_write(server, &msg, sizeof(struct name_message)) != sizeof(struct name_message))
			goto error1;

		/* Wait server response */
		if (sys_mailbox_read(get_inbox(), &msg, sizeof(struct name_message)) != sizeof(struct name_message))
			goto error1;

	pthread_mutex_unlock(&lock);

	if ((msg.op != NAME_SUCCESS) && (msg.op != NAME_FAIL))
		return (-EAGAIN);

	if (msg.op == NAME_SUCCESS)
		return (0);

	return (-1);

error1:
	pthread_mutex_unlock(&lock);
	return (-EAGAIN);
}
