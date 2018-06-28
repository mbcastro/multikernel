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

#include <errno.h>

#define __NEED_HAL_NOC_
#define __NEED_HAL_SYNC_

#include <nanvix/hal.h>

/**
 * @brief Barrier flags.
 */
/**@{*/
#define BARRIER_USED   (1 << 0)
/**@}*/

/**
 * @brief Barrier.
 */
struct barrier
{
	int local;                   /* Local sync.  */
	int remote;                  /* Remote sync. */
	int nnodes;                  /* Number of NoC nodes in the barrier. */
	int nodes[HAL_NR_NOC_NODES]; /* Id of NoC Nodes in the barrier. */
	int flags;                   /* Flags. */
};

/**
 * @brief table of barriers.
 */
static struct barrier barriers[HAL_NR_NOC_NODES];

/*=======================================================================*
 * barrier_alloc()                                                       *
 *=======================================================================*/

/**
 * @brief Allocates a barrier.
 *
 * @return Upon successful completion the ID of the newly allocated
 * barrier is returned. Upon failure, a negative error code is returned
 * instead.
 */
static int barrier_alloc(void)
{
	/* Search for a free barrier. */
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
	{
		/* Found. */
		if (!(barriers[i].flags & BARRIER_USED))
		{
			barriers[i].flags |= BARRIER_USED;
			return (i);
		}
	}

	return (-ENOENT);
}

/*=======================================================================*
 * barrier_free()                                                        *
 *=======================================================================*/

/**
 * @brief Frees a barrier.
 *
 * @param barrierid ID of the target barrier.
 */
static int barrier_free(int barrierid)
{
	/* Sanity check. */
	if ((barrierid < 0) || (barrierid >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	if (!(barriers[barrierid].flags & BARRIER_USED))
		return (-EINVAL);

	barriers[barrierid].flags = 0;

	return (0);
}

/*=======================================================================*
 * barrier_create()                                                        *
 *=======================================================================*/

/**
 * @brief Create a barrier.
 *
 * @param nodes		List of nodes in the barrier.
 * @param nnodes	Number of nodes in the barrier.
 */
int barrier_create(int *nodes, int nnodes)
{
	int barrierid;		/* Id of the barrier.         */
	int local;			/* Local sync connector.      */
	int remote;			/* Remote sync connector.     */
	int nodeid;			/* NoC node Id.               */
	int found = 0;		/* Is this node in the list ? */

	/* Invalid node list. */
	if (nodes == NULL)
		return (-EINVAL);

	/* Number of nodes Invalid. */
	if ((nnodes < 0) && (nnodes >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	nodeid = hal_get_node_id();

	/* This node should be in the list. */
	for (int i = 0; i < nnodes ; i++)
		if(nodes[i] == nodeid)
			found = 1;

	if (!found)
		return (-EINVAL);

	/* Allocate a barrier. */
	if((barrierid = barrier_alloc()) < 0)
		return (-EAGAIN);

	if (nodeid == nodes[0])
	{
		/* This node is the leader of the barrier. */
		if ((local = hal_sync_create(nodes, nnodes, HAL_SYNC_ALL_TO_ONE)) < 0)
			return (-EAGAIN);

		if ((remote = hal_sync_open(nodes, nnodes, HAL_SYNC_ONE_TO_ALL)) < 0)
			return (-EAGAIN);
	}
	else
	{
		/* This node is not the leader of the barrier. */
		if ((local = hal_sync_create(nodes, nnodes, HAL_SYNC_ONE_TO_ALL)) < 0)
			return (-EAGAIN);

		if ((remote = hal_sync_open(nodes, nnodes, HAL_SYNC_ALL_TO_ONE)) < 0)
			return (-EAGAIN);
	}

	/* Initialize the barrier. */
	barriers[barrierid].remote = remote;
	barriers[barrierid].local = local;
	barriers[barrierid].nnodes = nnodes;

	for(int i = 0; i < nnodes; i++)
		barriers[barrierid].nodes[i] = nodes[i];

	return (barrierid);
}

/*=======================================================================*
 * barrier_unlink()                                                      *
 *=======================================================================*/

/**
 * @brief Unlink a barrier.
 *
 * @param barrierid		Id of the barrier to unlink.
 */
int barrier_unlink(int barrierid)
{
	/* Invalid barrier Id. */
	if ((barrierid < 0) || (barrierid >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Bad barrier. */
	if (!(barriers[barrierid].flags & BARRIER_USED))
		return (-EINVAL);

	if (hal_sync_unlink(barriers[barrierid].local) != 0)
		return (-EAGAIN);

	if (hal_sync_close(barriers[barrierid].remote) != 0)
		return (-EAGAIN);

	if (barrier_free(barrierid) != 0)
		return (-EINVAL);

	return(0);
}

/*=======================================================================*
 * barrier_wait()                                                        *
 *=======================================================================*/

/**
 * @brief Waits on a barrier.
 *
 * @param barrierid ID of the target barrier.
 */
int barrier_wait(int barrierid)
{
	int found = 0;		/* Is this node in the list ? */
	int nodeid;			/* NoC node Id.               */

	/* Invalid barrier Id. */
	if ((barrierid < 0) || (barrierid >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Bad barrier. */
	if (!(barriers[barrierid].flags & BARRIER_USED))
		return (-EINVAL);

	nodeid = hal_get_node_id();

	/* Is this node the leader of the list ? */
	if (nodeid == barriers[barrierid].nodes[0])
	{
		/* Wait for others clusters. */
		if (hal_sync_wait(barriers[barrierid].local) != 0)
		return (-EAGAIN);

		/* Signal others clusters. */
		if (hal_sync_signal(barriers[barrierid].remote) != 0)
		return (-EAGAIN);
	}
	else
	{
		/* This node should be in the list. */
		for (int i = 0; i < barriers[barrierid].nnodes ; i++)
			if(barriers[barrierid].nodes[i] == nodeid)
				found = 1;

		if (!found)
			return (-EINVAL);

		/* Signal leader. */
		if (hal_sync_signal(barriers[barrierid].remote) != 0)
			return (-EAGAIN);

		/* Wait for leader cluster. */
		if (hal_sync_wait(barriers[barrierid].local) != 0)
			return (-EAGAIN);
	}

	return (0);
}
