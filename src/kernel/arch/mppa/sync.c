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

#include <errno.h>
#include <stdio.h>

#include <nanvix/hal.h>

#include "mppa.h"

/*============================================================================*
 * hal_sync_create()                                                          *
 *============================================================================*/

/**
 * @see hal_sync_create()
 */
static int _hal_sync_create(const int *nodes, int nnodes, int type)
{
	int fd;             /* NoC connector.           */
	uint64_t mask;      /* Sync mask.               */
	char remotes[128];  /* IDs of remote NoC nodes. */
	char pathname[128]; /* NoC connector name.      */

	/* Build pathname for NoC connector. */
	noc_get_names(remotes, &nodes[1], nnodes - 1);
	sprintf(pathname,
		"/mppa/sync/[%s]:%d",
		remotes,
		noctag_sync(nodes[1])
	);

	/* Open NoC connector. */
	if ((fd = mppa_open(pathname, O_RDONLY)) == -1)
		goto error0;

	/* Build sync mask. */
	mask = -1;
	if (type == HAL_SYNC_ALL_TO_ONE)
	{
		for (int i = 0; i < nnodes; i++)
			mask |= 1 << noc_get_node_num(nodes[i]);
	}
	
	/* Setup sync mask. */
	if (mppa_ioctl(fd, MPPA_RX_SET_MATCH, ~mask) != 0)
		goto error1;

	return (fd);

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
 * @param type   Type of synchronization point.
 *
 * @returns Upon successful completion, the ID of the newly created
 * synchronization point is returned. Upon failure, a negative error
 * code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int hal_sync_create(const int *nodes, int nnodes, int type)
{
	int nodeid;

	/* Invalid list of nodes. */
	if (nodes == NULL)
		return (-EINVAL);

	/* Invalid number of nodes. */
	if ((nnodes < 2) || (nnodes >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid type. */
	if ((type != HAL_SYNC_ONE_TO_ALL) || (type != HAL_SYNC_ALL_TO_ONE))
		return (-EINVAL);

	nodeid = hal_get_node_id();

	/* Underlying NoC node SHOULD be here. */
	for (int i = 1; i < nnodes; i++)
	{
		if (nodeid == nodes[i])
			goto found;
	}

	return (-EINVAL);

found:

	return (_hal_sync_create(nodes, nnodes, type));
}

/*============================================================================*
 * hal_sync_open()                                                            *
 *============================================================================*/

/**
 * @see hal_sync_open()
 */
static int _hal_sync_open(const int *nodes, int nnodes)
{
	int fd;             /* NoC connector.           */
	char remotes[128];  /* IDs of remote NoC nodes. */
	char pathname[128]; /* NoC connector name.      */

	/* Build pathname for NoC connector. */
	noc_get_names(remotes, nodes, nnodes);
	sprintf(pathname,
		"/mppa/sync/[%s]:%d",
		remotes,
		noctag_sync(nodes[0])
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
 * @note This function is @b NOT thread safe.
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
	if (nodeid != nodes[0])
		return (-EINVAL);

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
 *
 * @note This function is @b NOT thread safe.
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

/*============================================================================*
 * hal_sync_signal()                                                            *
 *============================================================================*/

/**
 * @brief Signals Waits on a synchronization point.
 *
 * @param syncid ID of the target synchronization point.
 * @param type   Type of synchronization point.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int hal_sync_signal(int syncid, int type)
{
	int nodeid;
	uint64_t mask;

	/* Invalid synchronization point. */
	if (syncid < 0)
		return (-EINVAL);

	/* Invalid type. */
	if ((type != HAL_SYNC_ONE_TO_ALL) || (type != HAL_SYNC_ALL_TO_ONE))
		return (-EINVAL);

	/* Signal. */
	mask = (type == HAL_SYNC_ALL_TO_ONE) ? 
		1 << noc_get_node_num(nodeid) : ~0;
	if (mppa_write(syncid, &mask, sizeof(uint64_t)) != sizeof(uint64_t))
		return (-EAGAIN);

	return (0);
}

/*============================================================================*
 * hal_sync_close()                                                           *
 *============================================================================*/

/**
 * @brief Closes a synchronization point.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int hal_sync_close(int syncid)
{
	/* Invalid sync. */
	if (syncid < 0)
		return (-EINVAL);

	return (mppa_close(syncid));
}

/*============================================================================*
 * hal_sync_unlink()                                                          *
 *============================================================================*/

/**
 * @brief Destroys a synchronization point.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int hal_sync_unlink(int syncid)
{
	/* Invalid sync. */
	if (syncid < 0)
		return (-EINVAL);

	return (mppa_close(syncid));
}
