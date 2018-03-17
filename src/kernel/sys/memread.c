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

#include <nanvix/hal.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Initializes the remote memory.
 */
static void meminit(void)
{
	static int initialized = 0;

	/* Already initialized.  */
	if (initialized)
		return;
}

/**
 * @brief Reads from a remote memory.
 *
 * @param addr Remote address.
 * @param bug  Location where the data should be written to.
 * @param n    Number of bytes to read.
 */
void memread(uint64_t addr, void *buf, size_t n)
{
	int clusterid;            /* CLuster ID of the calling process. */
	static int outbox = -1;   /* Mailbox used for small transfers.  */
	static int inportal = -1; /* Portal used for large transfers.   */
	struct rmem_message msg;

	clusterid = arch_get_cluster_id();

	/* Open output mailbox. */
	if (outbox < 0)
		outbox = mailbox_open("/io1");

	/* Open input portal. */
	if (inportal < 0)
		inportal = portal_create(name_cluster_name(clusterid));

	/* Build operation header. */
	msg.source = clusterid;
	msg.op = RMEM_READ;
	msg.blknum = addr;
	msg.size = n;{

	/* Send operation header. */
	mailbox_write(outbox, &msg);

	/* Send data. */
	portal_allow(inportal, IOCLUSTER1);
	portal_read(inportal, buf, n);

	portal_unlink(inportal);
	mailbox_close(outbox);
}


