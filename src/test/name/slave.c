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

int msg;

/*====================================================================*
 * main                                                               *
 *====================================================================*/

/**
 * @brief Remote name unit test.
 */
int main(int argc, char **argv)
{
  char pathname[50];
  char out_pathname[50];
  char process_name[50];             /* Process name */
  int barrier;
  int clusterid;
  int nclusters;
  int inbox;
  int outbox;

  clusterid = k1_get_cluster_id();

  /* Retrieve parameters. */
	assert(argc == 2);
	assert((nclusters = atoi(argv[1])) > 0);

  barrier = barrier_open(nclusters);

  sprintf(pathname, "/cpu%d", clusterid);
  sprintf(process_name, "process_on_cpu%d", clusterid);

  /* Primitives test */

  /* Ask for an unregistered entry */
  assert(name_cluster_id(pathname) == -2);
  assert(name_cluster_dma(pathname) == -2);
  assert(!strcmp(id_cluster_name(clusterid), " "));
  assert(!strcmp(id_process_name(clusterid), " "));

  /* Register this cluster */
  register_name(clusterid, pathname, process_name);

  /* Ask for a registered entry */
  assert(name_cluster_id(pathname) == clusterid);
  assert(name_cluster_dma(pathname) == clusterid);
  assert(!strcmp(id_cluster_name(clusterid), pathname));
  assert(!strcmp(id_process_name(clusterid), process_name));

  /* Remove the entry */
  remove_name(pathname);

  /* Verify that the entry is removed */
  assert(name_cluster_id(pathname) == -2);
  assert(name_cluster_dma(pathname) == -2);
  assert(!strcmp(id_cluster_name(clusterid), " "));
  assert(!strcmp(id_process_name(clusterid), " "));

  /* Register this cluster */
  register_name(clusterid, pathname, process_name);

  /* Wait for others clusters */
  barrier_wait(barrier);

  /* Message exchange test usign name resolution */

  inbox = mailbox_create(pathname);
  sprintf(out_pathname, "/cpu%d", (clusterid + 1)%nclusters);

  printf("Sending message to %s from %s...\n", out_pathname, pathname);

  outbox = mailbox_open(out_pathname);

  msg = clusterid;
  assert(mailbox_write(outbox, &msg) == 0);

  msg = -1;
  while(msg == -1){
    assert(mailbox_read(inbox, &msg) == 0);
  }

  printf("Message from /cpu%d received by %s.\n", msg, pathname);

  /* House keeping. */
  assert(mailbox_close(outbox) == 0);
  assert(mailbox_close(inbox) == 0);
  barrier_close(barrier);

	return (EXIT_SUCCESS);
}
