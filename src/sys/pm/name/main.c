/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
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

#include <nanvix/servers/message.h>
#include <nanvix/servers/name.h>
#include <nanvix/servers/spawn.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/utils.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/noc.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <stdint.h>

#define __DEBUG_NAME 0

#if (__DEBUG_NAME)
	#define name_debug(fmt, ...) uprintf(fmt, __VA_ARGS__)
#else
	#define name_debug(fmt, ...) { }
#endif

/**
 * @brief Number of registration.
 */
static int nr_registration = 0;

/**
 * @brief Inbox.
 */
static int inbox = -1;

/**
 * @brief Lookup table of process names.
 */
static struct {
	int nodenum;                     /**< NoC node.     */
	char name[NANVIX_PROC_NAME_MAX]; /**< Process name. */
} names[NANVIX_NODES_NUM];

/**
 * @brief Server stats.
 */
static struct
{
	int nlinks;         /**< Number of name link requests.   */
	int nunlinks;       /**< Number of unlink name requests. */
	int nlookups;       /**< Number of lookup requests.      */
} stats = { 0, 0, 0};

/*===================================================================*
 * do_name_init()                                                    *
 *===================================================================*/

/**
 * @brief Initializes the name server.
 */
static void do_name_init(void)
{
	/* Initialize lookup table. */
	for (int i = 0; i < NANVIX_NODES_NUM; i++)
	{
		names[i].nodenum = i;
		ustrcpy(names[i].name, "");
	}

	ustrcpy(names[NAME_SERVER_NODE].name, "/io0");

	uassert((inbox = stdinbox_get() >= 0));

	/* Unblock spawner. */
	uprintf("[nanvix][name] server alive");
	uprintf("[nanvix][name] attached to node %d", knode_get_num());
	uprintf("[nanvix][name] listening to inbox %d", inbox);
	uprintf("[nanvix][name] syncing in sync %d", stdsync_get());
}

/*=======================================================================*
 * do_name_lookup()                                                      *
 *=======================================================================*/

/**
 * @brief Converts a name into a NoC node number.
 *
 * @param name Target name.
 *
 * @returns Upon successful completion the NoC node number whose name is @p
 * name is returned. Upon failure, a negative error code is returned
 * instead.
 */
static int do_name_lookup(const char *name)
{
	name_debug("lookup name=%s", name);

	/* Search for portal name. */
	for (int i = 0; i < NANVIX_NODES_NUM; i++)
	{
		/* Found. */
		if (!ustrcmp(name, names[i].name))
			return (names[i].nodenum);
	}

	return (-ENOENT);
}

/*=======================================================================*
 * do_name_link()                                                        *
 *=======================================================================*/

/**
 * @brief Register a process name.
 *
 * @param nodenum Target NoC node.
 * @param name    Name of the process to register.
 *
 * @returns Upon successful registration the number of name registered
 * is returned. Upon failure, a negative error code is returned instead.
 */
static int do_name_link(int nodenum, char *name)
{
	int index;          /* Index where the process will be stored. */

	name_debug("link nodenum=%d name=%s", nodenum, name);

	/* No entry available. */
	if (nr_registration >= NANVIX_NODES_NUM)
		return (-EINVAL);

	/* Check that the name is not already used */
	for (int i = 0; i < NANVIX_NODES_NUM; i++)
	{
		if (ustrcmp(names[i].name, name) == 0)
			return (-EINVAL);
	}

	/* Find index. */
	for (int i = 0; i < NANVIX_NODES_NUM; i++)
	{
		/* Found. */
		if (names[i].nodenum == nodenum)
		{
			index = i;
			goto found;
		}
	}

	return (-EINVAL);

found:

	/* Entry not available */
	if (ustrcmp(names[index].name, ""))
		return (-EINVAL);

	ustrcpy(names[index].name, name);

	return (++nr_registration);
}

/*=======================================================================*
 *do_name_unlink()                                                       *
 *=======================================================================*/

/**
 * @brief Remove a name
 *
 * @param name Name of the process to unlink.
 *
 * @returns Upon successful registration the new number of name registered
 * is returned. Upon failure, a negative error code is returned instead.
 */
static int do_name_unlink(char *name)
{
	/* Search for portal name. */
	int i = 0;

	name_debug("unlink name=%s", name);

	while (i < NANVIX_NODES_NUM && ustrcmp(name, names[i].name))
	{
		i++;
	}

	if (i < NANVIX_NODES_NUM)
	{
		ustrcpy(names[i].name, "");
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
 * @returns Always returns 0.
 */
int do_name_server(void)
{
	int tmp;
	int source;
	int shutdown = 0;

	uprintf("[nanvix][name] booting up server");

	do_name_init();

	while (!shutdown)
	{
		struct name_message msg;

		uassert(kmailbox_read(inbox, &msg, sizeof(struct name_message)) == sizeof(struct name_message));

		name_debug("received request opcode=%d", msg.header.opcode);

		/* Handle name requests. */
		switch (msg.header.opcode)
		{
			/* Lookup. */
			case NAME_LOOKUP:
				stats.nlookups++;
				msg.nodenum = do_name_lookup(msg.name);

				/* Send response. */
				source = kmailbox_open(msg.header.source, msg.header.mailbox_port);

				uassert(source >= 0);
				uassert(kmailbox_write(source, &msg, sizeof(struct name_message)) == sizeof(struct name_message));
				uassert(kmailbox_close(source) == 0);

				break;

			/* Add name. */
			case NAME_LINK:
				stats.nlinks++;
				tmp = nr_registration;

				if (do_name_link(msg.nodenum, msg.name) == (tmp + 1))
					msg.header.opcode = NAME_SUCCESS;
				else
					msg.header.opcode = NAME_FAIL;

				uassert(nr_registration >= 0);

				/* Send acknowledgement. */
				source = kmailbox_open(msg.header.source, msg.header.mailbox_port);
				uassert(source >= 0);
				uassert(kmailbox_write(source, &msg, sizeof(struct name_message)) == sizeof(struct name_message));
				uassert(kmailbox_close(source) == 0);

				break;

			/* Remove name. */
			case NAME_UNLINK:
				stats.nunlinks++;
				tmp = nr_registration;

				if ((tmp > 0) && (do_name_unlink(msg.name) == (tmp - 1)))
					msg.header.opcode = NAME_SUCCESS;
				else
					msg.header.opcode = NAME_FAIL;

				uassert(nr_registration >= 0);

				/* Send acknowledgement. */
				source = kmailbox_open(msg.header.source, msg.header.mailbox_port);
				uassert(source >= 0);
				uassert(kmailbox_write(source, &msg, sizeof(struct name_message)) == sizeof(struct name_message));
				uassert(kmailbox_close(source) == 0);

				break;

			case NAME_EXIT:
				shutdown = 1;
				break;

			/* Should not happen. */
			default:
				break;
		}
	}

	uprintf("[nanvix][name] shutting down server");

	return (0);
}

/*============================================================================*
 * __main2()                                                                  *
 *============================================================================*/

/**
 * @brief Handles remote name requests.
 *
 * @returns Always returns zero.
 */
int name_server(void)
{
	__runtime_setup(0);

		do_name_server();

	__runtime_cleanup();

	return (0);
}
