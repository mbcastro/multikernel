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

#include <inttypes.h>

#include <mppaipc.h>

#define __NEED_HAL_NOC_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_MUTEX_
#include <hal.h>
#include <klib.h>
#include <resource.h>

#include "noc.h"

/**
 * @brief Table of synchronization points.
 */
static struct sync
{
	struct resource resource; /**< Underlying resource.            */
	int fd;                   /**< Underlying file descriptor.     */
	int ncount;               /**< Number of remotes in broadcast. */
	int type;                 /**< Synchronization point type.     */
} synctab[HAL_NR_SYNC];

/**
 * @brief Resource pool.
 */
static const struct resource_pool pool = {
	synctab,
	HAL_NR_SYNC,
	sizeof(struct sync)
};

/**
 * @brief Sync module lock.
 */
static hal_mutex_t lock = HAL_MUTEX_INITIALIZER;

/*============================================================================*
 * mppa256_sync_lock()                                                        *
 *============================================================================*/

/**
 * @brief Locks MPPA-256 sync module.
 */
static void mppa256_sync_lock(void)
{
	hal_mutex_lock(&lock);
}

/*============================================================================*
 * mppa256_sync_unlock()                                                      *
 *============================================================================*/

/**
 * @brief Unlocks MPPA-256 sync module.
 */
static void mppa256_sync_unlock(void)
{
	hal_mutex_unlock(&lock);
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
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
static int sync_is_valid(int syncid)
{
	return ((syncid >= 0) && (syncid < HAL_NR_SYNC));
}

/*============================================================================*
 * hal_sync_create()                                                          *
 *============================================================================*/

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
int hal_sync_create(const int *nodes, int nnodes, int type)
{
	int fd;             /* NoC connector.           */
	int syncid;         /* Synchronization point.   */
	uint64_t mask;      /* Sync mask.               */
	char remotes[128];  /* IDs of remote NoC nodes. */
	char pathname[128]; /* NoC connector name.      */

	mppa256_sync_lock();

	/* Allocate a synchronization point. */
	if ((syncid = resource_alloc(&pool)) < 0)
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
			mask |= 1 << hal_get_node_num(nodes[i]);
	}

	/* Open NoC connector. */
	if ((fd = mppa_open(pathname, O_RDONLY)) == -1)
		goto error1;

	/* Setup sync mask. */
	if (mppa_ioctl(fd, MPPA_RX_SET_MATCH, ~mask) == -1)
		goto error2;

	/* Initialize synchronization point. */
	synctab[syncid].fd = fd;
	synctab[syncid].type = type;
	resource_set_rdonly(&synctab[syncid].resource);
	resource_set_notbusy(&synctab[syncid].resource);

	mppa256_sync_unlock();
	
	return (syncid);

error2:
	mppa_close(fd);
error1:
	resource_free(&pool, syncid);
error0:
	mppa256_sync_unlock();
	return (-EAGAIN);
}

/*============================================================================*
 * hal_sync_open()                                                            *
 *============================================================================*/

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
int hal_sync_open(const int *nodes, int nnodes, int type)
{
	int fd;                /* NoC connector.             */
	int syncid;            /* Synchronization point.     */
	char remotes[128];     /* IDs of remote NoC nodes.   */
	char pathname[128];    /* NoC connector name.        */

	mppa256_sync_lock();

	/* Allocate a synchronization point. */
	if ((syncid = resource_alloc(&pool)) < 0)
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

	/* Initialize synchronization point. */
	synctab[syncid].fd = fd;
	synctab[syncid].ncount = nnodes - 1;
	synctab[syncid].type = type;
	resource_set_wronly(&synctab[syncid].resource);
	resource_set_notbusy(&synctab[syncid].resource);

	mppa256_sync_unlock();

	return (syncid);

error1:
	resource_free(&pool, syncid);
error0:
	mppa256_sync_unlock();
	return (-EAGAIN);
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
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_sync_wait(int syncid)
{
	int ret;
	uint64_t mask;

	/* Invalid sync. */
	if (!sync_is_valid(syncid))
		goto error0;

again:

	mppa256_sync_lock();

		/* Bad sync. */
		if (!resource_is_used(&synctab[syncid].resource))
			goto error1;

		/* Bad sync. */
		if (!resource_is_readable(&synctab[syncid].resource))
			goto error1;

		/* Busy sync. */
		if (resource_is_busy(&synctab[syncid].resource))
		{
			mppa256_sync_unlock();
			goto again;
		}

		/* Set sync as busy. */
		resource_set_busy(&synctab[syncid].resource);

	/*
	 * Release lock, since we may sleep below.
	 */
	mppa256_sync_unlock();

	/* Wait. */
	ret = (mppa_read(synctab[syncid].fd, &mask, sizeof(uint64_t)) != sizeof(uint64_t));
	
	mppa256_sync_lock();
		resource_set_notbusy(&synctab[syncid].resource);
	mppa256_sync_unlock();

	return ((ret) ? -EAGAIN : 0);

error1:
	mppa256_sync_unlock();
error0:
	return (-EAGAIN);
}

/*============================================================================*
 * hal_sync_signal()                                                          *
 *============================================================================*/

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
int hal_sync_signal(int syncid)
{
	int ret;
	int nodeid;
	uint64_t mask;

	/* Invalid sync. */
	if (!sync_is_valid(syncid))
		goto error0;

again:

	mppa256_sync_lock();

		/* Bad sync. */
		if (!resource_is_used(&synctab[syncid].resource))
			goto error1;

		/* Bad sync. */
		if (!resource_is_writable(&synctab[syncid].resource))
			goto error1;

		/* Busy sync. */
		if (resource_is_busy(&synctab[syncid].resource))
		{
			mppa256_sync_unlock();
			goto again;
		}

		/* Set sync as busy. */
		resource_set_busy(&synctab[syncid].resource);

	/*
	 * Release lock, since we may sleep below.
	 */
	mppa256_sync_unlock();

	/* Broadcast. */
	if (synctab[syncid].type == HAL_SYNC_ONE_TO_ALL)
	{
		ret = 0;

		for (int i = 0; i < synctab[syncid].ncount; i++)
		{
			if (mppa_ioctl(synctab[syncid].fd, MPPA_TX_SET_RX_RANK, i) == -1)
				goto error2;

			/* Signal. */
			mask = ~0;
			if (mppa_write(synctab[syncid].fd, &mask, sizeof(uint64_t)) != sizeof(uint64_t))
				goto error2;
		}
	}

	/* Unicast. */
	else
	{
		nodeid = hal_get_node_id();

		/* Signal. */
		mask = 1 << hal_get_node_num(nodeid);
		ret = (mppa_write(synctab[syncid].fd, &mask, sizeof(uint64_t)) != sizeof(uint64_t));
	}

	mppa256_sync_lock();
		resource_set_notbusy(&synctab[syncid].resource);
	mppa256_sync_unlock();

	return ((ret) ? -EAGAIN : 0);

error2:
	printf("[PANIC] failed to release sync\n");
	while(1);
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
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_sync_close(int syncid)
{
	/* Invalid sync. */
	if (!sync_is_valid(syncid))
		goto error0;

again:

	mppa256_sync_lock();

		/* Bad sync. */
		if (!resource_is_used(&synctab[syncid].resource))
			goto error1;

		/* Bad sync. */
		if (resource_is_readable(&synctab[syncid].resource))
			goto error1;

		/* Busy sync. */
		if (resource_is_busy(&synctab[syncid].resource))
		{
			mppa256_sync_unlock();
			goto again;
		}

		if (mppa_close(synctab[syncid].fd) < 0)
			goto error1;

		resource_free(&pool, syncid);

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
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_sync_unlink(int syncid)
{
	/* Invalid sync. */
	if (!sync_is_valid(syncid))
		goto error0;

again:

	mppa256_sync_lock();

		/* Bad sync. */
		if (!resource_is_used(&synctab[syncid].resource))
			goto error1;

		/* Bad sync. */
		if (resource_is_writable(&synctab[syncid].resource))
			goto error1;

		/* Busy sync. */
		if (resource_is_busy(&synctab[syncid].resource))
		{
			mppa256_sync_unlock();
			goto again;
		}

		if (mppa_close(synctab[syncid].fd) < 0)
			goto error1;

		resource_free(&pool, syncid);

	mppa256_sync_unlock();

	return (0);

error1:
	mppa256_sync_unlock();
error0:
	return (-EAGAIN);
}
