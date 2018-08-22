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

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_PORTAL_
#include <nanvix/hal.h>
#include <nanvix/klib.h>

#include "noc.h"

/**
 * @brief Portal flags.
 */
/**@{*/
#define PORTAL_FLAGS_USED   (1 << 0) /**< Used portal?     */
#define PORTAL_FLAGS_WRONLY (1 << 1) /**< Write only mode? */
#define PORTAL_FLAGS_BUSY   (1 << 2) /**< Busy?            */
/**@}*/

/**
 * @brief Table of portals.
 */
static struct
{
	int flags;     /**< Flags.                */
	int portal_fd; /**< Portal NoC connector. */
	int sync_fd;   /**< Sync NoC connector.   */
	int remote;    /**< Remote NoC node ID.   */
	int local;     /**< Local NoC node ID.    */
} portals[HAL_NR_PORTAL];

/**
 * @brief Portal module lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*============================================================================*
 * mppa256_portal_lock()                                                      *
 *============================================================================*/

/**
 * @brief Locks MPPA-256 portal module.
 */
static void mppa256_portal_lock(void)
{
	pthread_mutex_lock(&lock);
}

/*============================================================================*
 * mppa256_portal_unlock()                                                    *
 *============================================================================*/

/**
 * @brief Unlocks MPPA-256 portal module.
 */
static void mppa256_portal_unlock(void)
{
	pthread_mutex_unlock(&lock);
}

/*============================================================================*
 * portal_is_valid()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a portal is valid.
 *
 * @param portalid ID of the target portal.
 *
 * @returns One if the target portal is valid, and false
 * otherwise.
 *
 * @note This function is thread-safe.
 */
static int portal_is_valid(int portalid)
{
	return ((portalid >= 0) && (portalid < HAL_NR_PORTAL));
}

/*============================================================================*
 * portal_is_used()                                                           *
 *============================================================================*/

/**
 * @brief Asserts whether or not a portal is used.
 *
 * @param portalid ID of the target portal.
 *
 * @returns One if the target portal is used, and false
 * otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static int portal_is_used(int portalid)
{
	return (portals[portalid].flags & PORTAL_FLAGS_USED);
}

/*============================================================================*
 * portal_is_wronly()                                                         *
 *============================================================================*/

/**
 * @brief Asserts whether of not a portal is a write-only.
 *
 * @param portalid ID of the target portal.
 *
 * @returns One if the target portal is write-only, and
 * false otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static int portal_is_wronly(int portalid)
{
	return (portals[portalid].flags & PORTAL_FLAGS_WRONLY);
}

/*============================================================================*
 * portal_set_used()                                                          *
 *============================================================================*/

/**
 * @brief Sets a portal as used.
 *
 * @param portalid ID of the target portal.
 *
 * @note This function is @b NOT thread safe.
 */
static void portal_set_used(int portalid)
{
	portals[portalid].flags |= PORTAL_FLAGS_USED;
}

/*============================================================================*
 * portal_set_wronly()                                                        *
 *============================================================================*/

/**
 * @brief Sets a portal as a write-only.
 *
 * @param portalid ID of the target portal.
 *
 * @note This function is @b NOT thread safe.
 */
static void portal_set_wronly(int portalid)
{
	portals[portalid].flags |= PORTAL_FLAGS_WRONLY;
}

/*============================================================================*
 * portal_is_busy()                                                            *
 *============================================================================*/

/**
 * @brief Asserts whether or not a portal point is busy.
 *
 * @param portalid ID of the target portal point.
 *
 * @returns One if the target portal point is busy one, and false
 * otherwise.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static int portal_is_busy(int portalid)
{
	return (portals[portalid].flags & PORTAL_FLAGS_BUSY);
}

/*============================================================================*
 * portal_set_busy()                                                            *
 *============================================================================*/

/**
 * @brief Sets a portal point as busy.
 *
 * @param portalid ID of the target portal.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static void portal_set_busy(int portalid)
{
	portals[portalid].flags |= PORTAL_FLAGS_BUSY;
}

/*============================================================================*
 * portal_clear_busy()                                                        *
 *============================================================================*/

/**
 * @brief Clears the busy flag of a portal.
 *
 * @param portalid ID of the target portal.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static void portal_clear_busy(int portalid)
{
	portals[portalid].flags &= ~PORTAL_FLAGS_BUSY;
}

/*============================================================================*
 * portal_clear_flags()                                                       *
 *============================================================================*/

/**
 * @brief Clears the flags of a portal.
 *
 * @param portalid ID of the target portal.
 *
 * @note This function is @b NOT thread safe.
 */
static void portal_clear_flags(int portalid)
{
	portals[portalid].flags = 0;
}

/*============================================================================*
 * portal_alloc()                                                             *
 *============================================================================*/

/**
 * @brief Allocates a portal.
 *
 * @param nodeid ID of target NoC node.
 *
 * @returns Upon successful completion, the ID of a newly allocated
 * portal is returned. Upon failure, -1 is returned
 * instead.
 *
 * @note This function is @b NOT thread safe.
 */
static int portal_alloc(int nodeid)
{
	int portalid;

	/* Check for double allocation. */
	if (noc_is_cnode(hal_get_node_id()))
		portalid = hal_get_node_num(nodeid);
	else
	{
		portalid = hal_get_core_id()*HAL_NR_NOC_NODES + 
			hal_get_node_num(nodeid);
	}

	/* Allocate. */
	if (portal_is_used(portalid))
		return (-1);
	portal_set_used(portalid);

	return (portalid);
}

/*============================================================================*
 * portal_free()                                                              *
 *============================================================================*/

/**
 * @brief Frees a portal.
 *
 * @param portalid ID of the target portal.
 *
 * @note This function is @b NOT thread safe.
 */
static void portal_free(int portalid)
{
	portal_clear_flags(portalid);
}

/*============================================================================*
 * hal_portal_create()                                                        *
 *============================================================================*/

/**
 * @brief See hal_portal_create().
 */
static int mppa256_portal_create(int local)
{
	int portalid;       /* ID of  portal               */
	int fd;             /* Portal NoC Connector.       */
	char pathname[128]; /* NoC connector name.         */
	int noctag;         /* NoC tag used for transfers. */

	/* Allocate portal. */
	if ((portalid = portal_alloc(local)) < 0)
		goto error0;

	noctag = noctag_portal(local);

	/* Build pathname for NoC connector. */
	sprintf(pathname,
			"/mppa/portal/%d:%d",
			local,
			noctag
	);

	/* Open NoC connector. */
	if ((fd = mppa_open(pathname, O_RDONLY)) == -1)
		goto error1;

	portals[portalid].portal_fd = fd;
	portals[portalid].sync_fd = -1;
	portals[portalid].remote = -1;
	portals[portalid].local = local;
	portal_clear_busy(portalid);

	return (portalid);

error1:
	portal_free(portalid);
error0:
	return (-EAGAIN);
}

/**
 * @brief Creates a portal.
 *
 * @param local ID of the local NoC node.
 *
 * @returns Upon successful completion, the ID of a newly created
 * portal is returned. Upon failure, a negative error code is returned
 * instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_portal_create(int local)
{
	int portalid;

	/* Invalid local NoC node. */
	if (local < -1)
		return (-EINVAL);

	/* Bad local NoC node. */
	if (local != hal_get_node_id())
		return (-EINVAL);

	mppa256_portal_lock();
		portalid = mppa256_portal_create(local);
	mppa256_portal_unlock();

	return (portalid);
}

/*============================================================================*
 * hal_portal_allow()                                                         *
 *============================================================================*/

/**
 * @brief See hal_portal_allow().
 */
static int mppa256_portal_allow(int portalid, int local, int remote)
{
	int sync_fd;        /* Sync NoC connector. */
	char pathname[128]; /* Portal pathname.    */

	/* Create underlying sync. */
	sprintf(pathname,
			"/mppa/sync/%d:%d",
			remote,
			noctag_portal(local)
	);

	/* Open sync. */
	if ((sync_fd = mppa_open(pathname, O_WRONLY)) == -1)
		return (-EAGAIN);

	/* Initialize portal. */
	portals[portalid].remote = remote;
	portals[portalid].sync_fd = sync_fd;

	return (0);
}

/**
 * @brief Enables read operations from a remote.
 *
 * @param portalid ID of the target portal.
 * @param remote   NoC node ID of target remote.
 *
 * @returns Upons successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_portal_allow(int portalid, int remote)
{
	int ret;
	int local;

	/* Invalid portal.*/
	if (!portal_is_valid(portalid))
		return (-EINVAL);

	local = hal_get_node_id();

	/* Invalid remote. */
	if (remote == local)
		goto error0;

	/* Invalid remote. */
	if (remote < 0)
		goto error0;

again:

	mppa256_portal_lock();

		/* Bad portal. */
		if (!portal_is_used(portalid))
			goto error1;

		/* Bad portal. */
		if (portal_is_wronly(portalid))
			goto error1;

		/* Busy portal. */
		if (portal_is_busy(portalid))
		{
			mppa256_portal_unlock();
			goto again;
		}

		ret = mppa256_portal_allow(portalid, local, remote);

	mppa256_portal_unlock();

	return (ret);

error1:
	mppa256_portal_unlock();
error0:
	return (-EINVAL);
}

/*============================================================================*
 * hal_portal_open()                                                          *
 *============================================================================*/

/**
 * @brief See hal_portal_open().
 */
static int mppa256_portal_open(int local, int remote)
{
	int portalid;       /* ID of  portal         */
	int portal_fd;      /* Portal NoC Connector. */
	int sync_fd;        /* Sync NoC connector.   */
	char pathname[128]; /* NoC connector name.   */

	/* Allocate portal. */
	if ((portalid = portal_alloc(remote)) < 0)
		goto error0;

	/* Build pathname for NoC connector. */
	sprintf(pathname,
			"/mppa/portal/%d:%d",
			remote,
			noctag_portal(remote)
	);

	/* Open NoC connector. */
	if ((portal_fd = mppa_open(pathname, O_WRONLY)) == -1)
			goto error1;

	/* Build pathname for NoC connector. */
	sprintf(pathname,
			"/mppa/sync/%d:%d",
			local,
			noctag_portal(remote)
	);

	/* Open NoC connector. */
	if ((sync_fd = mppa_open(pathname, O_RDONLY)) == -1)
		goto error2;

	portals[portalid].portal_fd = portal_fd;
	portals[portalid].sync_fd = sync_fd;
	portals[portalid].remote = remote;
	portals[portalid].local = local;
	portal_set_wronly(portalid);
	portal_clear_busy(portalid);

	return (portalid);

error2:
	mppa_close(portal_fd);
error1:
	portal_free(portalid);
error0:
	return (-EAGAIN);
}

/**
 * @brief Opens a portal.
 *
 * @param remote ID of the target NoC node.
 *
 * @returns Upon successful completion, the ID of the target portal is
 * returned. Upon failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_portal_open(int remote)
{
	int local;
	int portalid;

	/* Invalid node ID. */
	if (remote < 0)
		return (-EINVAL);

	local = hal_get_node_id();

	/* Bad remote. */
	if (remote == local)
		return (-EINVAL);

	mppa256_portal_lock();
		portalid = mppa256_portal_open(local, remote);
	mppa256_portal_unlock();

	return (portalid);
}

/*============================================================================*
 * hal_portal_read()                                                          *
 *============================================================================*/

/**
 * @brief See hal_portal_read()
 */
static int mppa256_portal_read(int portalid, void *buf, size_t n)
{
	uint64_t mask;
	mppa_aiocb_t aiocb;
	size_t nread;

	/* Setup read operation. */
	mppa_aiocb_ctor(&aiocb, portals[portalid].portal_fd, buf, n);
	if (mppa_aio_read(&aiocb) == -1)
		goto error0;

	/* Unblock remote. */
	mask = 1 << hal_get_node_num(portals[portalid].local);
	if (mppa_write(portals[portalid].sync_fd, &mask, sizeof(uint64_t)) == -1)
		goto error0;

	/* Wait read operation to complete. */
	nread = mppa_aio_wait(&aiocb);
	mppa_close(portals[portalid].sync_fd);
	portals[portalid].sync_fd = -1;

	return (nread);

error0:
	return (-EAGAIN);
}

/**
 * @brief Reads data from a portal.
 *
 * @param portalid ID of target portal.
 * @param buf      Location from where data should be written.
 * @param n        Number of bytes to read.
 *
 * @returns Upon successful completion, the number of bytes read is
 * returned. Upon failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
ssize_t hal_portal_read(int portalid, void *buf, size_t n)
{
	int nread;

	/* Invalid portal ID.*/
	if (!portal_is_valid(portalid))
		goto error0;

	/* Invalid buffer. */
	if (buf == NULL)
		goto error0;

	/* Invalid read size. */
	if (n < 1)
		goto error0;

again:

	mppa256_portal_lock();

		/* Bad portal. */
		if (!portal_is_used(portalid))
			goto error1;

		/* Bad portal.*/
		if (portal_is_wronly(portalid))
			goto error1;

		/* Busy portal. */
		if (portal_is_busy(portalid))
		{
			mppa256_portal_unlock();
			goto again;
		}

		/* Set portal as busy. */
		portal_set_busy(portalid);

	/*
	 * Release lock, since we may sleep below.
	 */
	mppa256_portal_unlock();

	nread = mppa256_portal_read(portalid, buf, n);
	
	mppa256_portal_lock();
		portal_clear_busy(portalid);
	mppa256_portal_unlock();

	return (nread);

error1:
	mppa256_portal_unlock();
error0:
	return (-EINVAL);
}

/*============================================================================*
 * hal_portal_write()                                                         *
 *============================================================================*/

/**
 * @brief See hal_portal_write()
 */
static int mppa256_portal_write(int portalid, const void *buf, size_t n)
{
	uint64_t mask;
	size_t nwrite;

	/* Wait for remote to be ready. */
	mask = 1 << hal_get_node_num(portals[portalid].remote);
	if (mppa_ioctl(portals[portalid].sync_fd, MPPA_RX_SET_MATCH, ~mask) == -1)
		goto error0;

	if (mppa_read(portals[portalid].sync_fd, &mask, sizeof(uint64_t)) == -1)
		goto error0;

	/* Write. */
	nwrite = mppa_pwrite(portals[portalid].portal_fd, buf, n, 0);

	return (nwrite);

error0:
	return (-EAGAIN);
}

/**
 * @brief Writes data to a portal.
 *
 * @param portalid ID of target portal.
 * @param buf      Location from where data should be read.
 * @param n        Number of bytes to write.
 *
 * @returns Upon successful the number of bytes written is returned.
 * Upon failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
ssize_t hal_portal_write(int portalid, const void *buf, size_t n)
{
	int nwrite;

	/* Invalid portal ID.*/
	if (!portal_is_valid(portalid))
		goto error0;

	/* Invalid buffer. */
	if (buf == NULL)
		goto error0;

	/* Invalid write size. */
	if (n < 1)
		goto error0;

again:
	
	mppa256_portal_lock();

		/* Bad portal. */
		if (!portal_is_used(portalid))
			goto error1;

		/* Bad portal. */
		if (!portal_is_wronly(portalid))
			goto error1;

		/* Busy portal. */
		if (portal_is_busy(portalid))
		{
			mppa256_portal_unlock();
			goto again;
		}

		/* Set portal as busy. */
		portal_set_busy(portalid);

	/*
	 * Release lock, since we may sleep below.
	 */
	mppa256_portal_unlock();

	nwrite = mppa256_portal_write(portalid, buf, n);
	
	mppa256_portal_lock();
		portal_clear_busy(portalid);
	mppa256_portal_unlock();

	return (nwrite);

error1:
	mppa256_portal_unlock();
error0:
	return (-EINVAL);
}

/*============================================================================*
 * hal_portal_close()                                                         *
 *============================================================================*/

/**
 * @brief Closes a portal.
 *
 * @param portalid ID of target portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_portal_close(int portalid)
{
	/* Invalid portal.*/
	if (!portal_is_valid(portalid))
		goto error0;

again:

	mppa256_portal_lock();

		/* Bad portal. */
		if (!portal_is_used(portalid))
			goto error1;

		/* Bad portal. */
		if (!portal_is_wronly(portalid))
			goto error1;

		/* Busy portal. */
		if (portal_is_busy(portalid))
		{
			mppa256_portal_unlock();
			goto again;
		}

		/* Close underlying portal. */
		if (mppa_close(portals[portalid].portal_fd) < 0)
			goto error0;

		/* Close underlying sync connector. */
		if (mppa_close(portals[portalid].sync_fd) < 0)
			goto error2;

		portal_free(portalid);

	mppa256_portal_unlock();

	return (0);

error2:
	printf("[PANIC] failed cannot close portal\n");
	while (1);
error1:
	mppa256_portal_unlock();
error0:
	return (-EINVAL);
}

/*============================================================================*
 * hal_portal_unlink()                                                        *
 *============================================================================*/

/**
 * @brief Destroys a portal.
 *
 * @param portalid ID of the target portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_portal_unlink(int portalid)
{
	/* Invalid portal.*/
	if (!portal_is_valid(portalid))
		goto error0;

again:

	mppa256_portal_lock();

		/* Bad portal. */
		if (!portal_is_used(portalid))
			goto error1;

		/* Bad portal. */
		if (portal_is_wronly(portalid))
			goto error1;

		/* Busy portal. */
		if (portal_is_busy(portalid))
		{
			mppa256_portal_unlock();
			goto again;
		}

		/* Close underlying portal. */
		if (mppa_close(portals[portalid].portal_fd) < 0)
			goto error0;

		/* Close underlying sync connector. */
		if (portals[portalid].sync_fd != -1)
		{
			if (mppa_close(portals[portalid].sync_fd) < 0)
				goto error2;
		}

		portals[portalid].sync_fd = -1;
		portal_free(portalid);

	mppa256_portal_unlock();

	return (0);

error2:
	printf("[PANIC] failed cannot unlink portal\n");
	while (1);
error1:
	mppa256_portal_unlock();
error0:
	return (-EINVAL);
}
