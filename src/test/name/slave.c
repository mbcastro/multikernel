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
  // struct name_message msg;
  // int inbox, server;         /* Mailbox for small messages. */
  char pathname[15];
  char process_name[50];             /* Process name */

//   printf("Creating inbox of cluster %d...\n", k1_get_cluster_id());
//   inbox = mailbox_create(k1_get_cluster_id());
//   server = mailbox_open(IOCLUSTER0);
//
//   /* Ask for a unregistered name */
//
// 	/* Build operation header. */
// 	msg.source = k1_get_cluster_id();
// 	msg.op = NAME_QUERY;
//   msg.id = -1;
//   msg.dma = -1;
// 	sprintf(msg.name, "/cpu%d", k1_get_cluster_id());
//   sprintf(msg.process_name, "_");
//
//   /* Send name request. */
//   printf("Sending request for /cpu%d...\n", k1_get_cluster_id());
// 	assert(mailbox_write(server, &msg) == 0);
//
//   while(msg.id == -1){
//     assert(mailbox_read(inbox, &msg) == 0);
//   }
//   printf("Before registration [op: %d, name: %s, process name: %s, id: %d, dma: %d]\n", msg.op, msg.name, msg.process_name, msg.id, msg.dma);
//
//   /* Add name */
//
//   /* Build operation header. */
// 	msg.source = k1_get_cluster_id();
// 	msg.op = NAME_ADD;
//   msg.id = k1_get_cluster_id();
//   msg.dma = k1_get_cluster_id();
// 	sprintf(msg.name, "/cpu%d", k1_get_cluster_id());
//   sprintf(msg.process_name, "process_on_cpu%d", k1_get_cluster_id());
//
//   /* Send name request. */
//   printf("Sending add request for /cpu%d...\n", k1_get_cluster_id());
// 	assert(mailbox_write(server, &msg) == 0);
//
//   /* Ask for a registered name */
//
// 	/* Build operation header. */
// 	msg.source = k1_get_cluster_id();
// 	msg.op = NAME_QUERY;
//   msg.id = -1;
//   msg.dma = -1;
// 	sprintf(msg.name, "/cpu%d", k1_get_cluster_id());
//   sprintf(msg.process_name, "_");
//
//   /* Send name request. */
//   printf("Sending request for /cpu%d...\n", k1_get_cluster_id());
// 	assert(mailbox_write(server, &msg) == 0);
//
//   while(msg.id == -1){
//     assert(mailbox_read(inbox, &msg) == 0);
//   }
//   printf("After registration [op: %d, name: %s, process name: %s, id: %d, dma: %d]\n", msg.op, msg.name, msg.process_name, msg.id, msg.dma);
//
//   /* Remove name */
//
//   /* Build operation header. */
// 	msg.source = k1_get_cluster_id();
// 	msg.op = NAME_REMOVE;
//   msg.id = -1;
//   msg.dma = -1;
// 	sprintf(msg.name, "/cpu%d", k1_get_cluster_id());
//   sprintf(msg.process_name, " ");
//
//   /* Send name request. */
//   printf("Sending remove request for /cpu%d...\n", k1_get_cluster_id());
// 	assert(mailbox_write(server, &msg) == 0);
//
//   /* Ask for a deleted name */
//
// 	/* Build operation header. */
// 	msg.source = k1_get_cluster_id();
// 	msg.op = NAME_QUERY;
//   msg.id = -1;
//   msg.dma = -1;
// 	sprintf(msg.name, "/cpu%d", k1_get_cluster_id());
//   sprintf(msg.process_name, "_");
//
//   /* Send name request. */
//   printf("Sending request for /cpu%d...\n", k1_get_cluster_id());
// 	assert(mailbox_write(server, &msg) == 0);
//
//   while(msg.id == -1){
//     assert(mailbox_read(inbox, &msg) == 0);
//   }
//   printf("After deletion [op: %d, name: %s, process name: %s, id: %d, dma: %d]\n", msg.op, msg.name, msg.process_name, msg.id, msg.dma);
  //
  /* Primitives test */

  sprintf(pathname, "/cpu%d", k1_get_cluster_id());
  sprintf(process_name, "process_on_cpu%d", k1_get_cluster_id());

  printf("name_cluster_id(%s) call before registration, id: %d.\n", pathname, name_cluster_id(pathname));
  printf("name_cluster_dma(%s) call before registration, dma: %d.\n", pathname, name_cluster_dma(pathname));
  printf("id_cluster_name(%d) call before registration, name: %s.\n", k1_get_cluster_id(), id_cluster_name(k1_get_cluster_id()));
  printf("id_process_name(%d) call before registration, name: %s.\n", k1_get_cluster_id(), id_process_name(k1_get_cluster_id()));
  register_name(k1_get_cluster_id(), pathname, process_name);
  printf("name_cluster_id(%s) call after registration, id: %d.\n", pathname, name_cluster_id(pathname));
  printf("name_cluster_dma(%s) call after registration, dma: %d.\n", pathname, name_cluster_dma(pathname));
  printf("id_cluster_name(%d) call after registration, name: %s.\n", k1_get_cluster_id(), id_cluster_name(k1_get_cluster_id()));
  printf("id_process_name(%d) call after registration, name: %s.\n", k1_get_cluster_id(), id_process_name(k1_get_cluster_id()));
  remove_name(pathname);
  printf("name_cluster_id(%s) call after deletion, id: %d.\n", pathname, name_cluster_id(pathname));
  printf("name_cluster_dma(%s) call after deletion, dma: %d.\n", pathname, name_cluster_dma(pathname));
  printf("id_cluster_name(%d) call after deletion, name: %s.\n", k1_get_cluster_id(), id_cluster_name(k1_get_cluster_id()));
  printf("id_process_name(%d) call after deletion, name: %s.\n", k1_get_cluster_id(), id_process_name(k1_get_cluster_id()));

  if(k1_get_cluster_id() != 0)
    return (EXIT_SUCCESS);

  /* IO cluster registration test */
	for(int i = 0; i < NR_IOCLUSTER_DMA; i++){
		sprintf(pathname, "/name%d", i);
		sprintf(process_name, "name-test%d", i);
    remove_name(pathname);
    printf("register_name(%d, %s, %s)\n", IOCLUSTER1 + i, pathname, process_name);
	  register_name(IOCLUSTER1 + i, pathname, process_name);
    
		printf("name_cluster_id(%s) id: %d.\n", pathname, name_cluster_id(pathname));
	  printf("name_cluster_dma(%s) dma: %d.\n", pathname, name_cluster_dma(pathname));
	  printf("id_cluster_name(%d) name: %s.\n", IOCLUSTER1 + i, id_cluster_name(IOCLUSTER1 + i));
	  printf("id_process_name(%d) name: %s.\n", IOCLUSTER1 + i, id_process_name(IOCLUSTER1 + i));
	}

	return (EXIT_SUCCESS);
}
