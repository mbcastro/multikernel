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

#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>

#define __NEED_HAL_NOC_
#define __NEED_HAL_CORE_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_MUTEX_
#include <hal.h>
#include <klib.h>
#include <resource.h>

/**
 * @brief Table of synchronization points.
 */
static struct sync
{
	struct resource rsrc;        /**< Underlying resource.            */
	int type;                    /**< Type.                           */
	mqd_t fd;                    /**< Underlying file descriptor.     */
	int ncount;                  /**< Number of remotes in broadcast. */
	char pathname[128];          /**< Name of underlying mqueue.      */
	int nodes[HAL_NR_NOC_NODES]; /**< IDs of attached nodes.          */
} synctab[HAL_NR_SYNC];

/**
 * @brief Resource pool for synchronization points.
 */
static const struct resource_pool pool = {
	synctab,
	HAL_NR_SYNC,
	sizeof(struct sync)}
;

/**
 * @brief Sync module lock.
 */
static hal_mutex_t lock = HAL_MUTEX_INITIALIZER;

/**
 * @brief Default message queue attribute.
 */
static struct mq_attr mq_attr = {
	.mq_maxmsg = HAL_NR_NOC_NODES,
	.mq_msgsize = sizeof(int)
};

/*============================================================================*
 * unix_sync_lock()                                                           *
 *============================================================================*/

/**
 * @brief Locks Unix sync module.
 */
static void unix_sync_lock(void)
{
	hal_mutex_lock(&lock);
}

/*============================================================================*
 * unix_sync_unlock()                                                         *
 *============================================================================*/

/**
 * @brief Unlocks Unix sync module.
 */
static void unix_sync_unlock(void)
{
	hal_mutex_unlock(&lock);
}

/*============================================================================*
 * hal_sync_is_valid()                                                        *
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
int hal_sync_is_valid(int syncid)
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
	int fd;         /* NoC connector.         */
	int syncid;     /* Synchronization point. */
	char *pathname; /* NoC connector name.    */

	unix_sync_lock();

	/* Allocate a synchronization point. */
	if ((syncid = resource_alloc(&pool)) < 0)
		goto error0;

	pathname = synctab[syncid].pathname;

	/* Broadcast. */
	if (type == HAL_SYNC_ONE_TO_ALL)
	{
		/* Build pathname for NoC connector. */
		sprintf(pathname,
			"/sync-%d-broadcast",
			nodes[0]
		);
	}

	else
	{
		/* Build pathname for NoC connector. */
		sprintf(pathname,
			"/sync-%d-gather",
			nodes[0]
		);
	}

	/* Open NoC connector. */
	if ((fd = mq_open(pathname, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR, &mq_attr)) == -1)
		goto error1;

	/* Initialize synchronization point. */
	synctab[syncid].fd = fd;
	synctab[syncid].ncount = nnodes;
	memcpy(synctab[syncid].nodes, nodes, nnodes*sizeof(int));
	synctab[syncid].type = type;
	resource_set_rdonly(&synctab[syncid].rsrc);
	resource_set_notbusy(&synctab[syncid].rsrc);

	unix_sync_unlock();

	return (syncid);

error1:
	resource_free(&pool, syncid);
error0:
	unix_sync_unlock();
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
	int fd;         /* NoC connector.         */
	int syncid;     /* Synchronization point. */
	char *pathname; /* NoC connector name.    */

	unix_sync_lock();

	/* Allocate a synchronization point. */
	if ((syncid = resource_alloc(&pool)) < 0)
		goto error0;

	pathname = synctab[syncid].pathname;

	/* Broadcast. */
	if (type == HAL_SYNC_ONE_TO_ALL)
	{
		/* Build pathname for NoC connector. */
		sprintf(pathname,
			"/sync-%d-broadcast",
			nodes[0]
		);
	}

	else
	{
		/* Build pathname for NoC connector. */
		sprintf(pathname,
			"/sync-%d-gather",
			nodes[0]
		);
	}

	/* Open NoC connector. */
	if ((fd = mq_open(pathname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR, &mq_attr)) == -1)
		goto error1;

	/* Initialize synchronization point. */
	synctab[syncid].fd = fd;
	synctab[syncid].ncount = nnodes;
	memcpy(synctab[syncid].nodes, nodes, nnodes*sizeof(int));
	synctab[syncid].type = type;
	resource_set_wronly(&synctab[syncid].rsrc);
	resource_set_notbusy(&synctab[syncid].rsrc);

	unix_sync_unlock();

	return (syncid);

error1:
	resource_free(&pool, syncid);
error0:
	unix_sync_unlock();
	return (-EAGAIN);
}

/*============================================================================*
 * hal_sync_wait()                                                            *
 *============================================================================*/

/**
 * @brief Waits for a signal.
 *
 * @param syncid ID of the target sync.
 * @param sig    Place where the received signal should be stored.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int unix_sync_wait(int syncid, int *sig)
{
	if (mq_receive(synctab[syncid].fd, (char *)sig, sizeof(int), NULL) == -1)
		return (-EAGAIN);

	return (0);
}

/**
 * @brief Waits for a broadcast signal.
 *
 * @param syncid ID of the target sync.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static inline int unix_sync_wait_broadcast(int syncid)
{
	int sig;
	int ret = 0;

	if ((ret = unix_sync_wait(syncid, &sig)) == 0)
	{
		if (sig != synctab[syncid].nodes[0])
			ret = -EAGAIN;
	}

	return (ret);
}


/**
 * @brief Gathers broadcast signals.
 *
 * @param syncid ID of the target sync.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int unix_sync_wait_gather(int syncid)
{
	int nsignals = synctab[syncid].ncount - 1;
	int signals[nsignals];

	memset(signals, 0, nsignals*sizeof(int));

	/* Wait. */
	do
	{
		int err;
		int sig;

		if ((err = unix_sync_wait(syncid, &sig)) < 0)
			return (err);

		/* Check if node is in the list. */
		err = -EINVAL;
		for (int i = 1; i < synctab[syncid].ncount; i++)
		{
			/* Not this node. */
			if (synctab[syncid].nodes[i] != sig)
				continue;

			if (signals[i - 1] == 0)
			{
				signals[i - 1] = 1;
				nsignals--;
				err = 0;
				break;
			}
		}

		KASSERT(err == 0);
	} while (nsignals > 0);

	return (0);
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
int hal_sync_wait(int syncid)
{
	int ret;

	/* Invalid sync. */
	if (!hal_sync_is_valid(syncid))
		goto error0;

again:

	unix_sync_lock();

		/* Bad sync. */
		if (!resource_is_used(&synctab[syncid].rsrc))
			goto error1;

		/* Bad sync. */
		if (resource_is_writable(&synctab[syncid].rsrc))
			goto error1;

		/* Busy sync. */
		if (resource_is_busy(&synctab[syncid].rsrc))
		{
			unix_sync_unlock();
			goto again;
		}

		/* Set sync as busy. */
		resource_set_busy(&synctab[syncid].rsrc);

	/*
	 * Release lock, since we may sleep below.
	 */
	unix_sync_unlock();

	/* Broadcast. */
	ret = (synctab[syncid].type == HAL_SYNC_ONE_TO_ALL) ?
		unix_sync_wait_broadcast(syncid) :
		unix_sync_wait_gather(syncid);

	unix_sync_lock();
		resource_set_notbusy(&synctab[syncid].rsrc);
	unix_sync_unlock();

	return ((ret == -1) ? -EAGAIN : 0);

error1:
	unix_sync_unlock();
error0:
	return (-EAGAIN);
}

/*============================================================================*
 * hal_sync_signal()                                                          *
 *============================================================================*/

/**
 * @brief Sends a signal.
 *
 * @param syncid ID of the target sync.
 * @param sig    Signal.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static inline int unix_sync_signal(int syncid, int sig)
{
	return (mq_send(synctab[syncid].fd, (char *) &sig, sizeof(int), 1));
}

/**
 * @brief Broadcasts a signal.
 *
 * @param syncid ID of the target sync.
 * @param sig    Signal.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static inline int unix_sync_signal_broadcast(int syncid, int sig)
{
	int ret = 0;

	for (int i = 1; i < synctab[syncid].ncount; i++)
	{
		if ((ret = unix_sync_signal(syncid, sig)) == -1)
			break;
	}

	return (ret);
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
int hal_sync_signal(int syncid)
{
	int ret;
	int nodeid;

	/* Invalid sync. */
	if (!hal_sync_is_valid(syncid))
		goto error0;

again:

	unix_sync_lock();

		/* Bad sync. */
		if (!resource_is_used(&synctab[syncid].rsrc))
			goto error1;

		/* Bad sync. */
		if (!resource_is_writable(&synctab[syncid].rsrc))
			goto error1;

		/* Busy sync. */
		if (resource_is_busy(&synctab[syncid].rsrc))
		{
			unix_sync_unlock();
			goto again;
		}

		/* Set sync as busy. */
		resource_set_busy(&synctab[syncid].rsrc);

	/*
	 * Release lock, since we may sleep below.
	 */
	unix_sync_unlock();

	nodeid = hal_get_node_id();

	/* Broadcast. */
	ret = (synctab[syncid].type == HAL_SYNC_ONE_TO_ALL) ?
		unix_sync_signal_broadcast(syncid, nodeid) :
		unix_sync_signal(syncid, nodeid);

	unix_sync_lock();
		resource_set_notbusy(&synctab[syncid].rsrc);
	unix_sync_unlock();

	return ((ret == -1) ? -EAGAIN : 0);

error1:
	unix_sync_unlock();
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
	if (!hal_sync_is_valid(syncid))
		goto error0;

again:

	unix_sync_lock();

		/* Bad sync. */
		if (!resource_is_used(&synctab[syncid].rsrc))
			goto error1;

		/* Bad sync. */
		if (!resource_is_writable(&synctab[syncid].rsrc))
			goto error1;

		/* Busy sync. */
		if (resource_is_busy(&synctab[syncid].rsrc))
		{
			unix_sync_unlock();
			goto again;
		}

		if (mq_close(synctab[syncid].fd) < 0)
			goto error1;

		resource_free(&pool, syncid);

	unix_sync_unlock();

	return (0);

error1:
	unix_sync_unlock();
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
	if (!hal_sync_is_valid(syncid))
		goto error0;

again:

	unix_sync_lock();

		/* Bad sync. */
		if (!resource_is_used(&synctab[syncid].rsrc))
			goto error1;

		/* Bad sync. */
		if (resource_is_writable(&synctab[syncid].rsrc))
			goto error1;

		/* Busy sync. */
		if (resource_is_busy(&synctab[syncid].rsrc))
		{
			unix_sync_unlock();
			goto again;
		}

		/* Destroy underlying message queue. */
		if (mq_close(synctab[syncid].fd) < 0)
			goto error1;
		mq_unlink(synctab[syncid].pathname);

		resource_free(&pool, syncid);

	unix_sync_unlock();

	return (0);

error1:
	unix_sync_unlock();
error0:
	return (-EAGAIN);
}
