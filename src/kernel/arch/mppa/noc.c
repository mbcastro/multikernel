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
 * @brief NoC tags offset.
 *
 * @detail All NoC connectors that are listed bellow support 1:N
 * single-direction communication. Therefore, we need NR_DMA NoC tags
 * for each. The first two tags are used by the hardware and thus are
 * skipped.
 */
/**@{*/
#define NOCTAG_MAILBOX_OFF 2                            /**< Mailbox. */
#define NOCTAG_PORTAL_OFF (NOCTAG_MAILBOX_OFF + NR_DMA) /**< Portal.  */
#define NOCTAG_SYNC_OFF   (NOCTAG_PORTAL_OFF + NR_DMA)  /**< Sync.    */
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
 * noc_get_dma()                                                              *
 *============================================================================*/

/**
 * @brief Gets the DMA channel of a NoC node.
 *
 * @param nodeid ID of the target NoC node.
 */
int noc_get_dma(int nodeid)
{
	return (k1_is_ccluster(nodeid) ?
			nodeid%NR_CCLUSTER_DMA : nodeid%NR_IOCLUSTER_DMA);
}

/*=======================================================================*
 * noc_get_remotes()                                                         *
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
 *
 * @returns The NoC tag attached to the underlying node ID is
 * returned.
 */
int noctag_mailbox(int nodeid)
{
	if ((nodeid >= IOCLUSTER0) && (nodeid < (IOCLUSTER0 + NR_IOCLUSTER_DMA)))
	{
		return (NOCTAG_MAILBOX_OFF + nodeid%NR_IOCLUSTER_DMA);
	}
	else if ((nodeid >= IOCLUSTER1) && (nodeid < (IOCLUSTER1 + NR_IOCLUSTER_DMA)))
	{
		return (NOCTAG_MAILBOX_OFF + NR_IOCLUSTER_DMA + nodeid%NR_IOCLUSTER_DMA);
	}

	return (NOCTAG_MAILBOX_OFF + NR_IOCLUSTER_DMA + NR_IOCLUSTER_DMA + nodeid);
}

/*=======================================================================*
 * noctag_portal()                                                       *
 *=======================================================================*/

/**
 * @brief Returns the portal NoC tag for a target NoC node ID.
 *
 * @param nodeid     ID of the target NoC node.
 *
 * @returns The NoC tag attached to the underlying node ID is
 * returned.
 */
int noctag_portal(int nodeid)
{
	if ((nodeid >= IOCLUSTER0) && (nodeid < (IOCLUSTER0 + NR_IOCLUSTER_DMA)))
	{
		return (NOCTAG_PORTAL_OFF + nodeid%NR_IOCLUSTER_DMA);
	}
	else if ((nodeid >= IOCLUSTER1) && (nodeid < (IOCLUSTER1 + NR_IOCLUSTER_DMA)))
	{
		return (NOCTAG_PORTAL_OFF + NR_IOCLUSTER_DMA + nodeid%NR_IOCLUSTER_DMA);
	}

	return (NOCTAG_PORTAL_OFF + NR_IOCLUSTER_DMA + NR_IOCLUSTER_DMA + nodeid);
}
