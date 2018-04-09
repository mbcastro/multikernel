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

#include <nanvix/arch/mppa.h>
#include <nanvix/klib.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Number of portals.
 */
#define NR_PORTAL 256

/**
 * @brief Portal flags.
 */
/**@{*/
#define PORTAL_USED   (1 << 0)
#define PORTAL_WRONLY (1 << 1)
/**@}*/

/**
 * @brief Portal.
 */
struct portal
{
	int dma_local;  /**< ID of local DMA.      */
	int dma_remote; /**< Cluster ID of remote. */
	int portal_fd;  /**< Portal NoC connector. */
	int sync_fd;    /**< Sync connector.       */
	int flags;      /**< Flags.                */
};

/**
 * @brief table of portals.
 */
static struct portal portals[NR_PORTAL];

/*=======================================================================*
 * portal_alloc()                                                        *
 *=======================================================================*/

/**
 * @brief Allocates a portal.
 *
 * @return Upon successful completion the ID of the newly allocated
 * portal is returned. Upon failure, a negative error code is returned
 * instead.
 */
static int portal_alloc(void)
{
	/* Search for a free portal. */
	for (int i = 0; i < NR_PORTAL; i++)
	{
		/* Found. */
		if (!(portals[i].flags & PORTAL_USED))
		{
			portals[i].flags |= PORTAL_USED;
			return (i);
		}
	}

	return (-ENOENT);
}

/*=======================================================================*
 * portal_free()                                                         *
 *=======================================================================*/

/**
 * @brief Frees a portal.
 *
 * @param portalid ID of the target portal.
 */
static void portal_free(int portalid)
{
	/* Sanity check. */
	assert((portalid >= 0) && (portalid < NR_PORTAL));
	assert(portals[portalid].flags & PORTAL_USED);

	portals[portalid].flags = 0;
	mppa_close(portals[portalid].portal_fd);
}

/*=======================================================================*
 * portal_noctag()                                                       *
 *=======================================================================*/

/**
 * @brief Computes the portal NoC tag for a cluster.
 *
 * @param local Id of target cluster.
 */
static int portal_noctag(int local)
{
	if ((local >= CCLUSTER0) && (local <= CCLUSTER15))
		return (64 + local);
	else if ((local >= IOCLUSTER0) && (local < (IOCLUSTER0 + NR_IOCLUSTER_DMA)))
		return (64 + 16 + 0);
	else if ((local >= IOCLUSTER1) && (local < (IOCLUSTER1 + NR_IOCLUSTER_DMA)))
		return (64 + 16 + 0);

	return (0);
}

/*=======================================================================*
 * portal_create()                                                       *
 *=======================================================================*/

/**
 * @brief Creates a portal.
 *
 * @param name Portal name.
 *
 * @returns Upon successful completion, the ID of the new portal is
 * returned. Upon failure, a negative error code is returned instead.
 */
int portal_create(const char *name)
{
	int local;          /* ID of local cluster.  */
	int dma_local;      /* ID of local DMA.      */
	int portal_fd;      /* Portal NoC Connector. */
	int portalid;       /* ID of mailbix.        */
	char pathname[128]; /* NoC connector name.   */

	/* Invalid portal name. */
	if (name == NULL)
		return (-EINVAL);

	local = k1_get_cluster_id();

	/* Invalid cluster name. */
	assert(name_cluster_id(name) == local);

	/* Allocate a portal. */
	portalid = portal_alloc();
	if (portalid < 0)
		return (-ENOENT);

	/* Create underlying portal. */
	dma_local = name_cluster_dma(name);
	snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/portal/%d:%d",
			dma_local,
			portal_noctag(dma_local)
	);
	assert((portal_fd = mppa_open(pathname, O_RDONLY)) != -1);

	/* Initialize portal. */
	portals[portalid].dma_local = dma_local;
	portals[portalid].dma_remote = -1;
	portals[portalid].portal_fd = portal_fd;
	portals[portalid].sync_fd = -1;
	portals[portalid].flags &= ~(PORTAL_WRONLY);

	return (portalid);
}

/*=======================================================================*
 * portal_allow()                                                        *
 *=======================================================================*/

/**
 * @brief Enables read operations from a remote.
 *
 * @param portalid  Target portal.
 * @param remote    Cluster ID of target remote.
 *
 * @returns Upons successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int portal_allow(int portalid, int remote)
{
	int local;          /* ID of local cluster. */
	int sync_fd;        /* Sync NoC connector.  */
	int dma_remote;     /* ID of remote DMA.    */
	char pathname[128]; /* Portal pathname.     */

	/* Invalid portal ID.*/
	if ((portalid < 0) || (portalid >= NR_PORTAL))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!(portals[portalid].flags & PORTAL_USED))
		return (-EINVAL);

	/* Invalid portal. */
	if (portals[portalid].flags & PORTAL_WRONLY)
		return (-EINVAL);

	/* Invalid remote. */
	if (!(k1_is_iocluster(remote) || k1_is_ccluster(remote)))
		return (-EINVAL);

	local = k1_get_cluster_id();

	/* Invalid remote. */
	if (remote == local)
		return (-EINVAL);

	/* Create underlying sync. */
	dma_remote = (k1_is_ccluster(remote)) ?
		remote : remote + local%NR_IOCLUSTER_DMA;
	snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/sync/%d:%d",
			dma_remote,
			(k1_is_ccluster(remote)) ? 
				portal_noctag(dma_remote) : 
				portal_noctag(dma_remote) + portals[portalid].dma_local
	);
	assert((sync_fd = mppa_open(pathname, O_WRONLY)) != -1);

	/* Initialize portal. */
	portals[portalid].dma_remote = dma_remote;
	portals[portalid].sync_fd = sync_fd;

	return (0);
}

/*=======================================================================*
 * portal_open()                                                         *
 *=======================================================================*/

/**
 * @brief Opens a portal.
 *
 * @param name Portal name.
 *
 * @returns Upon successful completion, the ID of the target portal is
 * returned. Upon failure, a negative error code is returned instead.
 */
int portal_open(const char *name)
{
	int local;          /* ID of local cluster.  */
	int remote;         /* ID of remote cluster. */
	int dma_remote;     /* ID of remote DMA.     */
	int dma_local;      /* ID of local DMA.      */
	int portal_fd;      /* Portal NoC Connector. */
	int sync_fd;        /* Sync NoC connector.   */
	int portalid;       /* ID of mailbix.        */
	char pathname[128]; /* NoC connector name.   */

	/* Invalid portal name. */
	if (name == NULL)
		return (-EINVAL);

	local = k1_get_cluster_id();
	remote = name_cluster_id(name);

	/* Invalid cluster name. */
	assert(remote != local);

	/* Allocate a portal. */
	portalid = portal_alloc();
	if (portalid < 0)
		return (-ENOENT);

	/* Create underlying portal. */
	dma_remote = name_cluster_dma(name);
	snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/portal/%d:%d",
			dma_remote,
			portal_noctag(dma_remote)
	);
	assert((portal_fd = mppa_open(pathname, O_WRONLY)) != -1);

	/* Create underlying sync. */
	dma_local = (k1_is_ccluster(local)) ?
		local : local + remote%NR_IOCLUSTER_DMA;
	snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/sync/%d:%d",
			dma_local,
			(k1_is_ccluster(local)) ? 
				portal_noctag(dma_local) : 
				portal_noctag(dma_local) + dma_remote
	);
	assert((sync_fd = mppa_open(pathname, O_RDONLY)) != -1);

	/* Initialize portal. */
	portals[portalid].dma_local = dma_local;
	portals[portalid].dma_remote = dma_remote;
	portals[portalid].portal_fd = portal_fd;
	portals[portalid].sync_fd = sync_fd;
	portals[portalid].flags |= PORTAL_WRONLY;

	return (portalid);
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
 * portal_read()                                                         *
 *=======================================================================*/

/**
 * @brief Reads data from a portal.
 *
 * @param portalid ID of the target portal.
 * @param buf   Location from where data should be written.
 * @param n     Number of bytes to read.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int portal_read(int portalid, void *buf, size_t n)
{
	uint64_t mask;
	mppa_aiocb_t aiocb;

	/* Invalid portal ID.*/
	if ((portalid < 0) || (portalid >= NR_PORTAL))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!(portals[portalid].flags & PORTAL_USED))
		return (-EINVAL);

	/* Operation no supported. */
	if (portals[portalid].flags & PORTAL_WRONLY)
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid read size. */
	if (n < 1)
		return (-EINVAL);

	/* Setup read operation. */
	mppa_aiocb_ctor(&aiocb, portals[portalid].portal_fd, buf, n);
	assert(mppa_aio_read(&aiocb) != -1);

	/* Unblock remote. */
	mask = portal_sync(portals[portalid].dma_local);
	assert(mppa_write(portals[portalid].sync_fd, &mask, sizeof(uint64_t)) != -1);
	mppa_close(portals[portalid].sync_fd);

	/* Wait read operation to complete. */
	assert(mppa_aio_wait(&aiocb) == n);

	return (0);
}

/*=======================================================================*
 * portal_write()                                                        *
 *=======================================================================*/

/**
 * @brief Writes data to a portal.
 *
 * @param portalid ID of the target portal.
 * @param buf   Location from where data should be read.
 * @param n     Number of bytes to write.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int portal_write(int portalid, const void *buf, size_t n)
{
	uint64_t mask;

	/* Invalid portal ID.*/
	if ((portalid < 0) || (portalid >= NR_PORTAL))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!(portals[portalid].flags & PORTAL_USED))
		return (-EINVAL);

	/* Operation no supported. */
	if (!(portals[portalid].flags & PORTAL_WRONLY))
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid write size. */
	if (n < 1)
		return (-EINVAL);

	/* Wait for remote to be ready. */
	mask = portal_sync(portals[portalid].dma_remote);
	assert(mppa_ioctl(portals[portalid].sync_fd, MPPA_RX_SET_MATCH, ~mask) != -1);
	assert(mppa_read(portals[portalid].sync_fd, &mask, sizeof(uint64_t)) != -1);

	/* Write. */
	assert(mppa_pwrite(portals[portalid].portal_fd, buf, n, 0) == n);

	return (0);
}

/*=======================================================================*
 * portal_close()                                                        *
 *=======================================================================*/

/**
 * @brief Closes a portal.
 *
 * @param portalid ID of the target portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int portal_close(int portalid)
{
	/* Invalid portal ID.*/
	if ((portalid < 0) || (portalid >= NR_PORTAL))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!(portals[portalid].flags & PORTAL_USED))
		return (-EINVAL);

	/* Invalid portal. */
	if (!(portals[portalid].flags & PORTAL_WRONLY))
		return (-EINVAL);

	portal_free(portalid);
	mppa_close(portals[portalid].sync_fd);

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
int portal_unlink(int portalid)
{
	/* Invalid portal ID.*/
	if ((portalid < 0) || (portalid >= NR_PORTAL))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!(portals[portalid].flags & PORTAL_USED))
		return (-EINVAL);

	/* Invalid portal. */
	if (portals[portalid].flags & PORTAL_WRONLY)
		return (-EINVAL);

	portal_free(portalid);

	return (0);
}
