/*
 * Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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
#include <nanvix/hal.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Remote memory.
 */
static char rmem[RMEM_SIZE];

/**
 * @brief Remote memory server.
 */
int main(int argc, char **argv)
{
	int inbox;    /* Mailbox for small messages. */
	int inportal; /* Portal for data transfers.  */

	((void) argc);
	((void) argv);

#ifdef DEBUG
	printf("[RMEM] booting up server\n");
#endif

	inbox = mailbox_create("/io1");
	inportal = portal_create("/io1");

	/* Release master IO cluster. */
	barrier_open(NR_IOCLUSTER);
	barrier_release();

#ifdef DEBUG
	printf("[RMEM] server alive\n");
#endif

	while(1)
	{
		struct rmem_message msg;

		mailbox_read(inbox, &msg);

		/* Write. */
		if (msg.op == RMEM_WRITE)
		{
			portal_allow(inportal, msg.source);
			portal_read(inportal, &rmem[msg.blknum], msg.size);
		}
		/* Read. */
		else if (msg.op == RMEM_READ)
		{
			int outportal = portal_open(name_cluster_name(msg.source));
			portal_write(outportal, &rmem[msg.blknum], msg.size);
			portal_close(outportal);
		}
	}

	/* House keeping. */
	barrier_close();
	portal_unlink(inportal);
	mailbox_unlink(inbox);

	return (EXIT_SUCCESS);
}
