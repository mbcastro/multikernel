/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#include <hal.h>
#include <klib.h>

#include "core.h"

/**
 * @brief Number of DMAs per compute cluster.
 */
#define NR_CCLUSTER_DMA 1

/**
 * @brief Number of DMAs per IO cluster.
 */
#define NR_IOCLUSTER_DMA 4

/**
 * @brief NoC tags offsets.
 *
 * @detail All NoC connectors that are listed bellow support 1:N
 * single-direction communication. Therefore, we need HAL_NR_NOC_NODES
 * NoC tags for each. The first two tags are used by the hardware and
 * thus are skipped.
 */
/**@{*/
#define NOCTAG_MAILBOX_OFF 2                                      /**< Mailbox. */
#define NOCTAG_PORTAL_OFF (NOCTAG_MAILBOX_OFF + HAL_NR_NOC_NODES) /**< Portal.  */
#define NOCTAG_SYNC_OFF   (NOCTAG_PORTAL_OFF + HAL_NR_NOC_NODES)  /**< Sync.    */
/**@}*/

/**
 * @brief IDs of NoC nodes.
 */
const int hal_noc_nodes[HAL_NR_NOC_NODES] = {
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
	CCLUSTER15,
	IOCLUSTER0 + 0,
	IOCLUSTER0 + 1,
	IOCLUSTER0 + 2,
	IOCLUSTER0 + 3,
	IOCLUSTER1 + 0,
	IOCLUSTER1 + 1,
	IOCLUSTER1 + 2,
	IOCLUSTER1 + 3
};

/*============================================================================*
 * hal_get_node_id()                                                          *
 *============================================================================*/

/**
 * @brief Gets the ID of the NoC node attached to the underlying core.
 *
 * @returns The ID of the NoC node attached to the underlying core is
 * returned.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 */
int hal_get_node_id(void)
{
	int clusterid;

	clusterid = mppa256_get_cluster_id();

	if (mppa256_is_iocluster(clusterid))
		return (clusterid + hal_get_core_id());

	return (clusterid);
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
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int hal_get_node_num(int nodeid)
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
 * noc_is_ionode0()                                                            *
 *============================================================================*/

/**
 * @brief Asserts whether a NoC node is attached to IO cluster 0.
 *
 * @param nodeid ID of the target NoC node.
 *
 * @returns One if the target NoC node is attached to IO cluster 0,
 * and zero otherwise.
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int noc_is_ionode0(int nodeid)
{
	return ((nodeid >= IOCLUSTER0) && (nodeid < IOCLUSTER0 + 4));
}

/*============================================================================*
 * noc_is_ionode1()                                                           *
 *============================================================================*/

/**
 * @brief Asserts whether a NoC node is attached to IO cluster 1.
 *
 * @param nodeid ID of the target NoC node.
 *
 * @returns One if the target NoC node is attached to IO cluster 1,
 * and zero otherwise.
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
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
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
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
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int noc_is_cnode(int nodeid)
{
	return ((nodeid >= CCLUSTER0) && (nodeid <= CCLUSTER15));
}

/*============================================================================*
 * noc_get_dma()                                                              *
 *============================================================================*/

/**
 * @brief Gets the DMA channel to use in a data transfer.
 *
 * @param local  ID of local NoC node.
 * @param remote ID of remote NoC node.
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int noc_get_dma(int local, int remote)
{
	return (noc_is_cnode(local) ?
			local%NR_CCLUSTER_DMA : remote%NR_IOCLUSTER_DMA);
}

/*=======================================================================*
 * noc_get_names()                                                       *
 *=======================================================================*/

/**
 * @brief Gets the name of NoC nodes.
 *
 * @param names  Place where the names should be stored.
 * @param nodes  List of NoC node IDs.
 * @param nnodes Number of NoC nodes in the list.
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
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
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
void noc_get_remotes(char *remotes, int local)
{
	char tmp[5];

	static int cclusters[HAL_NR_CCLUSTERS*NR_CCLUSTER_DMA] = {
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
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int noctag_mailbox(int nodeid)
{
	if (noc_is_ionode0(nodeid))
	{
		return (NOCTAG_MAILBOX_OFF + nodeid%NR_IOCLUSTER_DMA);
	}
	else if (noc_is_ionode1(nodeid))
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
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int noctag_portal(int nodeid)
{
	if (noc_is_ionode0(nodeid))
	{
		return (NOCTAG_PORTAL_OFF + nodeid%NR_IOCLUSTER_DMA);
	}
	else if (noc_is_ionode1(nodeid))
	{
		return (NOCTAG_PORTAL_OFF + NR_IOCLUSTER_DMA + nodeid%NR_IOCLUSTER_DMA);
	}

	return (NOCTAG_PORTAL_OFF + NR_IOCLUSTER_DMA + NR_IOCLUSTER_DMA + nodeid);
}

/*=======================================================================*
 * noctag_sync()                                                         *
 *=======================================================================*/

/**
 * @brief Returns the synchronization NoC tag for a target NoC node ID.
 *
 * @param nodeid ID of the target NoC node.
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int noctag_sync(int nodeid)
{
	if (noc_is_ionode0(nodeid))
		return (NOCTAG_SYNC_OFF + nodeid%NR_IOCLUSTER_DMA);
	else if (noc_is_ionode1(nodeid))
		return (NOCTAG_SYNC_OFF + NR_IOCLUSTER_DMA + nodeid%NR_IOCLUSTER_DMA);

	return (NOCTAG_SYNC_OFF + NR_IOCLUSTER_DMA + NR_IOCLUSTER_DMA + nodeid);
}
