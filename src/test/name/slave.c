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
  int inbox, server;         /* Mailbox for small messages. */

  printf("Creating inbox of cluster %d...\n", k1_get_cluster_id());
  inbox = mailbox_create(k1_get_cluster_id());
  server = mailbox_open(IOCLUSTER0);

  /* Ask for a unregistered name */

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
  msg.id = -1;
  msg.dma = -1;
	sprintf(msg.name, "/cpu%d", k1_get_cluster_id());
  sprintf(msg.process_name, "_");

  /* Send name request. */
  printf("Sending request for /cpu%d...\n", k1_get_cluster_id());
	assert(mailbox_write(server, &msg) == 0);

  while(msg.id == -1){
    assert(mailbox_read(inbox, &msg) == 0);
  }
  printf("Before registration [op: %d, name: %s, process name: %s, id: %d, dma: %d]\n", msg.op, msg.name, msg.process_name, msg.id, msg.dma);

  /* Add name */

  /* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_ADD;
  msg.id = k1_get_cluster_id();
  msg.dma = k1_get_cluster_id();
	sprintf(msg.name, "/cpu%d", k1_get_cluster_id());
  sprintf(msg.process_name, "process_on_cpu%d", k1_get_cluster_id());

  /* Send name request. */
  printf("Sending add request for /cpu%d...\n", k1_get_cluster_id());
	assert(mailbox_write(server, &msg) == 0);

  /* Ask for a registered name */

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
  msg.id = -1;
  msg.dma = -1;
	sprintf(msg.name, "/cpu%d", k1_get_cluster_id());
  sprintf(msg.process_name, "_");

  /* Send name request. */
  printf("Sending request for /cpu%d...\n", k1_get_cluster_id());
	assert(mailbox_write(server, &msg) == 0);

  while(msg.id == -1){
    assert(mailbox_read(inbox, &msg) == 0);
  }
  printf("After registration [op: %d, name: %s, process name: %s, id: %d, dma: %d]\n", msg.op, msg.name, msg.process_name, msg.id, msg.dma);

  /* Remove name */

  /* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_REMOVE;
  msg.id = -1;
  msg.dma = -1;
	sprintf(msg.name, "/cpu%d", k1_get_cluster_id());
  sprintf(msg.process_name, " ");

  /* Send name request. */
  printf("Sending remove request for /cpu%d...\n", k1_get_cluster_id());
	assert(mailbox_write(server, &msg) == 0);

  /* Ask for a deleted name */

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
  msg.id = -1;
  msg.dma = -1;
	sprintf(msg.name, "/cpu%d", k1_get_cluster_id());
  sprintf(msg.process_name, "_");

  /* Send name request. */
  printf("Sending request for /cpu%d...\n", k1_get_cluster_id());
	assert(mailbox_write(server, &msg) == 0);

  while(msg.id == -1){
    assert(mailbox_read(inbox, &msg) == 0);
  }
  printf("After deletion [op: %d, name: %s, process name: %s, id: %d, dma: %d]\n", msg.op, msg.name, msg.process_name, msg.id, msg.dma);

	return (EXIT_SUCCESS);
}
