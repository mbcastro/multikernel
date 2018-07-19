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

#include <nanvix/syscalls.h>
#include <nanvix/const.h>

/**
 * @brief Barrier flags.
 */
/**@{*/
#define BARRIER_USED (1 << 0)
/**@}*/

/**
 * @brief Barrier.
 */
struct barrier
{
	int local;                  /* Local sync.                         */
	int remote;                 /* Remote sync.                        */
	int nnodes;                 /* Number of NoC nodes in the barrier. */
	int nodes[NANVIX_NR_NODES]; /* Id of NoC nodes in the barrier.     */
	int flags;                  /* Flags.                              */
};

/**
 * @brief Table of barriers.
 */
static struct barrier barriers[NANVIX_NR_NODES];

/*=======================================================================*
 * barrier_is_valid()                                                    *
 *=======================================================================*/

/**
 * @brief Asserts whether or not a barrier is valid.
 *
 * @param barrierid		ID of the target barrier.
 *
 * @returns One if the target barrier is valid, and false
 * otherwise.
 */
static int barrier_is_valid(int barrierid)
{
	return ((barrierid >= 0) && (barrierid < NANVIX_NR_NODES));
}

/*=======================================================================*
 * barrier_is_used()                                                     *
 *=======================================================================*/

/**
 * @brief Asserts whether or not a barrier is used.
 *
 * @param barrierid		ID of the target barrier.
 *
 * @returns One if the target barrier is used, and false
 * otherwise.
 */
static int barrier_is_used(int barrierid)
{
	return (barriers[barrierid].flags & BARRIER_USED);
}

/*=======================================================================*
 * barrier_set_used()                                                    *
 *=======================================================================*/

/**
 * @brief Sets a barrier as used.
 *
 * @param barrierid		ID of the target barrier.
 */
static void barrier_set_used(int barrierid)
{
	barriers[barrierid].flags |= BARRIER_USED;
}

/*=======================================================================*
 * barrier_clear_flags()                                                    *
 *=======================================================================*/

/**
 * @brief Clears barrier flags.
 *
 * @param barrierid		ID of the target barrier.
 */
static void barrier_clear_flags(int barrierid)
{
	barriers[barrierid].flags = 0;
}

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
	for (int i = 0; i < NANVIX_NR_NODES; i++)
	{
		/* Found. */
		if (!barrier_is_used(i))
		{
			barrier_set_used(i);
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
	if (!barrier_is_valid(barrierid))
		return (-EINVAL);

	if (!barrier_is_used(barrierid))
		return (-EINVAL);

	barrier_clear_flags(barrierid);

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
	int barrierid; /* Id of the barrier.     */
	int local;	   /* Local sync connector.  */
	int remote;	   /* Remote sync connector. */
	int nodenum;   /* NoC node.              */

	/* Invalid node list. */
	if (nodes == NULL)
		return (-EINVAL);

	/* Number of nodes Invalid. */
	if ((nnodes < 0) || (nnodes >= NANVIX_NR_NODES))
		return (-EINVAL);

	nodenum = sys_get_node_num();

	/* This node should be in the list. */
	for (int i = 0; i < nnodes ; i++)
	{
		if (nodes[i] == nodenum)
			goto found;
	}

	return (-EINVAL);

found:

	/* Allocate a barrier. */
	if((barrierid = barrier_alloc()) < 0)
		return (-EAGAIN);

	if (nodenum == nodes[0])
	{
		/* Check for a bad barrier leader. */
		for (int i = 1; i < nnodes; i++)
		{
			if (nodenum < nodes[0])
				goto error0;
		}

		/* This node is the leader of the barrier. */
		if ((local = sys_sync_create(nodes, nnodes, SYNC_ALL_TO_ONE)) < 0)
			goto error0;

		if ((remote = sys_sync_open(nodes, nnodes, SYNC_ONE_TO_ALL)) < 0)
			goto error1;
	}
	else
	{
		/* This node is not the leader of the barrier. */
		if ((local = sys_sync_create(nodes, nnodes, SYNC_ONE_TO_ALL)) < 0)
			goto error0;

		if ((remote = sys_sync_open(nodes, nnodes, SYNC_ALL_TO_ONE)) < 0)
			goto error1;
	}

	/* Initialize the barrier. */
	barriers[barrierid].remote = remote;
	barriers[barrierid].local = local;
	barriers[barrierid].nnodes = nnodes;

	for(int i = 0; i < nnodes; i++)
		barriers[barrierid].nodes[i] = nodes[i];

	return (barrierid);

error1:
	sys_sync_unlink(local);
error0:
	barrier_free(barrierid);
	return (-EAGAIN);
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
	if (!barrier_is_valid(barrierid))
		return (-EINVAL);

	/* Bad barrier. */
	if (!barrier_is_used(barrierid))
		return (-EINVAL);

	if (sys_sync_unlink(barriers[barrierid].local) != 0)
		return (-EAGAIN);

	if (sys_sync_close(barriers[barrierid].remote) != 0)
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
	int nodenum;

	/* Invalid barrier Id. */
	if (!barrier_is_valid(barrierid))
		return (-EINVAL);

	/* Bad barrier. */
	if (!barrier_is_used(barrierid))
		return (-EINVAL);

	nodenum = sys_get_node_num();

	/* Is this node the leader of the list ? */
	if (nodenum == barriers[barrierid].nodes[0])
	{
		/* Wait for others nodes. */
		if (sys_sync_wait(barriers[barrierid].local) != 0)
			return (-EAGAIN);

		/* Signal others nodes. */
		if (sys_sync_signal(barriers[barrierid].remote) != 0)
			return (-EAGAIN);
	}
	else
	{
		/* This node should be in the list. */
		for (int i = 0; i < barriers[barrierid].nnodes ; i++)
		{
			if (barriers[barrierid].nodes[i] == nodenum)
				goto found;
		}

		return (-EINVAL);

found:

		/* Signal leader. */
		if (sys_sync_signal(barriers[barrierid].remote) != 0)
			return (-EAGAIN);

		/* Wait for leader node. */
		if (sys_sync_wait(barriers[barrierid].local) != 0)
			return (-EAGAIN);
	}

	return (0);
}
