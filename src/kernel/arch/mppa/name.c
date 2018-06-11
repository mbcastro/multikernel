/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#include <nanvix/name.h>
#include <nanvix/pm.h>
#include <nanvix/klib.h>
#include <nanvix/hal.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mppa.h"

/**
 * @brief name server node ID.
 */
#define SERVER IOCLUSTER0

/**
 * @brief name server message.
 */
static struct name_message msg;

/*=======================================================================*
 * name_lookup()                                                         *
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
int name_lookup(char *name)
{
	int server;         /* Mailbox for small messages. */
	int inbox;

	/* Sanity check. */
	assert((name != NULL) && (strlen(name) < (PROC_NAME_MAX - 1))
	                                && (strcmp(name, "\0") != 0));

	#ifdef DEBUG
		printf("name_lookup(%s) called from cluster %d...\n", name,
		                                      hal_get_cluster_id());
	#endif

	inbox = hal_mailbox_create(hal_get_cluster_id());
	server = hal_mailbox_open(SERVER);

	/* Build operation header. */
	msg.source = hal_get_cluster_id();
	msg.op = NAME_LOOKUP;
	msg.nodeid = -1;
	strcpy(msg.name, name);

	/* Send name request. */
	#ifdef DEBUG
		printf("Sending request for name: %s...\n", msg.name);
	#endif

	assert(hal_mailbox_write(server, &msg, MAILBOX_MSG_SIZE)
	                                   == MAILBOX_MSG_SIZE);

	while(msg.nodeid == -1){
		assert(hal_mailbox_read(inbox, &msg, MAILBOX_MSG_SIZE)
		                                 == MAILBOX_MSG_SIZE);
	}

	/* House keeping. */
	assert(hal_mailbox_close(server) == 0);
	assert(hal_mailbox_close(inbox) == 0);

	return (msg.nodeid);
}

/*=======================================================================*
 * name_link()                                                           *
 *=======================================================================*/

/**
 * @brief link a process name.
 *
 * @param nodeid    NoC node ID of the process to link.
 * @param name      Name of the process to link.
 */
void name_link(int nodeid, const char *name)
{
	int server;        /* Mailbox for small messages. */

	/* Sanity check. */
	assert(nodeid >= 0);
	assert((name != NULL) && (strlen(name) < (PROC_NAME_MAX - 1))
                                   && (strcmp(name, "\0") != 0));

	server =hal_mailbox_open(SERVER);

	/* Build operation header. */
	msg.source = hal_get_cluster_id();
	msg.op = NAME_ADD;
	msg.nodeid = nodeid;
	strcpy(msg.name, name);

	/* Send link request. */
	assert(hal_mailbox_write(server, &msg, MAILBOX_MSG_SIZE)
	                                   == MAILBOX_MSG_SIZE);

	/* House keeping. */
	assert(hal_mailbox_close(server) == 0);
}

/*=======================================================================*
 * name_unlink()                                                         *
 *=======================================================================*/

/**
 * @brief Unlink a process name.
 *
 * @param name	    Name of the process to unlink.
 */
void name_unlink(char *name)
{
	int server;         /* Mailbox for small messages. */

	/* Sanity check. */
	assert((name != NULL) && (strlen(name) < (PROC_NAME_MAX - 1))
								   && (strcmp(name, "\0") != 0));

	#ifdef DEBUG
		printf("name_unlink(%s): called from cluster %d...\n",
		                          name, hal_get_cluster_id());
	#endif

	server =hal_mailbox_open(SERVER);

	/* Build operation header. */
	msg.source = hal_get_cluster_id();
	msg.op = NAME_REMOVE;
	msg.nodeid = -1;
	strcpy(msg.name, name);

	/* Send name request. */
	#ifdef DEBUG
		printf("Sending remove request for name: %s...\n", msg.name);
	#endif

	assert(hal_mailbox_write(server, &msg, MAILBOX_MSG_SIZE)
	                                    == MAILBOX_MSG_SIZE);

	/* House keeping. */
	assert(hal_mailbox_close(server) == 0);
}
