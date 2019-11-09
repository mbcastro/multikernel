/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define __NAME_SERVICE

#include <nanvix/servers/name.h>
#include <nanvix/servers/spawn.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/mutex.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <posix/stdbool.h>

/**
 * @brief Mailbox for small messages.
 */
static int server;

/**
 * @brief Is the name service initialized ?
 */
static bool initialized = false;

/**
* @brief Name linked to the process.
*/
static char process_name[NANVIX_NODES_NUM][NANVIX_PROC_NAME_MAX];

/**
 * @brief Mailbox module lock.
 */
static struct nanvix_mutex lock;

/*============================================================================*
 * __name_setup()                                                             *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __name_setup(void)
{
	/* Nothing to do. */
	if (initialized)
		return (0);

	/* Open connection with Name Server. */
	if ((server = kmailbox_open(NAME_SERVER_NODE)) < 0)
		return (-1);

	nanvix_mutex_init(&lock);
	initialized = true;

	return (0);
}

/*============================================================================*
 * name_finalize()                                                            *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __name_cleanup(void)
{
	/* Nothing to do. */
	if (!initialized)
		return (0);

	/* Close connection with Name Server. */
	if (kmailbox_close(server) < 0)
		return (-EAGAIN);

	initialized = false;

	return (0);
}

/*============================================================================*
 * name_lookup()                                                              *
 *============================================================================*/

/**
 * @todo TODO: provide a long description for this function.
 */
int name_lookup(const char *name)
{
	struct name_message msg;

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Bad name. */
	if ((ustrlen(name) >= (NANVIX_PROC_NAME_MAX - 1)) || (!ustrcmp(name, "")))
		return (-EINVAL);

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Build operation header. */
	msg.header.source = processor_node_get_num(core_get_id());
	msg.header.opcode = NAME_LOOKUP;
	msg.nodenum = -1;
	ustrcpy(msg.name, name);

	nanvix_mutex_lock(&lock);

		if (kmailbox_write(server, &msg, sizeof(struct name_message)) != sizeof(struct name_message))
			goto error1;

		if (kmailbox_read(stdinbox_get(), &msg, sizeof(struct name_message)) != sizeof(struct name_message))
			goto error1;

	nanvix_mutex_unlock(&lock);

	return (msg.nodenum);

error1:
	nanvix_mutex_unlock(&lock);
	return (-EAGAIN);
}

/*============================================================================*
 * name_link()                                                                *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
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
	if ((ustrlen(name) >= (NANVIX_PROC_NAME_MAX - 1)) || (!ustrcmp(name, "")))
		return (-EINVAL);

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Build operation header. */
	msg.header.source = processor_node_get_num(core_get_id());
	msg.header.opcode = NAME_LINK;
	msg.nodenum = nodenum;
	ustrcpy(msg.name, name);

	nanvix_mutex_lock(&lock);

		/* Send link request. */
		if (kmailbox_write(server, &msg, sizeof(struct name_message)) != sizeof(struct name_message))
			goto error1;

		/* Wait server response */
		if (kmailbox_read(stdinbox_get(), &msg, sizeof(struct name_message)) != sizeof(struct name_message))
			goto error1;

	nanvix_mutex_unlock(&lock);

	if ((msg.header.opcode != NAME_SUCCESS) && (msg.header.opcode != NAME_FAIL))
		return (-EAGAIN);

	if (msg.header.opcode == NAME_SUCCESS)
	{
		ustrcpy(process_name[core_get_id()], name);
		return (0);
	}

	return (-1);

error1:
	nanvix_mutex_unlock(&lock);
	return (-EAGAIN);
}

/*============================================================================*
 * name_unlink()                                                              *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int name_unlink(const char *name)
{
	struct name_message msg;

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Bad name. */
	if ((ustrlen(name) >= (NANVIX_PROC_NAME_MAX - 1)) || (!ustrcmp(name, "")))
		return (-EINVAL);

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Build operation header. */
	msg.header.source = processor_node_get_num(core_get_id());
	msg.header.opcode = NAME_UNLINK;
	msg.nodenum = -1;
	ustrcpy(msg.name, name);

	nanvix_mutex_lock(&lock);

		if (kmailbox_write(server, &msg, sizeof(struct name_message)) != sizeof(struct name_message))
			goto error1;

		/* Wait server response */
		if (kmailbox_read(stdinbox_get(), &msg, sizeof(struct name_message)) != sizeof(struct name_message))
			goto error1;

	nanvix_mutex_unlock(&lock);

	if ((msg.header.opcode != NAME_SUCCESS) && (msg.header.opcode != NAME_FAIL))
		return (-EAGAIN);

	if (msg.header.opcode == NAME_SUCCESS)
		return (0);

	return (-1);

error1:
	nanvix_mutex_unlock(&lock);
	return (-EAGAIN);
}

/*============================================================================*
 * name_shutdown()                                                            *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int name_shutdown(void)
{
	int ret;
	struct name_message msg;

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Build operation header. */
	msg.header.source = processor_node_get_num(core_get_id());
	msg.header.opcode = NAME_EXIT;

		if ((ret = kmailbox_write(server, &msg, sizeof(struct name_message))) != sizeof(struct name_message))
			return (ret);

	return (0);
}
