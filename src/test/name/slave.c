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

#include <nanvix/arch/mppa.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/*====================================================================*
 * main                                                               *
 *====================================================================*/

/**
 * @brief Remote name unit test.
 */
int main()
{
  struct name_message msg;
  char pathname[16];
  int inbox, server;         /* Mailbox for small messages. */

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
  msg.id = -1;
  msg.dma = -1;
	sprintf(msg.name, "/cpu3");
  sprintf(msg.process_name, "_");

  printf("Creating inbox of cluster %d...\n", k1_get_cluster_id());
  sprintf(pathname, "/cpu%d", k1_get_cluster_id());
	inbox = mailbox_create(pathname);
  server = mailbox_open("/io0");

  /* Send name request. */
  printf("Sending request for /cpu3...\n");
	assert(mailbox_write(server, &msg) == 0);

  while(msg.id == -1){
    assert(mailbox_read(inbox, &msg) == 0);
  }
  printf("Server response = [op: %d, name: %s, process name: %s, id: %d, dma: %d]\n", msg.op, msg.name, msg.process_name, msg.id, msg.dma);

	return (EXIT_SUCCESS);
}
