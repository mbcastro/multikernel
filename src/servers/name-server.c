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
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <nanvix/spawner.h>
#include <nanvix/utils.h>
#include <nanvix/syscalls.h>
#include <nanvix/const.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>

#ifdef DEBUG_NAME
	#define name_debug(fmt, ...) debug("name", fmt, __VA_ARGS__)
#else
	#define name_debug(fmt, ...) { }
#endif

/**
 * @brief Number of registration.
 */
static int nr_registration = 0;

/**
 * @brief Lookup table of process names.
 */
static struct {
	int nodenum;                     /**< NoC node.     */
	char name[NANVIX_PROC_NAME_MAX]; /**< Process name. */
} names[NANVIX_NR_NODES];

/**
 * @brief Server stats.
 */
static struct
{
	int nlinks;         /**< Number of name link requests.   */
	int nunlinks;       /**< Number of unlink name requests. */
	int nlookups;       /**< Number of lookup requests.      */
	uint64_t tstart;    /**< Start time.                     */
	uint64_t tshutdown; /**< Shutdown time.                  */
} stats = { 0, 0, 0, 0, 0 };

/*===================================================================*
 * _name_init()                                                       *
 *===================================================================*/

/**
 * @brief Initializes the name server.
 */
static void _name_init(void)
{
	/* Initialize lookup table. */
	for (int i = 0; i < NANVIX_NR_NODES; i++)
	{
		names[i].nodenum = i;
		strcpy(names[i].name, "");
	}

	strcpy(names[NAME_SERVER_NODE].name, "/io0");
}

/*=======================================================================*
 * _name_lookup()                                                        *
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
static int _name_lookup(const char *name)
{
	name_debug("lookup name=%s", name);

	/* Search for portal name. */
	for (int i = 0; i < NANVIX_NR_NODES; i++)
	{
		/* Found. */
		if (!strcmp(name, names[i].name))
			return (names[i].nodenum);
	}

	return (-ENOENT);
}

/*=======================================================================*
 * _name_link()                                                          *
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
static int _name_link(int nodenum, char *name)
{
	int index;          /* Index where the process will be stored. */

	name_debug("link nodenum=%d name=%s", nodenum, name);

	/* No entry available. */
	if (nr_registration >= NANVIX_NR_NODES)
		return (-EINVAL);

	/* Check that the name is not already used */
	for (int i = 0; i < NANVIX_NR_NODES; i++)
	{
		if (strcmp(names[i].name, name) == 0)
			return (-EINVAL);
	}

	/* Find index. */
	for (int i = 0; i < NANVIX_NR_NODES; i++)
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
	if (strcmp(names[index].name, ""))
		return (-EINVAL);

	strcpy(names[index].name, name);

	return (++nr_registration);
}

/*=======================================================================*
 *_name_unlink()                                                         *
 *=======================================================================*/

/**
 * @brief Remove a name
 *
 * @param name Name of the process to unlink.
 *
 * @returns Upon successful registration the new number of name registered
 * is returned. Upon failure, a negative error code is returned instead.
 */
static int _name_unlink(char *name)
{
	/* Search for portal name. */
	int i = 0;

	name_debug("unlink name=%s", name);

	while (i < NANVIX_NR_NODES && strcmp(name, names[i].name))
	{
		i++;
	}

	if (i < NANVIX_NR_NODES)
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
 * @param inbox    Input mailbox.
 * @param inportal Input portal.
 *
 * @returns Always returns NULL.
 */
int name_server(int inbox, int inportal)
{
	int tmp;
	int source;
	int shutdown = 0;

	((void) inportal);

	printf("[nanvix][name] booting up server\n");

	_name_init();

	spawner_ack();

	printf("[nanvix][name] server alive\n");
	stats.tstart = sys_timer_get();

	while (!shutdown)
	{
		struct name_message msg;

		assert(sys_mailbox_read(inbox, &msg, sizeof(struct name_message)) == sizeof(struct name_message));

		/* Handle name requests. */
		switch (msg.header.opcode)
		{
			/* Lookup. */
			case NAME_LOOKUP:
				stats.nlookups++;
				msg.nodenum = _name_lookup(msg.name);

				/* Send response. */
				source = sys_mailbox_open(msg.header.source);

				assert(source >= 0);
				assert(sys_mailbox_write(source, &msg, sizeof(struct name_message)) == sizeof(struct name_message));
				assert(sys_mailbox_close(source) == 0);

				break;

			/* Add name. */
			case NAME_LINK:
				stats.nlinks++;
				tmp = nr_registration;

				if (_name_link(msg.nodenum, msg.name) == (tmp + 1))
					msg.header.opcode = NAME_SUCCESS;
				else
					msg.header.opcode = NAME_FAIL;

				assert(nr_registration >= 0);

				/* Send acknowledgement. */
				source = sys_mailbox_open(msg.header.source);
				assert(source >= 0);
				assert(sys_mailbox_write(source, &msg, sizeof(struct name_message)) == sizeof(struct name_message));
				assert(sys_mailbox_close(source) == 0);

				break;

			/* Remove name. */
			case NAME_UNLINK:
				stats.nunlinks++;
				tmp = nr_registration;

				if ((tmp > 0) && (_name_unlink(msg.name) == (tmp - 1)))
					msg.header.opcode = NAME_SUCCESS;
				else
					msg.header.opcode = NAME_FAIL;

				assert(nr_registration >= 0);

				/* Send acknowledgement. */
				source = sys_mailbox_open(msg.header.source);
				assert(source >= 0);
				assert(sys_mailbox_write(source, &msg, sizeof(struct name_message)) == sizeof(struct name_message));
				assert(sys_mailbox_close(source) == 0);

				break;

			case NAME_EXIT:
				shutdown = 1;
				break;

			/* Should not happen. */
			default:
				break;
		}
	}

	/* Dump statistics. */
	stats.tshutdown = sys_timer_get();
	printf("[nanvix][name] trunning=%.6lf nlinks=%d nunlinks=%d nlookups=%d\n",
			(stats.tshutdown - stats.tstart)/((double) sys_get_core_freq()),
			stats.nlinks, stats.nunlinks, stats.nlookups
	);

	printf("[nanvix][name] shutting down server\n");

	return (EXIT_SUCCESS);
}
