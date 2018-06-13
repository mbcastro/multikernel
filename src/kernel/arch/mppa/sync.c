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


/*============================================================================*
 * hal_sync_open()                                                            *
 *============================================================================*/

/**
 * @brief Opens a synchronization point.
 *
 * @param nodes  IDs of target NoC nodes.
 * @param nnodes Number of target NoC nodes. 
 *
 * @returns Upon successful completion, the ID of the target
 * synchronization point is returned. Upon failure, a negative error
 * code is returned instead.
 */
int hal_sync_open(int *nodes, int nnodes)
{
	int fd;             /* NoC connector.             */
	int nodeid;         /* ID of underlying NoC node. */
	char remotes[128];  /* IDs of remote NoC nodes.   */
	char pathname[128]; /* NoC connector name.        */

	/* Invalid list of nodes. */
	if (nodes == NULL)
		return (-EINVAL);

	nodeid = hal_get_node_id();

	/* Invalid number of nodes. */
	if ((nnodes < 0) || (nnodes >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Bad list of nodes. */
	for (int i = 0; i < nnodes; i++)
	{
		/* Underlying NoC node should not be here. */
		if (nodeid == nodes[i])
			return (-EINVAL);
	}

	/* Build pathname for NoC connector. */
	noc_node_names(remotes, nodes, nnodes);
	sprintf(pathname,
		"/mppa/sync/%d:[%s]",
		remotes,
		barrier_noctag(nodeid)
	);

	/* Open NoC connector. */
	if ((fd = mppa_open(pathname, O_WRONLY)) == -1)
		goto error0;

error0:
	return (-EAGAIN);
}
