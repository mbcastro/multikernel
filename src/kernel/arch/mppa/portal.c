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

#include <nanvix/klib.h>
#include <nanvix/hal.h>
#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include "mppa.h"

/*=======================================================================*
 * hal_portal_create()                                                   *
 *=======================================================================*/

/**
 * @brief Creates a portal.
 *
 * @param local     ID of the local NoC node.
 *
 * @returns Upon successful completion, the portal NoC Connector
 * of the new portal is returned.
 * Upon failure, a negative error code is returned instead.
 */
int hal_portal_create(int local)
{
	int fd;               /* Portal NoC Connector.       */
	char pathname[128];   /* NoC connector name.         */
	int noctag;           /* NoC tag used for transfers. */

#ifdef _HAS_NOC_GET_NODE_ID_
	/* Invalid node ID. */
	if (local != hal_get_node_id())
		return (-EINVAL);
#endif

	noctag = noctag_portal(local);

	/* Build pathname for NoC connector. */
	snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/portal/%d:%d",
			local,
			noctag
	);

	/* Open NoC connector. */
	if ((fd = mppa_open(pathname, O_RDONLY)) == -1)
		return (-EAGAIN);

	return (fd);
}

/*=======================================================================*
 * hal_portal_allow()                                                        *
 *=======================================================================*/

/**
 * @brief Enables read operations from a remote.
 *
 * @param portal    Adress of the target portal.
 * @param remote    Cluster ID of target remote.
 *
 * @returns Upons successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int hal_portal_allow(portal_t *portal, int remote)
{
	int local;          /* Local NoC node ID    */
	int sync_fd;        /* Sync NoC connector.  */
	char pathname[128]; /* Portal pathname.     */

	/* Invalid portal.*/
	if (portal == NULL)
		return (-EINVAL);

	/* Invalid remote. */
	if (!(k1_is_iocluster(remote) || k1_is_ccluster(remote)))
		return (-EINVAL);

	#ifdef _HAS_NOC_GET_NODE_ID_
		/* Invalid remote. */
		if (remote != hal_get_node_id())
			return (-EINVAL);
	#endif

	local = hal_get_cluster_id();

	/* Create underlying sync. */
	remote = (k1_is_ccluster(remote)) ?
		remote : remote + local%NR_IOCLUSTER_DMA;
	snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/sync/%d:%d",
			remote,
			(k1_is_ccluster(remote) || k1_is_ccluster(local)) ?
			                       noctag_portal(local) : 127);

	if ((sync_fd = mppa_open(pathname, O_WRONLY)) == -1)
		return (-EAGAIN);

	/* Initialize portal. */
	portal->remote = remote;
	portal->sync_fd = sync_fd;

	return (0);
}

/*=======================================================================*
 * hal_portal_open()                                                         *
 *=======================================================================*/

/**
 * @brief Opens a portal.
 *
 * @param remote     ID of the target NoC node.
 * @param portal     Adress where the portal will be stored
 *
 * @returns Upon successful completion 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
int hal_portal_open(int remote, portal_t *portal)
{
	int local;          /* ID of local NoC node.  */
	int portal_fd;      /* Portal NoC Connector.  */
	int sync_fd;        /* Sync NoC connector.    */
	char pathname[128]; /* NoC connector name.    */

	/* Invalid portal. */
	if (portal == NULL)
		return (-EINVAL);

	/* Invalid node ID. */
	if (remote < 0)
		return (-EINVAL);

#ifdef _HAS_NOC_GET_NODE_ID_
	/* Invalid node ID. */
	if (remote == hal_get_node_id())
		return (-EINVAL);
#endif

	local = hal_get_cluster_id();

	/* Build pathname for NoC connector. */
	snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/portal/%d:%d",
			remote,
			noctag_portal(remote)
	);

	/* Open NoC connector. */
	if ((portal_fd = mppa_open(pathname, O_WRONLY)) == -1)
		return (-EAGAIN);

	/* Build pathname for NoC connector. */
	local = (k1_is_ccluster(local)) ?
	local : local + remote%NR_IOCLUSTER_DMA;
	snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/sync/%d:%d",
			local,
			(k1_is_ccluster(remote) || k1_is_ccluster(local)) ?
			                        noctag_portal(remote) : 127
	);

	/* Open NoC connector. */
	if ((sync_fd = mppa_open(pathname, O_RDONLY)) == -1)
		return (-EAGAIN);

	portal->portal_fd = portal_fd;
	portal->sync_fd = sync_fd;
	portal->remote = remote;

	return (0);
}

/*=======================================================================*
 * portal_sync()                                                         *
 *=======================================================================*/

/**
 * @brief Builds sync mask for a dma.
 *
 * @param dma Target dma.
 *
 * @return Sync mask.
 */
static inline uint64_t portal_sync(int dma)
{
	if ((dma >= IOCLUSTER0) && (dma < IOCLUSTER0 + NR_IOCLUSTER_DMA))
		return (1 << (CCLUSTER15 + 1 + dma%NR_IOCLUSTER_DMA));
	else if ((dma >= IOCLUSTER1) && (dma < IOCLUSTER1 + NR_IOCLUSTER_DMA))
		return (1 << (CCLUSTER15 + 1 + NR_IOCLUSTER_DMA + dma%NR_IOCLUSTER_DMA));
	return (1 << dma);
}

/*=======================================================================*
 * hal_portal_read()                                                         *
 *=======================================================================*/

/**
 * @brief Reads data from a portal.
 *
 * @param portal  Targeted adress portal.
 * @param buf     Location from where data should be written.
 * @param n       Number of bytes to read.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int hal_portal_read(portal_t *portal, void *buf, size_t n)
{
	uint64_t mask;
	mppa_aiocb_t aiocb;
	int local;             /* Local NoC node ID. */

	/* Invalid portal ID.*/
	if (portal == NULL)
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid read size. */
	if (n < 1)
		return (-EINVAL);

	/* Setup read operation. */
	mppa_aiocb_ctor(&aiocb, portal->portal_fd, buf, n);
	assert(mppa_aio_read(&aiocb) != -1);

	local = hal_get_cluster_id();

	/* Unblock remote. */
	mask = portal_sync(local);
	assert(mppa_write(portal->sync_fd, &mask, sizeof(uint64_t)) != -1);

	/* Wait read operation to complete. */
	assert(mppa_aio_wait(&aiocb) == n);
	mppa_close(portal->sync_fd);

	return (0);
}

/*=======================================================================*
 * hal_portal_write()                                                        *
 *=======================================================================*/

/**
 * @brief Writes data to a portal.
 *
 * @param portal  Targeted adress portal.
 * @param buf     Location from where data should be read.
 * @param n       Number of bytes to write.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int hal_portal_write(portal_t *portal, const void *buf, size_t n)
{
	uint64_t mask;

	/* Invalid portal.*/
	if (portal == NULL)
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid write size. */
	if (n < 1)
		return (-EINVAL);

	/* Wait for remote to be ready. */
	mask = portal_sync(portal->remote);
	assert(mppa_ioctl(portal->sync_fd, MPPA_RX_SET_MATCH, ~mask) != -1);
	assert(mppa_read(portal->sync_fd, &mask, sizeof(uint64_t)) != -1);

	/* Write. */
	assert(mppa_pwrite(portal->portal_fd, buf, n, 0) == n);

	return (0);
}

/*=======================================================================*
 * hal_portal_close()                                                        *
 *=======================================================================*/

/**
 * @brief Closes a portal.
 *
 * @param portal     Adress of the target portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int hal_portal_close(portal_t *portal)
{
	/* Invalid portal.*/
	if (portal == NULL)
		return (-EINVAL);

	mppa_close(portal->sync_fd);

	return (0);
}

/*=======================================================================*
 * portal_unlink()                                                       *
 *=======================================================================*/

/**
 * @brief Destroys a portal.
 *
 * @param portalid ID of the target portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int hal_portal_unlink(int portal_fd)
{
	/*  Invalid portal. */
	if (portal_fd < 0)
		return (-EINVAL);

	mppa_close(portal_fd);

	return (0);
}
