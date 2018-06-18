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

	clusterid = hal_get_cluster_id();

	/* Open mailboxes. */
	inbox = hal_mailbox_create(clusterid);
	outbox =hal_mailbox_open(IOCLUSTER0);

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
