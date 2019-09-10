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

#define __NEED_HAL_NOC_
#define __NEED_HAL_SYNC_
#include <hal.h>
#include <klib.h>

/**
 * @brief Builds the list of RX NoC nodes.
 *
 * @param ranks  Target list of RX NoC nodes.
 * @param nodes  IDs of target NoC nodes.
 * @param nnodes Number of target NoC nodes. 
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
static void sync_ranks(int *ranks, const int *nodes, int nnodes)
{
	int j, tmp;
	int nodeid;

	j = 1;
	nodeid = hal_get_node_id();

	/* Build list of RX NoC nodes. */
	for (int i = 0; i < nnodes; i++)
	{
		if (nodes[i] == nodeid)
			j = i;

		ranks[i] = nodes[i];
	}

	tmp = ranks[1];
	ranks[1] = ranks[j];
	ranks[j] = tmp;
}

/**
 * @brief Converts a nodes list.
 *
 * @param _nodes Place to store converted list.
 * @param nodes  Target nodes list.
 * @param nnodes Number of nodes in the list.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int nodes_convert(int *_nodes, const int *nodes, int nnodes)
{
	/* Convert NoC node numbers into IDs. */
	for (int i = 0; i < nnodes; i++)
	{
		/* Invalid nodes list. */
		if ((nodes[i] < 0) || (nodes[i] >= HAL_NR_NOC_NODES))
			return (-EINVAL);

		_nodes[i] = hal_noc_nodes[nodes[i]];
	}

	return (0);
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
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_sync_create(const int *nodes, int nnodes, int type)
{
	int nodeid;
	int ranks[nnodes];
	int _nodes[nnodes];

	/* Invalid list of nodes. */
	if (nodes == NULL)
		return (-EINVAL);

	/* Invalid number of nodes. */
	if ((nnodes < 2) || (nnodes > HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid type. */
	if ((type != HAL_SYNC_ONE_TO_ALL) && (type != HAL_SYNC_ALL_TO_ONE))
		return (-EINVAL);

	/* Convert NoC node numbers into IDs. */
	if (nodes_convert(_nodes, nodes, nnodes) < 0)
		return (-EINVAL);

	nodeid = hal_get_node_id();

	if (type == HAL_SYNC_ONE_TO_ALL)
	{
		int found = 0;

		/* Underlying NoC node SHOULD NOT be here. */
		if (nodeid == _nodes[0])
			return (-EINVAL);

		/* Underlying NoC node SHOULD be here. */
		for (int i = 1; i < nnodes; i++)
		{
			if (nodeid == _nodes[i])
				found++;
		}

		if (found != 1)
			return (-EINVAL);
	}

	else
	{
		/* Underlying NoC node SHOULD be here. */
		if (nodeid != _nodes[0])
			return (-EINVAL);

		/* Underlying NoC node SHOULD be here. */
		for (int i = 1; i < nnodes; i++)
		{
			if (nodeid == _nodes[i])
				return (-EINVAL);
		}
	}

	if (type == HAL_SYNC_ONE_TO_ALL)
		sync_ranks(ranks, _nodes, nnodes);
	else
		memcpy(ranks, _nodes, nnodes*sizeof(int));

	return (hal_sync_create(ranks, nnodes, type));
}


/**
 * @brief Opens a synchronization point.
 *
 * @param nodes  IDs of target NoC nodes.
 * @param nnodes Number of target NoC nodes. 
 * @param type   Type of synchronization point.
 *
 * @returns Upon successful completion, the ID of the target
 * synchronization point is returned. Upon failure, a negative error
 * code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 *
 * @todo Check for Invalid Remote
 */
int sys_sync_open(const int *nodes, int nnodes, int type)
{
	int nodeid;
	int _nodes[nnodes];

	/* Invalid nodes list. */
	if (nodes == NULL)
		return (-EINVAL);

	/* Invalid number of nodes. */
	if ((nnodes < 2) || (nnodes > HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid type. */
	if ((type != HAL_SYNC_ONE_TO_ALL) && (type != HAL_SYNC_ALL_TO_ONE))
		return (-EINVAL);

	/* Convert NoC node numbers into IDs. */
	if (nodes_convert(_nodes, nodes, nnodes) < 0)
		return (-EINVAL);

	nodeid = hal_get_node_id();

	if (type == HAL_SYNC_ONE_TO_ALL)
	{
		/* Underlying NoC node SHOULD be here. */
		if (nodeid != _nodes[0])
			return (-EINVAL);

		/* Underlying NoC node SHOULD be here. */
		for (int i = 1; i < nnodes; i++)
		{
			if (nodeid == _nodes[i])
				return (-EINVAL);
		}
	}

	else
	{
		int found = 0;

		/* Underlying NoC node SHOULD NOT be here. */
		if (nodeid == _nodes[0])
			return (-EINVAL);

		/* Underlying NoC node SHOULD be here. */
		for (int i = 1; i < nnodes; i++)
		{
			if (nodeid == _nodes[i])
				found++;
		}

		if (found != 1)
			return (-EINVAL);
	}

	return (hal_sync_open(_nodes, nnodes, type));
}

/**
 * @brief Waits on a synchronization point.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_sync_wait(int syncid)
{
	return (hal_sync_wait(syncid));
}

/**
 * @brief Signals Waits on a synchronization point.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_sync_signal(int syncid)
{
	return (hal_sync_signal(syncid));
}

/**
 * @brief Closes a synchronization point.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_sync_close(int syncid)
{
	return (hal_sync_close(syncid));
}

/**
 * @brief Destroys a synchronization point.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_sync_unlink(int syncid)
{
	return (hal_sync_unlink(syncid));
}
