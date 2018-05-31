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

#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "kernel.h"

/*====================================================================*
 * kernel                                                             *
 *====================================================================*/

/**
 * @brief Send several messages through mailboxes.
 *
 * @param nmessages Number of messages.
 */
static void kernel(int nmessages)
{
	int inbox;
	int outbox;
	int clusterid;

	clusterid = k1_get_cluster_id();

	/* Open mailboxes. */
	inbox = mailbox_create(clusterid);
	outbox = mailbox_open(IOCLUSTER0);

	/* Send messages. */
	for (int i = 0; i < nmessages; i++)
	{
		struct message msg;
		msg.magic = MESSAGE_MAGIC;
		mailbox_write(outbox, &msg);
	}

	/* Receive messages. */
	for (int i = 0; i < nmessages; i++)
	{
		struct message msg;
		mailbox_read(inbox, &msg);
		assert(msg.magic == MESSAGE_MAGIC);
	}

	/* House keeping. */
	mailbox_close(outbox);
	mailbox_unlink(inbox);
}

/*====================================================================*
 * main                                                               *
 *====================================================================*/

/**
 * @brief Mailbox unit test.
 */
int main(int argc, char **argv)
{
	int nmessages;

	/* Retrieve parameters. */
	assert(argc == 2);
	assert((nmessages = atoi(argv[1])));

	kernel(nmessages);

	return (EXIT_SUCCESS);
}
