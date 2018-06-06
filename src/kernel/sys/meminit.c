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

#include <nanvix/hal.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <stdio.h>

/**
 * @brief Underlying IPC connectors.
 */
/**@{*/
int _mem_outbox = -1;    /* Mailbox used for small transfers. */
int _mem_inportal = -1;  /* Portal used for large transfers.  */
int _mem_outportal = -1; /* Portal used for large transfers.  */
/**@}*/

/**
 * @brief Initializes the RMA engine.
 */
void meminit(void)
{
	int clusterid;              /* Cluster ID of the calling process.   */
	char pathname[128];         /* Name of underlying IPC connector.    */
	char *clustername;          /* Cluster name of the calling process. */
	static int initialized = 0; /* IS RMA Engine initialized?           */

	/* Already initialized.  */
	if (initialized)
		return;

	/* Retrieve cluster information. */
	clusterid = k1_get_cluster_id();
	clustername = id_cluster_name(clusterid);

	/* Open underlying IPC connectors. */
	sprintf(pathname, "/rmem%d", clusterid%NR_IOCLUSTER_DMA);
	_mem_inportal = portal_create(clustername);
	_mem_outbox = _mailbox_open(IOCLUSTER1 + clusterid%NR_IOCLUSTER_DMA, STD);
	_mem_outportal = portal_open(pathname);

	initialized = 1;
}
