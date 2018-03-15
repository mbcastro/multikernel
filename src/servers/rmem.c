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
	int inbox;
	int inportal;

	((void) argc);
	((void) argv);

	printf("[RMEM] booting up server\n");
	inbox = mailbox_create("/io1");
	inportal = portal_create("/io1");

	printf("[RMEM] server alive\n");
	int sync_fd = mppa_open("/mppa/sync/128:8", O_WRONLY);
	uint64_t mask = (1 << 0);
	mppa_write(sync_fd, &mask, sizeof(uint64_t));
	mppa_close(sync_fd);

	timer_init();

	while(1)
	{
		struct rmem_message msg;

		mailbox_read(inbox, &msg);
#ifdef DEBUG
		printf("[RMEM] client connected\n");
#endif
		portal_allow(inportal, msg.source);

#ifdef DEBUG
		printf("[RMEM] serving client\n");
#endif
		portal_read(inportal, &rmem[msg.blknum], msg.size);

#ifdef DEBUG
		printf("[RMEM] client disconnected\n");
#endif
	}

	portal_unlink(inportal);
	mailbox_unlink(inbox);

	return (EXIT_SUCCESS);
}
