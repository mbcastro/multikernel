/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdio.h>

#include <HAL/hal/core/mp.h>
#include <nanvix/hal.h>

#include <nanvix/klib.h>

#include "mppa.h"

/**
 * @brief NoC tags offsets.
 *
 * @detail All NoC connectors that are listed bellow support 1:N
 * single-direction communication. Therefore, we need HAL_NR_NOC_NODES
 * NoC tags for each. The first two tags are used by the hardware and
 * thus are skipped.
 */
/**@{*/
#define NOCTAG_MAILBOX_OFF 5                                      /**< Mailbox. */
#define NOCTAG_PORTAL_OFF (NOCTAG_MAILBOX_OFF + HAL_NR_NOC_NODES) /**< Portal.  */
#define NOCTAG_SYNC_OFF   (NOCTAG_PORTAL_OFF + HAL_NR_NOC_NODES)  /**< Sync.    */
/**@}*/

/**
 * @brief IDs of NoC nodes.
 */
const int hal_noc_nodes[HAL_NR_NOC_NODES] = {
	IOCLUSTER0 + 0,
	IOCLUSTER0 + 1,
	IOCLUSTER0 + 2,
	IOCLUSTER0 + 3,
	IOCLUSTER1 + 0,
	IOCLUSTER1 + 1,
	IOCLUSTER1 + 2,
	IOCLUSTER1 + 3,
	CCLUSTER0,
	CCLUSTER1,
	CCLUSTER2,
	CCLUSTER3,
	CCLUSTER4,
	CCLUSTER5,
	CCLUSTER6,
	CCLUSTER7,
	CCLUSTER8,
	CCLUSTER9,
	CCLUSTER10,
	CCLUSTER11,
	CCLUSTER12,
	CCLUSTER13,
	CCLUSTER14,
	CCLUSTER15
};

/*============================================================================*
 * hal_get_node_id()                                                          *
 *============================================================================*/

/**
 * @brief Gets the ID of the NoC node attached to the underlying core.
 *
 * @returns The ID of the NoC node attached to the underlying core is
 * returned.
 */
int hal_get_node_id(void)
{
	return (__k1_get_cluster_id() + hal_get_core_id());
}

/*============================================================================*
 * hal_get_node_num()                                                          *
 *============================================================================*/

/**
 * @brief Gets the logic number of a NoC node.
 *
 * @param nodeid ID of the target NoC node.
 *
 * @returns The logic number of the target NoC node.
 */
int noc_get_node_num(int nodeid)
{
	/* Lookup table of NoC node IDs. */
	for (int i = 1; i < HAL_NR_NOC_NODES; i++)
	{
		/* Found. */
		if (nodeid == hal_noc_nodes[i])
			return (i);
	}

	return (0);
}

/*============================================================================*
 * noc_get_dma()                                                              *
 *============================================================================*/

/**
 * @brief Gets the DMA channel of a NoC node.
 *
 * @param nodeid ID of the target NoC node.
 */
int noc_get_dma(int nodeid)
{
	return (noc_is_cnode(nodeid) ?
			nodeid%NR_CCLUSTER_DMA : nodeid%NR_IOCLUSTER_DMA);
}

/*============================================================================*
 * noc_is_ionode0()                                                            *
 *============================================================================*/

/**
 * @brief Asserts whether a NoC node is attached to IO cluster 0.
 *
 * @param nodeid ID of the target NoC node.
 *
 * @returns One if the target NoC node is attached to IO cluster 0,
 * and zero otherwise.
 */
int noc_is_ionode0(int nodeid)
{
	return ((nodeid >= IOCLUSTER0) && (nodeid < IOCLUSTER0 + 4));
}

/*============================================================================*
 * noc_is_ionode1()                                                            *
 *============================================================================*/

/**
 * @brief Asserts whether a NoC node is attached to IO cluster 1.
 *
 * @param nodeid ID of the target NoC node.
 *
 * @returns One if the target NoC node is attached to IO cluster 1,
 * and zero otherwise.
 */
int noc_is_ionode1(int nodeid)
{
	return ((nodeid >= IOCLUSTER1) && (nodeid < IOCLUSTER1 + 4));
}

/*============================================================================*
 * noc_is_ionode()                                                            *
 *============================================================================*/

/**
 * @brief Asserts whether a NoC node is attached to an IO cluster.
 *
 * @param nodeid ID of the target NoC node.
 *
 * @returns One if the target NoC node is attached to an IO cluster,
 * and zero otherwise.
 */
int noc_is_ionode(int nodeid)
{
	return (noc_is_ionode0(nodeid) || noc_is_ionode1(nodeid));
}

/*============================================================================*
 * noc_is_cnode()                                                             *
 *============================================================================*/

/**
 * @brief Asserts whether a NoC node is attached to a compute cluster.
 *
 * @param nodeid ID of the target NoC node.
 *
 * @returns One if the target NoC node is attached to a compute
 * cluster, and zero otherwise.
 */
int noc_is_cnode(int nodeid)
{
	return ((nodeid >= CCLUSTER0) && (nodeid <= CCLUSTER15));
}

/*=======================================================================*
 * noc_get_names()                                                       *
 *=======================================================================*/

/**
 * @brief Gets the name of NoC nodes.
 *
 * @param names Place where the names should be stored.
 * @param nodes  List of of NoC node IDs.
 * @param nnodes Number of NoC nodes in the list.
 */
void noc_get_names(char *names, const int *nodes, int nnodes)
{
	char tmp[5];

	names[0] = '\0';

	for (int i = 0; i < nnodes; i++)
	{
		sprintf(tmp, "%d,", nodes[i]);
		strcat(names, tmp);
	}

	names[strlen(names) - 1] = '\0';
}

/*=======================================================================*
 * noc_get_remotes()                                                     *
 *=======================================================================*/

/**
 * @brief Builds a list of remote NoC nodes.
 *
 * @param remotes   Place where the list should be stored.
 * @param clusterid ID of local cluster.
 */
void noc_get_remotes(char *remotes, int local)
{
	char tmp[5];

	static int cclusters[NR_CCLUSTER*NR_CCLUSTER_DMA] = {
		CCLUSTER0,  CCLUSTER1,  CCLUSTER2,  CCLUSTER3,
		CCLUSTER4,  CCLUSTER5,  CCLUSTER6,  CCLUSTER7,
		CCLUSTER8,  CCLUSTER9,  CCLUSTER10, CCLUSTER11,
		CCLUSTER12, CCLUSTER13, CCLUSTER14, CCLUSTER15
	};

	remotes[0] = '\0';

	/*
	 * Append IO Clusters.
	 *
	 * Not that since there are more than one NoC
	 * node in each I/O cluster, then a NoC node
	 * for each I/O will always be included.
	 */
	sprintf(tmp, "%d,", IOCLUSTER0);
	strcat(remotes, tmp);
	sprintf(tmp, "%d,", IOCLUSTER1);
	strcat(remotes, tmp);

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
 * noctag_mailbox()                                                      *
 *=======================================================================*/

/**
 * @brief Returns the mailbox NoC tag for a target NoC node ID.
 *
 * @param nodeid ID of the target NoC node.
 */
int noctag_mailbox(int nodeid)
{
	if (noc_is_ionode0(nodeid))
		return (NOCTAG_MAILBOX_OFF + nodeid%NR_IOCLUSTER_DMA);
	else if (noc_is_ionode1(nodeid))
		return (NOCTAG_MAILBOX_OFF + NR_IOCLUSTER_DMA + nodeid%NR_IOCLUSTER_DMA);

	return (NOCTAG_MAILBOX_OFF + NR_IOCLUSTER_DMA + NR_IOCLUSTER_DMA + nodeid);
}

/*=======================================================================*
 * noctag_sync()                                                         *
 *=======================================================================*/

/**
 * @brief Returns the synchronization NoC tag for a target NoC node ID.
 *
 * @param nodeid ID of the target NoC node.
 */
int noctag_sync(int nodeid)
{
	if (noc_is_ionode0(nodeid))
		return (NOCTAG_SYNC_OFF + nodeid%NR_IOCLUSTER_DMA);
	else if (noc_is_ionode1(nodeid))
		return (NOCTAG_SYNC_OFF + NR_IOCLUSTER_DMA + nodeid%NR_IOCLUSTER_DMA);

	return (NOCTAG_SYNC_OFF + NR_IOCLUSTER_DMA + NR_IOCLUSTER_DMA + nodeid);
}
