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
	int remote;    /**< Cluster ID of remote.  */
	int portal_fd; /**< Portal NoC connector. */
	int sync_fd;   /**< Sync connector.       */
	int flags;     /**< Flags.                */
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
 * @param prtid ID of the target portal.
 */
static void portal_free(int prtid)
{
	/* Sanity check. */
	assert((prtid >= 0) && (prtid < NR_PORTAL));
	assert(portals[prtid].flags & PORTAL_USED);

	portals[prtid].flags = 0;
	mppa_close(portals[prtid].portal_fd);
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
	else if (local == IOCLUSTER0)
		return (64 + 16 + 0);
	else if (local == IOCLUSTER1)
		return (64 + 16 + 1);

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
	int local;          /* ID of local cluster.        */
	int portal_fd;      /* Portal NoC Connector.       */
	int prtid;          /* ID of mailbix.              */
	char pathname[128]; /* NoC connector name.         */
	int noctag;         /* NoC tag used for transfers. */

	/* Invalid portal name. */
	if (name == NULL)
		return (-EINVAL);

	local = name_lookup(name);
	assert(local == arch_get_cluster_id());

	/* Allocate a portal. */
	prtid = portal_alloc();
	if (prtid < 0)
		return (prtid);

	noctag = portal_noctag(local);
	
	/* Create underlying portal. */
	sprintf(pathname,
			"/mppa/portal/%d:%d",
			local,
			noctag
	);
	portal_fd = mppa_open(pathname, O_RDONLY);
	assert(portal_fd != -1);

	/* Initialize portal. */
	portals[prtid].portal_fd = portal_fd;
	portals[prtid].flags &= ~(PORTAL_WRONLY);

	return (prtid);
}

/*=======================================================================*
 * portal_allow()                                                        *
 *=======================================================================*/

/**
 * @brief Enables operations from a remote.
 *
 * @param prtid  target portal.
 * @param remote Remote to allow.
 *
 * @returns Upons successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int portal_allow(int prtid, int remote)
{
	int sync_fd;
	char pathname[128];

	/* Invalid portal ID.*/
	if ((prtid < 0) || (prtid >= NR_PORTAL))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!(portals[prtid].flags & PORTAL_USED))
		return (-EINVAL);

	/* Invalid portal. */
	if (portals[prtid].flags & PORTAL_WRONLY)
		return (-EINVAL);

	/* Invalid remote. */
	if (!(((remote >= CCLUSTER0) && (remote <= CCLUSTER15)) || 
		  (remote == IOCLUSTER0)                           || 
		  (remote == IOCLUSTER1)))
	{
		return (-EINVAL);
	}

	/* Invalid remote. */
	if (remote == arch_get_cluster_id())
		return (-EINVAL);

	/* Create underlying sync. */
	sprintf(pathname,
			"/mppa/sync/%d:%d",
			remote,
			portal_noctag(remote)
	);
	sync_fd = mppa_open(pathname, O_WRONLY);
	assert(sync_fd != -1);

	portals[prtid].remote = remote;
	portals[prtid].sync_fd = sync_fd;

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
	int local;          /* ID of local cluster.        */
	int portal_fd;      /* Portal NoC Connector.       */
	int sync_fd;        /* Sync NoC connector.         */
	int prtid;          /* ID of mailbix.              */
	char pathname[128]; /* NoC connector name.         */
	int noctag;         /* NoC tag used for transfers. */

	/* Invalid portal name. */
	if (name == NULL)
		return (-EINVAL);

	local = name_lookup(name);
	assert(local != arch_get_cluster_id());

	/* Allocate a portal. */
	prtid = portal_alloc();
	if (prtid < 0)
		return (prtid);

	noctag = portal_noctag(local);
	
	/* Create underlying portal. */
	sprintf(pathname,
			"/mppa/portal/%d:%d",
			local,
			noctag
	);
	portal_fd = mppa_open(pathname, O_WRONLY);
	assert(portal_fd != -1);

	/* Create underlying sync. */
	sprintf(pathname,
			"/mppa/sync/%d:%d",
			arch_get_cluster_id(),
			portal_noctag(arch_get_cluster_id())
	);
	sync_fd = mppa_open(pathname, O_RDONLY);
	assert(sync_fd != -1);

	/* Initialize portal. */
	portals[prtid].remote = local;
	portals[prtid].portal_fd = portal_fd;
	portals[prtid].sync_fd = sync_fd;
	portals[prtid].flags |= PORTAL_WRONLY;

	return (prtid);
}

/*=======================================================================*
 * portal_read()                                                         *
 *=======================================================================*/

/**
 * @brief Reads data from a portal.
 *
 * @param prtid ID of the target portal.
 * @param buf   Location from where data should be written.
 * @param n     Number of bytes to read.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int portal_read(int prtid, void *buf, size_t n)
{
	uint64_t mask;
	mppa_aiocb_t aiocb;

	/* Invalid portal ID.*/
	if ((prtid < 0) || (prtid >= NR_PORTAL))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!(portals[prtid].flags & PORTAL_USED))
		return (-EINVAL);

	/* Operation no supported. */
	if (portals[prtid].flags & PORTAL_WRONLY)
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid read size. */
	if (n < 1)
		return (-EINVAL);

	/* Setup read operation. */
	mppa_aiocb_ctor(&aiocb, portals[prtid].portal_fd, buf, n);
	assert(mppa_aio_read(&aiocb) != -1);

	/* Unblock remote. */
	mask = (1 << portals[prtid].remote);
	assert(mppa_write(portals[prtid].sync_fd, &mask, sizeof(uint64_t)) != -1);
	mppa_close(portals[prtid].sync_fd);

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
 * @param prtid ID of the target portal.
 * @param buf   Location from where data should be read.
 * @param n     Number of bytes to write.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int portal_write(int prtid, const void *buf, size_t n)
{
	uint64_t mask;

	/* Invalid portal ID.*/
	if ((prtid < 0) || (prtid >= NR_PORTAL))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!(portals[prtid].flags & PORTAL_USED))
		return (-EINVAL);

	/* Operation no supported. */
	if (!(portals[prtid].flags & PORTAL_WRONLY))
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid write size. */
	if (n < 1)
		return (-EINVAL);

	/* Wait for remote to be ready. */
	mask = (1 << arch_get_cluster_id());
	assert(mppa_ioctl(portals[prtid].sync_fd, MPPA_RX_SET_MATCH, ~mask) != -1);
	assert(mppa_read(portals[prtid].sync_fd, &mask, sizeof(uint64_t)) != -1);

	/* Write. */
	assert(mppa_pwrite(portals[prtid].portal_fd, buf, n, 0) == n);

	return (0);
}

/*=======================================================================*
 * portal_close()                                                        *
 *=======================================================================*/

/**
 * @brief Closes a portal.
 *
 * @param prtid ID of the target portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int portal_close(int prtid)
{
	/* Invalid portal ID.*/
	if ((prtid < 0) || (prtid >= NR_PORTAL))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!(portals[prtid].flags & PORTAL_USED))
		return (-EINVAL);

	/* Invalid portal. */
	if (!(portals[prtid].flags & PORTAL_WRONLY))
		return (-EINVAL);

	portal_free(prtid);
	mppa_close(portals[prtid].sync_fd);

	return (0);
}

/*=======================================================================*
 * portal_unlink()                                                       *
 *=======================================================================*/

/**
 * @brief Destroys a portal.
 *
 * @param prtid ID of the target portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int portal_unlink(int prtid)
{
	/* Invalid portal ID.*/
	if ((prtid < 0) || (prtid >= NR_PORTAL))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!(portals[prtid].flags & PORTAL_USED))
		return (-EINVAL);

	/* Invalid portal. */
	if (portals[prtid].flags & PORTAL_WRONLY)
		return (-EINVAL);

	portal_free(prtid);

	return (0);
}
