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
 * hal_sync_create()                                                          *
 *============================================================================*/

/**
 * @see hal_sync_create()
 */
static int _hal_sync_create(const int *nodes, int nnodes)
{
	int fd;             /* NoC connector.           */
	uint64_t mask;      /* Sync mask.               */
	char remotes[128];  /* IDs of remote NoC nodes. */
	char pathname[128]; /* NoC connector name.      */

	/* Build pathname for NoC connector. */
	noc_node_names(remotes, nodes, nnodes);
	sprintf(pathname,
		"/mppa/sync/[%s]:%d",
		remotes,
		barrier_noctag(nodes[0])
	);

	/* Open NoC connector. */
	if ((fd = mppa_open(pathname, O_RDONLY)) == -1)
		goto error0;

	mask = ~((1 << (nnodes - 1)) - 1);
	if (mppa_ioctl(fd, MPPA_RX_SET_MATCH, mask) != 0)
		goto error1;

error1:
	mppa_close(fd);
error0:
	return (-EAGAIN);
}

/**
 * @brief Creates a synchronization point.
 *
 * @param nodes  IDs of target NoC nodes.
 * @param nnodes Number of target NoC nodes. 
 *
 * @returns Upon successful completion, the ID of the newly created
 * synchronization point is returned. Upon failure, a negative error
 * code is returned instead.
 */
int hal_sync_create(const int *nodes, int nnodes)
{
	int nodeid;

	/* Invalid list of nodes. */
	if (nodes == NULL)
		return (-EINVAL);

	/* Invalid number of nodes. */
	if ((nnodes < 0) || (nnodes >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	nodeid = hal_get_node_id();

	/* Underlying NoC node SHOULD be here. */
	for (int i = 0; i < nnodes; i++)
	{
		if (nodeid == nodes[i])
			goto found;
	}

	return (-EINVAL);

found:

	return (_hal_sync_create(nodes, nnodes));
}

/*============================================================================*
 * hal_sync_open()                                                            *
 *============================================================================*/

/**
 * @see hal_sync_open()
 */
static int _hal_sync_open(int *nodes, int nnodes)
{
	int fd;             /* NoC connector.           */
	uint64_t mask;      /* Sync mask.               */
	char remotes[128];  /* IDs of remote NoC nodes. */
	char pathname[128]; /* NoC connector name.      */

	/* Build pathname for NoC connector. */
	noc_node_names(remotes, nodes, nnodes);
	sprintf(pathname,
		"/mppa/sync/[%s]:%d",
		remotes,
		barrier_noctag(nodes[0])
	);

	/* Open NoC connector. */
	if ((fd = mppa_open(pathname, O_WRONLY)) == -1)
		goto error0;

	if (mppa_ioctl(fd, MPPA_TX_SET_RX_RANKS, nnodes, nodes) != 0)
		goto error1;

	return (fd);

error1:
	mppa_close(fd);
error0:
	return (-EAGAIN);
}

/**
 * @brief Opens a synchronization point.
 *
 * @param nodes  IDs of target NoC nodes.
 * @param nnodes Number of target NoC nodes. 
 *
 * @returns Upon successful completion, the ID of the target
 * synchronization point is returned. Upon failure, a negative error
 * code is returned instead.
 *
 * @todo Check for Invalid Remote
 */
int hal_sync_open(const int *nodes, int nnodes)
{
	int nodeid;

	/* Invalid list of nodes. */
	if (nodes == NULL)
		return (-EINVAL);

	/* Invalid number of nodes. */
	if ((nnodes < 0) || (nnodes >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	nodeid = hal_get_node_id();

	/* Underlying NoC node SHOULD NOT be here. */
	for (int i = 0; i < nnodes; i++)
	{
		if (nodeid != nodes[i])
			return (-EINVAL);
	}

	return (_hal_sync_open(nodes, nnodes));
}

/*============================================================================*
 * hal_sync_wait()                                                            *
 *============================================================================*/

/**
 * @brief Waits on a synchronization point.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int hal_sync_wait(int syncid)
{
	uint64_t mask;

	/* Invalid synchronization point. */
	if (syncid < 0)
		return (-EINVAL);

	/* Wait. */
	if (mppa_read(syncid, &mask, sizeof(uint64_t)) != sizeof(uint64_t))
		return (-EAGAIN);

	return (0);
}
