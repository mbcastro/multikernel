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
 * @param name Target pathname name.
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

	inbox = _mailbox_create(k1_get_cluster_id(), NAME);
	server = _mailbox_open(IOCLUSTER0, NAME);

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
	msg.id = -1;
	msg.dma = -1;
	snprintf(msg.name, ARRAY_LENGTH(msg.name), name);

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

	inbox = _mailbox_create(k1_get_cluster_id(), NAME);
	server = _mailbox_open(IOCLUSTER0, NAME);

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
	msg.id = -1;
	msg.dma = -1;
	snprintf(msg.name, ARRAY_LENGTH(msg.name), name);

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

	return (msg.dma);
}

/*=======================================================================*
 * name_lookup_pathname()                                                       *
 *=======================================================================*/

/**
 * @brief Converts a cluster ID into a pathname.
 *
 * @param name Target pathnamel name.
 *
 * @returns Upon successful completion the pathname that matches the cluster ID
 * @p clusterid is returned. Upon failure, NULL is returned instead.
 */
char *name_lookup_pathname(int clusterid)
{
	int inbox, server;         /* Mailbox for small messages. */

	#ifdef DEBUG
		printf("name_lookup_pathname(%d): Creating inbox of cluster %d...\n", clusterid, k1_get_cluster_id());
	#endif

	inbox = _mailbox_create(k1_get_cluster_id(), NAME);
	server = _mailbox_open(IOCLUSTER0, NAME);

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
	msg.id = clusterid;
	msg.dma = -1;
	strcpy(msg.name, "\0");

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

	return (msg.name);
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
	char tmp[5];

	static int ioclusters[NR_IOCLUSTER*NR_IOCLUSTER_DMA] = {
		IOCLUSTER0 + 0, IOCLUSTER0 + 1, IOCLUSTER0 + 2, IOCLUSTER0 + 3, 
		IOCLUSTER1 + 0, IOCLUSTER1 + 1, IOCLUSTER1 + 2, IOCLUSTER1 + 3
	};

	static int cclusters[NR_CCLUSTER*NR_CCLUSTER_DMA] = {
		CCLUSTER0,  CCLUSTER1,  CCLUSTER2,  CCLUSTER3,
		CCLUSTER4,  CCLUSTER5,  CCLUSTER6,  CCLUSTER7,
		CCLUSTER8,  CCLUSTER9,  CCLUSTER10, CCLUSTER11,
		CCLUSTER12, CCLUSTER13, CCLUSTER14, CCLUSTER15
	};

	remotes[0] = '\0';

	/* Append IO Clusters. */
	for (unsigned i = 0; i < ARRAY_LENGTH(ioclusters); i++)
	{
		if (local == ioclusters[i])
			continue;

		sprintf(tmp, "%d,", ioclusters[i]);
		strcat(remotes, tmp);
	}

	/* Append Compute Clusters. */
	for (unsigned i = 0; i < ARRAY_LENGTH(cclusters); i++)
	{
		if (local == cclusters[i])
			continue;

		sprintf(tmp, "%d,", cclusters[i]);
		strcat(remotes, tmp);
	}

	remotes[strlen(remotes) - 1] = '\0';
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

	#ifdef DEBUG
		printf("name_unlink(%s): opening name server mailbox from cluster %d...\n", name, k1_get_cluster_id());
	#endif

	server = _mailbox_open(IOCLUSTER0, NAME);

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_REMOVE;
	msg.id = -1;
	msg.dma = -1;
	snprintf(msg.name, ARRAY_LENGTH(msg.name), name);

	/* Send name request. */
	#ifdef DEBUG
		printf("Sending remove request for name: %s...\n", name);
	#endif

	assert(mailbox_write(server, &msg) == 0);

	/* House keeping. */
	assert(mailbox_close(server) == 0);
}
