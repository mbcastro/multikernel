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
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define __NEED_HAL_NOC_
#define __NEED_HAL_SYNC_
#include <nanvix/hal.h>

#include "noc.h"

/**
 * @brief Synchronization point flags.
 */
/**@{*/
#define SYNC_FLAGS_USED      (1 << 0) /**< Used synchronization point?      */
#define SYNC_FLAGS_BROADCAST (1 << 1) /**< Broadcast synchronization point? */
#define SYNC_FLAGS_WRONLY    (1 << 2) /**< Write only mode?                 */
/**@}*/

/**
 * @brief Table of synchronization points.
 */
static struct 
{
	int fd;    /*< Underlying file descriptor. */
	int flags; /*< Flags.                      */
} synctab[HAL_NR_SYNC];

/**
 * @brief Sync module lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*============================================================================*
 * mppa256_sync_lock()                                                        *
 *============================================================================*/

/**
 * @brief Locks MPPA-256 sync module.
 */
static void mppa256_sync_lock(void)
{
	pthread_mutex_lock(&lock);
}

/*============================================================================*
 * mppa256_sync_unlock()                                                      *
 *============================================================================*/

/**
 * @brief Unlocks MPPA-256 sync module.
 */
static void mppa256_sync_unlock(void)
{
	pthread_mutex_unlock(&lock);
}

/*============================================================================*
 * sync_is_valid()                                                            *
 *============================================================================*/

/**
 * @brief Asserts whether or not a synchronization point is valid.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @returns One if the target synchronization point is valid, and false
 * otherwise.
 *
 * @note This function is thread-safe.
 */
static int sync_is_valid(int syncid)
{
	return ((syncid >= 0) || (syncid < HAL_NR_SYNC));
}

/*============================================================================*
 * sync_is_used()                                                             *
 *============================================================================*/

/**
 * @brief Asserts whether or not a synchronization point is used.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @returns One if the target synchronization point is used, and false
 * otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static int sync_is_used(int syncid)
{
	return (synctab[syncid].flags & SYNC_FLAGS_USED);
}

/*============================================================================*
 * sync_is_broadcast()                                                        *
 *============================================================================*/

/**
 * @brief Asserts whether of not a synchronization point is a broadcast one.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @returns One if the target synchronization point is a broadcast
 * one, and false otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static int sync_is_broadcast(int syncid)
{
	return (synctab[syncid].flags & SYNC_FLAGS_BROADCAST);
}

/*============================================================================*
 * sync_is_wronly()                                                           *
 *============================================================================*/

/**
 * @brief Asserts whether of not a synchronization point is a write-only.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @returns One if the target synchronization point is write-only, and
 * false otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static int sync_is_wronly(int syncid)
{
	return (synctab[syncid].flags & SYNC_FLAGS_WRONLY);
}

/*============================================================================*
 * sync_set_used()                                                            *
 *============================================================================*/

/**
 * @brief Sets a synchronization point as used.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @note This function is @b NOT thread safe.
 */
static void sync_set_used(int syncid)
{
	synctab[syncid].flags |= SYNC_FLAGS_USED;
}

/*============================================================================*
 * sync_set_broadcast()                                                       *
 *============================================================================*/

/**
 * @brief Sets a synchronization point as a broadcast one.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @note This function is @b NOT thread safe.
 */
static void sync_set_broadcast(int syncid)
{
	synctab[syncid].flags |= SYNC_FLAGS_BROADCAST;
}

/*============================================================================*
 * sync_set_wronly()                                                       *
 *============================================================================*/

/**
 * @brief Sets a synchronization point as a write-only.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @note This function is @b NOT thread safe.
 */
static void sync_set_wronly(int syncid)
{
	synctab[syncid].flags |= SYNC_FLAGS_WRONLY;
}

/*============================================================================*
 * sync_clear_flags()                                                         *
 *============================================================================*/

/**
 * @brief Clears the flags of a synchronization point.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @note This function is @b NOT thread safe.
 */
static void sync_clear_flags(int syncid)
{
	synctab[syncid].flags = 0;
}

/*============================================================================*
 * sync_alloc()                                                               *
 *============================================================================*/

/**
 * @brief Allocates a synchronization point.
 *
 * @returns Upon successful completion, the ID of a newly allocated
 * synchronization point is returned. Upon failure, -1 is returned
 * instead.
 *
 * @note This function is @b NOT thread safe.
 */
static int sync_alloc(void)
{
	/* Search for a free syncrhonization point. */
	for (int i = 0; i < HAL_NR_SYNC; i++)
	{
		/* Found. */
		if (!sync_is_used(i))
		{
			sync_set_used(i);
			return (i);
		}
	}

	return (-1);
}

/*============================================================================*
 * sync_free()                                                                *
 *============================================================================*/

/**
 * @brief Frees a synchronization point.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @note This function is @b NOT thread safe.
 */
static void sync_free(int syncid)
{
	sync_clear_flags(syncid);
}

/*============================================================================*
 * sync_ranks()                                                               *
 *============================================================================*/

/**
 * @brief Builds the list of RX NoC nodes.
 *
 * @param ranks Target list of RX NoC nodes.
 * @param nodes  IDs of target NoC nodes.
 * @param nnodes Number of target NoC nodes. 
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

/*============================================================================*
 * hal_sync_create()                                                          *
 *============================================================================*/

/**
 * @see hal_sync_create()
 *
 * @note In this function we may sleep while hold the lock. However, I
 * strongly beleive that no circular dependency may be introduced, so
 * we are good.
 */
static int mppa256_sync_create(const int *nodes, int nnodes, int type)
{
	int fd;             /* NoC connector.           */
	int syncid;         /* Synchronization point.   */
	uint64_t mask;      /* Sync mask.               */
	char remotes[128];  /* IDs of remote NoC nodes. */
	char pathname[128]; /* NoC connector name.      */

	/* Allocate a synchronization point. */
	if ((syncid = sync_alloc()) < 0)
		goto error0;

	mask = ~0;

	/* Broadcast. */
	if (type == HAL_SYNC_ONE_TO_ALL)
	{
		/* Build pathname for NoC connector. */
		noc_get_names(remotes, &nodes[1], nnodes - 1);
		sprintf(pathname,
			"/mppa/sync/[%s]:%d",
			remotes,
			noctag_sync(nodes[0])
		);
	}

	/* Gather. */
	else
	{
		/* Build pathname for NoC connector. */
		sprintf(pathname,
			"/mppa/sync/%d:%d",
			nodes[0],
			noctag_sync(nodes[0])
		);

		/* Build sync mask. */
		mask = 0;
		for (int i = 1; i < nnodes; i++)
			mask |= 1 << noc_get_node_num(nodes[i]);
	}

	/* Open NoC connector. */
	if ((fd = mppa_open(pathname, O_RDONLY)) == -1)
		goto error1;

	/* Setup sync mask. */
	if (mppa_ioctl(fd, MPPA_RX_SET_MATCH, ~mask) == -1)
		goto error2;

	/* Initialize synchronization point. */
	synctab[syncid].fd = fd;
	if (type == HAL_SYNC_ONE_TO_ALL)
		sync_set_broadcast(syncid);

	return (syncid);

error2:
	mppa_close(fd);
error1:
	sync_free(syncid);
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
 * @note This function is thread-safe.
 */
int hal_sync_create(const int *nodes, int nnodes, int type)
{
	int syncid;
	int nodeid;
	int ranks[nnodes];

	/* Invalid list of nodes. */
	if (nodes == NULL)
		return (-EINVAL);

	/* Invalid number of nodes. */
	if ((nnodes < 2) || (nnodes > HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid type. */
	if ((type != HAL_SYNC_ONE_TO_ALL) && (type != HAL_SYNC_ALL_TO_ONE))
		return (-EINVAL);

	nodeid = hal_get_node_id();

	if (type == HAL_SYNC_ONE_TO_ALL)
	{
		int found = 0;

		/* Underlying NoC node SHOULD NOT be here. */
		if (nodeid == nodes[0])
			return (-EINVAL);

		/* Underlying NoC node SHOULD be here. */
		for (int i = 1; i < nnodes; i++)
		{
			if (nodeid == nodes[i])
				found++;
		}

		if (found != 1)
			return (-EINVAL);
	}

	else
	{
		/* Underlying NoC node SHOULD be here. */
		if (nodeid != nodes[0])
			return (-EINVAL);

		/* Underlying NoC node SHOULD be here. */
		for (int i = 1; i < nnodes; i++)
		{
			if (nodeid == nodes[i])
				return (-EINVAL);
		}
	}

	if (type == HAL_SYNC_ONE_TO_ALL)
		sync_ranks(ranks, nodes, nnodes);
	else
		memcpy(ranks, nodes, nnodes*sizeof(int));

	mppa256_sync_lock();
		syncid = mppa256_sync_create(ranks, nnodes, type);
	mppa256_sync_unlock();

	return (syncid);
}

/*============================================================================*
 * hal_sync_open()                                                            *
 *============================================================================*/

/**
 * @see hal_sync_open()
 *
 * @note In this function we may sleep while hold the lock. However, I
 * strongly beleive that no circular dependency may be introduced, so
 * we are good.
 */
static int mppa256_sync_open(const int *nodes, int nnodes, int type)
{
	int fd;                /* NoC connector.             */
	int syncid;            /* Synchronization point.     */
	char remotes[128];     /* IDs of remote NoC nodes.   */
	char pathname[128];    /* NoC connector name.        */
	int ranks[nnodes - 1]; /* Offset to RX NoC nodes.    */
	int nodeid;            /* ID of underlying NoC node. */

	/* Allocate a synchronization point. */
	if ((syncid = sync_alloc()) < 0)
		goto error0;

	/* Broadcast. */
	if (type == HAL_SYNC_ONE_TO_ALL)
	{
		/* Build pathname for NoC connector. */
		noc_get_names(remotes, &nodes[1], nnodes - 1);
		sprintf(pathname,
			"/mppa/sync/[%s]:%d",
			remotes,
			noctag_sync(nodes[0])
		);
	}

	/* Gather. */
	else
	{
		/* Build pathname for NoC connector. */
		sprintf(pathname,
			"/mppa/sync/%d:%d",
			nodes[0],
			noctag_sync(nodes[0])
		);

		nnodes = 2;
	}

	/* Open NoC connector. */
	if ((fd = mppa_open(pathname, O_WRONLY)) == -1)
		goto error1;

	/* Set DMA interface for IO cluster. */
	nodeid = hal_get_node_id();
	if (noc_is_ionode(nodeid))
	{
		if (mppa_ioctl(fd, MPPA_TX_SET_INTERFACE, noc_get_dma(nodeid)) == -1)
			goto error2;
	}

	if (type == HAL_SYNC_ONE_TO_ALL)
	{
		/* Build list of RX NoC nodes. */
		for (int i = 0; i < nnodes - 1; i++)
			ranks[i] = i;

		if (mppa_ioctl(fd, MPPA_TX_SET_RX_RANKS, nnodes - 1, ranks) == -1)
			goto error2;
	}

	/* Initialize synchronization point. */
	synctab[syncid].fd = fd;
	if (type == HAL_SYNC_ONE_TO_ALL)
		sync_set_broadcast(syncid);
	sync_set_wronly(syncid);

	return (syncid);

error2:
	mppa_close(fd);
error1:
	sync_free(syncid);
error0:
	return (-EAGAIN);
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
 * @note This function is thread-safe.
 *
 * @todo Check for Invalid Remote
 */
int hal_sync_open(const int *nodes, int nnodes, int type)
{
	int syncid;
	int nodeid;

	/* Invalid list of nodes. */
	if (nodes == NULL)
		return (-EINVAL);

	/* Invalid number of nodes. */
	if ((nnodes < 2) || (nnodes > HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Invalid type. */
	if ((type != HAL_SYNC_ONE_TO_ALL) && (type != HAL_SYNC_ALL_TO_ONE))
		return (-EINVAL);

	nodeid = hal_get_node_id();

	if (type == HAL_SYNC_ONE_TO_ALL)
	{
		/* Underlying NoC node SHOULD be here. */
		if (nodeid != nodes[0])
			return (-EINVAL);

		/* Underlying NoC node SHOULD be here. */
		for (int i = 1; i < nnodes; i++)
		{
			if (nodeid == nodes[i])
				return (-EINVAL);
		}
	}

	else
	{
		int found = 0;

		/* Underlying NoC node SHOULD NOT be here. */
		if (nodeid == nodes[0])
			return (-EINVAL);

		/* Underlying NoC node SHOULD be here. */
		for (int i = 1; i < nnodes; i++)
		{
			if (nodeid == nodes[i])
				found++;
		}

		if (found != 1)
			return (-EINVAL);
	}

	mppa256_sync_lock();
		syncid = mppa256_sync_open(nodes, nnodes, type);
	mppa256_sync_unlock();

	return (syncid);
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

	/* Invalid sync. */
	if (!sync_is_valid(syncid))
		goto error0;

	mppa256_sync_lock();

		/* Bad sync. */
		if (!sync_is_used(syncid))
			goto error1;

		/* Bad sync. */
		if (sync_is_wronly(syncid))
			goto error1;

	/*
	 * Realease lock, since we may sleep below.
	 */
	mppa256_sync_unlock();

	/* Wait. */
	if (mppa_read(synctab[syncid].fd, &mask, sizeof(uint64_t)) != sizeof(uint64_t))
		goto error0;

	return (0);

error1:
	mppa256_sync_unlock();
error0:
	return (-EAGAIN);
}

/*============================================================================*
 * hal_sync_signal()                                                            *
 *============================================================================*/

/**
 * @brief Signals Waits on a synchronization point.
 *
 * @param syncid ID of the target synchronization point.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int hal_sync_signal(int syncid)
{
	int nodeid;
	uint64_t mask;
	int is_broadcast;

	/* Invalid sync. */
	if (!sync_is_valid(syncid))
		goto error0;

	mppa256_sync_lock();

		/* Bad sync. */
		if (!sync_is_used(syncid))
			goto error1;

		/* Bad sync. */
		if (!sync_is_wronly(syncid))
			goto error1;

		is_broadcast = sync_is_broadcast(syncid);

	/*
	 * Realease lock, since we may sleep below.
	 */
	mppa256_sync_unlock();

	nodeid = hal_get_node_id();

	/* Signal. */
	mask = (!is_broadcast) ? 1 << noc_get_node_num(nodeid) : ~0;
	if (mppa_write(synctab[syncid].fd, &mask, sizeof(uint64_t)) != sizeof(uint64_t))
		goto error0;

	return (0);

error1:
	mppa256_sync_unlock();
error0:
	return (-EAGAIN);
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
	if (!sync_is_valid(syncid))
		goto error0;

	mppa256_sync_lock();

		/* Bad sync. */
		if (!sync_is_used(syncid))
			goto error1;

		/* Bad sync. */
		if (!sync_is_wronly(syncid))
			goto error1;

	/*
	 * Realease lock, since we may sleep below.
	 */
	mppa256_sync_unlock();

	if (mppa_close(synctab[syncid].fd) < 0)
		goto error0;

	mppa256_sync_lock();
		sync_free(syncid);
	mppa256_sync_unlock();

	return (0);

error1:
	mppa256_sync_unlock();
error0:
	return (-EAGAIN);
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
	if (!sync_is_valid(syncid))
		goto error0;

	mppa256_sync_lock();

		/* Bad sync. */
		if (!sync_is_used(syncid))
			goto error1;

		/* Bad sync. */
		if (sync_is_wronly(syncid))
			goto error1;

	/*
	 * Realease lock, since we may sleep below.
	 */
	mppa256_sync_unlock();

	if (mppa_close(synctab[syncid].fd) < 0)
		goto error0;

	mppa256_sync_lock();
		sync_free(syncid);
	mppa256_sync_unlock();

	return (0);

error1:
	mppa256_sync_unlock();
error0:
	return (-EAGAIN);
}
