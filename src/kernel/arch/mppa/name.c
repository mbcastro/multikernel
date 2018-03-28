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
#include <errno.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Lookup table of cluster names.
 */
static const struct {
	int id;     /**< Cluster ID.  */
	int dma;    /**< DMA channel. */
	char *name; /**< Portal name. */
} names[NR_DMA] = {
	{ CCLUSTER0,  CCLUSTER0, "/cpu0" },
	{ CCLUSTER1,  CCLUSTER1, "/cpu1" },
	{ CCLUSTER2,  CCLUSTER2,  "/cpu2" },
	{ CCLUSTER3,  CCLUSTER3,  "/cpu3" },
	{ CCLUSTER4,  CCLUSTER4,  "/cpu4" },
	{ CCLUSTER5,  CCLUSTER5,  "/cpu5" },
	{ CCLUSTER6,  CCLUSTER6,  "/cpu6" },
	{ CCLUSTER7,  CCLUSTER7,  "/cpu7" },
	{ CCLUSTER8,  CCLUSTER8,  "/cpu8" },
	{ CCLUSTER9,  CCLUSTER9,  "/cpu9" },
	{ CCLUSTER10, CCLUSTER10, "/cpu10" },
	{ CCLUSTER11, CCLUSTER11, "/cpu11" },
	{ CCLUSTER12, CCLUSTER12, "/cpu12" },
	{ CCLUSTER13, CCLUSTER13, "/cpu13" },
	{ CCLUSTER14, CCLUSTER14, "/cpu14" },
	{ CCLUSTER15, CCLUSTER15, "/cpu15" },
	{ IOCLUSTER0, IOCLUSTER0 + 0, "/io0" },
	{ IOCLUSTER0, IOCLUSTER0 + 1, "/io1" },
	{ IOCLUSTER0, IOCLUSTER0 + 2, "/io2" },
	{ IOCLUSTER0, IOCLUSTER0 + 3, "/io3" },
	{ IOCLUSTER1, IOCLUSTER1 + 0, "/rmem0" },
	{ IOCLUSTER1, IOCLUSTER1 + 1, "/rmem1" },
	{ IOCLUSTER1, IOCLUSTER1 + 2, "/rmem2" },
	{ IOCLUSTER1, IOCLUSTER1 + 3, "/rmem3" }
};

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
int name_cluster_id(const char *name)
{
	/* Search for portal name. */
	for (int i = 0; i < NR_DMA; i++)
	{
		/* Found. */
		if (!strcmp(name, names[i].name))
			return (names[i].id);
	}

	return (-ENOENT);
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
int name_cluster_dma(const char *name)
{
	/* Search for portal name. */
	for (int i = 0; i < NR_DMA; i++)
	{
		/* Found. */
		if (!strcmp(name, names[i].name))
			return (names[i].dma);
	}

	return (-ENOENT);
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
const char *name_cluster_name(int clusterid)
{
	/* Search for portal name. */
	for (int i = 0; i < NR_DMA; i++)
	{
		/* Found. */
		if (names[i].id == clusterid)
			return (names[i].name);
	}

	return (NULL);
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
				"%d..%d,%d..%d",
				CCLUSTER0, CCLUSTER15, IOCLUSTER1, IOCLUSTER1 + NR_IOCLUSTER_DMA - 1
		);
	}
	if ((local >= IOCLUSTER1) && (local < (IOCLUSTER1 + NR_IOCLUSTER_DMA)))
	{
		sprintf(remotes,
				"%d..%d,%d..%d",
				CCLUSTER0, CCLUSTER15, IOCLUSTER0, IOCLUSTER0 + NR_IOCLUSTER_DMA - 1
		);
	}
	else if (local == CCLUSTER0)
	{
		sprintf(remotes,
				"%d..%d,%d..%d,%d..%d",
				CCLUSTER1, CCLUSTER15, IOCLUSTER0, IOCLUSTER0 + NR_IOCLUSTER_DMA - 1, IOCLUSTER1, IOCLUSTER1 + NR_IOCLUSTER_DMA - 1
		);
	}
	else if (local  == CCLUSTER15)
	{
		sprintf(remotes,
				"%d..%d,%d..%d,%d..%d",
				CCLUSTER0, CCLUSTER14, IOCLUSTER0, IOCLUSTER0 + NR_IOCLUSTER_DMA - 1, IOCLUSTER1, IOCLUSTER1 + NR_IOCLUSTER_DMA - 1
		);
	}
	else
	{
		sprintf(remotes,
				"%d..%d,%d..%d,%d..%d,%d..%d",
				CCLUSTER0, local - 1, local + 1, CCLUSTER15, IOCLUSTER0, IOCLUSTER0 + NR_IOCLUSTER_DMA - 1, IOCLUSTER1, IOCLUSTER1 + NR_IOCLUSTER_DMA - 1
		);
	}
}
