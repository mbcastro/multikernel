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

static struct name_message msg;

/*=======================================================================*
 * name_cluster_id()                                                     *
 *=======================================================================*/

/**
 * @brief Converts a pathname name into a cluster ID.
 *
 * @param name 		Target pathname name.
 *
 * @returns Upon successful completion the cluster ID whose name is @p
 * name is returned. Upon failure, a negative error code is returned
 * instead.
 */
int name_cluster_id(char *name)
{
	int server;         /* Mailbox for small messages. */
	int inbox;

	/* Sanity check. */
	assert((name != NULL) && (strlen(name) < (PROC_NAME_MAX - 1)));

	#ifdef DEBUG
		printf("name_cluster_id(%s): Creating inbox of cluster %d...\n", name, k1_get_cluster_id());
	#endif

	inbox = _mailbox_create(k1_get_cluster_id(), NAME);
	server = _mailbox_open(IOCLUSTER0, NAME);

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
	msg.id = -1;
	msg.dma = -1;
	strcpy(msg.name, name);

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

	return (msg.id);
}

/*=======================================================================*
 * name_cluster_dma()                                                    *
 *=======================================================================*/

/**
 * @brief Converts a pathnamel name into a DMA channel number.
 *
 * @param name 		Target pathnamel name.
 *
 * @returns Upon successful completion the DMA number whose name is @p
 * name is returned. Upon failure, a negative error code is returned
 * instead.
 */
int name_cluster_dma(char *name)
{
	int server;         /* Mailbox for small messages. */
	int inbox;

	/* Sanity check. */
	assert((name != NULL) && (strlen(name) < (PROC_NAME_MAX - 1)));

	#ifdef DEBUG
		printf("name_cluster_dma(%s): Creating inbox of cluster %d...\n", name, k1_get_cluster_id());
	#endif

	inbox = _mailbox_create(k1_get_cluster_id(), NAME);
	server = _mailbox_open(IOCLUSTER0, NAME);

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
	msg.id = -1;
	msg.dma = -1;
	strcpy(msg.name, name);

	/* Send name request. */
	#ifdef DEBUG
		printf("Sending request for name: %s...\n", msg.name);
	#endif

	assert(mailbox_write(server, &msg) == 0);

	while(msg.id == -1){
		assert(mailbox_read(inbox, &msg) == 0);
	}

	/* House keeping. */
	assert(mailbox_close(server) == 0);
	assert(mailbox_close(inbox) == 0);

	return (msg.dma);
}

/*=======================================================================*
 * name_lookup_pathname()                                                       *
 *=======================================================================*/

/**
 * @brief Converts a cluster ID into a pathname.
 *
 * @param clusterid 	Target cluster ID.
 * @param name 			Address where the result will be written.
 *
 * @returns Upon successful completion 0 is returned. Upon failure, negative
 * error code is returned.
 */
int name_lookup_pathname(int dma, char *name)
{
	int inbox; 			/* Mailbox for small messages. */
	int server;

	/* Sanity check. */
	assert(dma >= 0);
	assert((name != NULL) && (strlen(name) < (PROC_NAME_MAX - 1)));

	#ifdef DEBUG
		printf("name_lookup_pathname(%d): Creating inbox of cluster %d...\n", dma, k1_get_cluster_id());
	#endif

	inbox = _mailbox_create(k1_get_cluster_id(), NAME);
	server = _mailbox_open(IOCLUSTER0, NAME);

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
	msg.id = -1;
	msg.dma = dma;
	strcpy(msg.name, "\0");

	/* Send name request. */
	#ifdef DEBUG
		printf("Sending request for ID: %d...\n", dma);
	#endif

	assert(mailbox_write(server, &msg) == 0);

	while(msg.id == -1){
		assert(mailbox_read(inbox, &msg) == 0);
	}

	/* House keeping. */
	assert(mailbox_close(server) == 0);
	assert(mailbox_close(inbox) == 0);

	if(!strcmp(msg.name, "\0"))
	{
		return (-ENOENT);
	}
	else
	{
		strcpy(name, msg.name);
		return (0);
	}
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
 * name_link()                                                       *
 *=======================================================================*/

/**
 * @brief Register a process name
 *
 * @param dma       Target DMA channel.
 * @param name      Portal name.
 */
void name_link(int dma, const char *name)
{
	int server;

	/* Sanity check. */
	assert(dma >= 0);
	assert((name != NULL) && (strlen(name) < (PROC_NAME_MAX - 1)));

	server = _mailbox_open(IOCLUSTER0, NAME);

	/* Build request message. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_ADD;
	msg.dma = dma;
	strcpy(msg.name, name);

	/* Send name request. */
	assert(mailbox_write(server, &msg) == 0);

	/* House keeping. */
	assert(mailbox_close(server) == 0);
}

/*=======================================================================*
 * name_unlink()                                                        *
 *=======================================================================*/

/**
 * @brief Remove a process name
 *
 * @param name	Portal name.
 */
void name_unlink(char *name)
{
	int server;         /* Mailbox for small messages. */

	/* Sanity check. */
	assert((name != NULL) && (strlen(name) < (PROC_NAME_MAX - 1)));

	#ifdef DEBUG
		printf("name_unlink(%s): opening name server mailbox from cluster %d...\n", name, k1_get_cluster_id());
	#endif

	server = _mailbox_open(IOCLUSTER0, NAME);

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_REMOVE;
	msg.id = -1;
	msg.dma = -1;
	strcpy(msg.name, name);

	/* Send name request. */
	#ifdef DEBUG
		printf("Sending remove request for name: %s...\n", msg.name);
	#endif

	assert(mailbox_write(server, &msg) == 0);

	/* House keeping. */
	assert(mailbox_close(server) == 0);
}
