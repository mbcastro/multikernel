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
#include <nanvix/name.h>
#include <nanvix/pm.h>
#include <nanvix/klib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct name_message msg;

/*=======================================================================*
 * name_cluster_id()                                                     *
 *=======================================================================*/

/**
 * @brief Converts a pathnamel name into a cluster ID.
 *
 * @param name Target pathnamel name.
 *
 * @returns Upon successful completion the cluster ID whose name is @p
 * name is returned. Upon failure, a negative error code is returned
 * instead.
 */
int name_cluster_id(char *name)
{
  int inbox, server;         /* Mailbox for small messages. */

  #ifdef DEBUG
    printf("name_cluster_id(%s): Creating inbox of cluster %d...\n", name, k1_get_cluster_id());
  #endif

  inbox = mailbox_create(k1_get_cluster_id(), NAME);
  server = mailbox_open(IOCLUSTER0, NAME);

  /* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
  msg.id = -1;
  msg.dma = -1;
	snprintf(msg.name, ARRAY_LENGTH(msg.name), name);
  snprintf(msg.process_name, ARRAY_LENGTH(msg.process_name), " ");

  /* Send name request. */
  #ifdef DEBUG
    printf("Sending request for name: %s...\n", name);
  #endif

  assert(mailbox_write(server, &msg) == 0);

  while(msg.id == -1){
    assert(mailbox_read(inbox, &msg) == 0);
  }

  /* House keeping. */
  assert(mailbox_close(server) == 0);
  assert(mailbox_close(inbox) == 0);

	return msg.id;
}

/*=======================================================================*
 * name_cluster_dma()                                                    *
 *=======================================================================*/

/**
 * @brief Converts a pathnamel name into a DMA channel id.
 *
 * @param name Target pathnamel name.
 *
 * @returns Upon successful completion the DMA ID whose name is @p
 * name is returned. Upon failure, a negative error code is returned
 * instead.
 */
int name_cluster_dma(char *name)
{
  int inbox, server;         /* Mailbox for small messages. */

  #ifdef DEBUG
    printf("name_cluster_dma(%s): Creating inbox of cluster %d...\n", name, k1_get_cluster_id());
  #endif

  inbox = mailbox_create(k1_get_cluster_id(), NAME);
  server = mailbox_open(IOCLUSTER0, NAME);

  /* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
  msg.id = -1;
  msg.dma = -1;
	snprintf(msg.name, ARRAY_LENGTH(msg.name), name);
  snprintf(msg.process_name, ARRAY_LENGTH(msg.process_name), " ");

  /* Send name request. */
  #ifdef DEBUG
    printf("Sending request for name: %s...\n", name);
  #endif

  assert(mailbox_write(server, &msg) == 0);

  while(msg.id == -1){
    assert(mailbox_read(inbox, &msg) == 0);
  }

  /* House keeping. */
  assert(mailbox_close(server) == 0);
  assert(mailbox_close(inbox) == 0);

	return msg.dma;
}

/*=======================================================================*
 * name_lookdown()                                                       *
 *=======================================================================*/

/**
 * @brief Converts a cluster ID into a pathname.
 *
 * @param name Target pathnamel name.
 *
 * @returns Upon successful completion the pathname that matches the cluster ID
 * @p clusterid is returned. Upon failure, NULL is returned instead.
 */
char *id_cluster_name(int clusterid)
{
  int inbox, server;         /* Mailbox for small messages. */

  #ifdef DEBUG
    printf("id_cluster_name(%d): Creating inbox of cluster %d...\n", clusterid, k1_get_cluster_id());
  #endif

  inbox = mailbox_create(k1_get_cluster_id(), NAME);
  server = mailbox_open(IOCLUSTER0, NAME);

  /* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
  msg.id = clusterid;
  msg.dma = -1;
	snprintf(msg.name, ARRAY_LENGTH(msg.name), " ");
  snprintf(msg.process_name, ARRAY_LENGTH(msg.process_name), " ");

  /* Send name request. */
  #ifdef DEBUG
    printf("Sending request for ID: %d...\n", clusterid);
  #endif

  assert(mailbox_write(server, &msg) == 0);

  while(msg.dma == -1){
    assert(mailbox_read(inbox, &msg) == 0);
  }

  /* House keeping. */
  assert(mailbox_close(server) == 0);
  assert(mailbox_close(inbox) == 0);

	return msg.name;
}

/*=======================================================================*
 * id_process_name()                                                      *
 *=======================================================================*/

/**
 * @brief Converts a cluster ID into a process name.
 *
 * @param name Target process name.
 *
 * @returns Upon successful completion the process name that matches the cluster ID.
 * Upon failure, NULL is returned instead.
 */
char *id_process_name(int clusterid)
{
  int inbox, server;         /* Mailbox for small messages. */

  #ifdef DEBUG
    printf("id_process_name(%d): Creating inbox of cluster %d...\n", clusterid, k1_get_cluster_id());
  #endif

  inbox = mailbox_create(k1_get_cluster_id(), NAME);
  server = mailbox_open(IOCLUSTER0, NAME);

  /* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
  msg.id = clusterid;
  msg.dma = -1;
  snprintf(msg.name, ARRAY_LENGTH(msg.name), " ");
  snprintf(msg.process_name, ARRAY_LENGTH(msg.process_name), " ");

  /* Send name request. */
  #ifdef DEBUG
    printf("Sending request for ID: %d...\n", clusterid);
  #endif

  assert(mailbox_write(server, &msg) == 0);

  while(msg.dma == -1){
    assert(mailbox_read(inbox, &msg) == 0);
  }

  /* House keeping. */
  assert(mailbox_close(server) == 0);
  assert(mailbox_close(inbox) == 0);

	return msg.process_name;
}

/*=======================================================================*
 * name_remotes()                                                        *
 *=======================================================================*/

/**
 * @brief Builds a list of remotes.
 *
 * @param remotes List of IDs of remote clusters.
 * @param local   ID of local cluster.
 */
void name_remotes(char *remotes, int local)
{
	if ((local >= IOCLUSTER0) && (local < (IOCLUSTER0 + NR_IOCLUSTER_DMA)))
	{
		sprintf(remotes,
				"%d..%d,%d",
				CCLUSTER0, CCLUSTER15, IOCLUSTER1
		);
	}
	else if ((local >= IOCLUSTER1) && (local < (IOCLUSTER1 + NR_IOCLUSTER_DMA)))
	{
		sprintf(remotes,
				"%d..%d,%d",
				CCLUSTER0, CCLUSTER15, IOCLUSTER0
		);
	}
	else if (local == CCLUSTER0)
	{
		sprintf(remotes,
				"%d..%d,%d,%d",
				CCLUSTER1, CCLUSTER15, IOCLUSTER0, IOCLUSTER1
		);
	}
	else if (local  == CCLUSTER15)
	{
		sprintf(remotes,
				"%d..%d,%d,%d",
				CCLUSTER0, CCLUSTER14, IOCLUSTER0, IOCLUSTER1
		);
	}
	else
	{
		sprintf(remotes,
				"%d..%d,%d..%d,%d,%d",
				CCLUSTER0, local - 1, local + 1, CCLUSTER15, IOCLUSTER0, IOCLUSTER1
		);
	}
}

/*=======================================================================*
 * register_name()                                                        *
 *=======================================================================*/

/**
 * @brief Register a process name
 *
 * @param DMA		DMA channel.
 * @param name	Portal name.
 * @param process_name	Process name.
 */
void register_name(int dma, char *name, char *process_name)
{
  int server;         /* Mailbox for small messages. */

  #ifdef DEBUG
    printf("register_name(%s): opening name server mailbox from cluster %d...\n", name, k1_get_cluster_id());
  #endif

  server = mailbox_open(IOCLUSTER0, NAME);

  /* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_ADD;
  msg.dma = dma;
	snprintf(msg.name, ARRAY_LENGTH(msg.name), name);
  snprintf(msg.process_name, ARRAY_LENGTH(msg.process_name), process_name);

  /* Send name request. */
  #ifdef DEBUG
    printf("Sending add request for name: %s...\n", name);
  #endif

  assert(mailbox_write(server, &msg) == 0);

  /* House keeping. */
  assert(mailbox_close(server) == 0);
}

/*=======================================================================*
 * remove_name()                                                        *
 *=======================================================================*/

/**
 * @brief Remove a process name
 *
 * @param name	Portal name.
 */
void remove_name(char *name)
{
  int server;         /* Mailbox for small messages. */

  #ifdef DEBUG
    printf("remove_name(%s): opening name server mailbox from cluster %d...\n", name, k1_get_cluster_id());
  #endif

  server = mailbox_open(IOCLUSTER0, NAME);

  /* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_REMOVE;
  msg.id = -1;
  msg.dma = -1;
  snprintf(msg.name, ARRAY_LENGTH(msg.name), name);
  snprintf(msg.process_name, ARRAY_LENGTH(msg.process_name), " ");

  /* Send name request. */
  #ifdef DEBUG
    printf("Sending remove request for name: %s...\n", name);
  #endif

	assert(mailbox_write(server, &msg) == 0);

  /* House keeping. */
  assert(mailbox_close(server) == 0);
}
