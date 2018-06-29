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
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#define __NEED_HAL_NOC_
#define __NEED_HAL_PORTAL_
#include <nanvix/name.h>
#include <nanvix/klib.h>
#include <nanvix/hal.h>

#include "noc.h"

/**
 * @brief Portal flags.
 */
/**@{*/
#define PORTAL_FLAGS_USED   (1 << 0) /**< Used portal?     */
#define PORTAL_FLAGS_WRONLY (1 << 2) /**< Write only mode? */
/**@}*/

/**
 * @brief Table of portals.
 */
struct
{
	int flags;     /**< Flags.                */
	int portal_fd; /**< Portal NoC connector. */
	int sync_fd;   /**< Sync NoC connector.   */
	int remote;    /**< Remote NoC node ID.   */
	int local;     /**< Local NoC node ID.    */
} portals[HAL_NR_PORTAL];

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
 * @note This function is @b NOT thread safe.
 */
static int portal_is_valid(int portalid)
{
	return ((portalid >= 0) || (portalid < HAL_NR_PORTAL));
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
 * @returns Upon successful completion, the ID of a newly allocated
 * portal is returned. Upon failure, -1 is returned
 * instead.
 *
 * @note This function is @b NOT thread safe.
 */
static int portal_alloc(void)
{
	/* Search for a free portal. */
	for (int i = 0; i < HAL_NR_PORTAL; i++)
	{
		/* Found. */
		if (!portal_is_used(i))
		{
			portal_set_used(i);
			return (i);
		}
	}

	return (-1);
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
 * @brief Internal hal_portal_create().
 *
 * @param local ID of the local NoC node.
 *
 * @returns Upon successful completion, the ID of a newly created
 * portal is returned. Upon failure, a negative error code is returned
 * instead.
 *
 * @note This function is @b NOT thread safe.
 */
static int _hal_portal_create(int local)
{
	int portalid;       /* ID of  portal               */
	int fd;             /* Portal NoC Connector.       */
	char pathname[128]; /* NoC connector name.         */
	int noctag;         /* NoC tag used for transfers. */

	/* Allocate portal. */
	if ((portalid = portal_alloc()) < 0)
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
 * @note This function is @b NOT thread safe.
 */
int hal_portal_create(int local)
{
	/* Invalid local NoC node. */
	if (local < -1)
		return (-EINVAL);

	/* Bad local NoC node. */
	if (local != hal_get_node_id())
		return (-EINVAL);

	return (_hal_portal_create(local));
}

/*============================================================================*
 * hal_portal_allow()                                                         *
 *============================================================================*/

/**
 * @brief Enables read operations from a remote.
 *
 * @param portalid ID of the target portal.
 * @paral local    NoC node ID of local.
 * @param remote   NoC node ID of target remote.
 *
 * @returns Upons successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int _hal_portal_allow(int portalid, int local, int remote)
{
	int sync_fd;        /* Sync NoC connector. */
	char pathname[128]; /* Portal pathname.    */

	/* Create underlying sync. */
	sprintf(pathname,
			"/mppa/sync/%d:%d",
			remote,
			(noc_is_cnode(remote) || noc_is_cnode(local)) ?
							 noctag_portal(portals[portalid].local) : 127
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
 */
int hal_portal_allow(int portalid, int remote)
{
	int local;

	/* Invalid portal.*/
	if (!portal_is_valid(portalid))
		return (-EINVAL);

	/* Bad portal. */
	if (!portal_is_used(portalid))
		return (-EINVAL);

	/* Bad portal. */
	if (portal_is_wronly(portalid))
		return (-EINVAL);

	/* Invalid remote. */
	if (remote < 0)
		return (-EINVAL);

	local = hal_get_node_id();

	/* Invalid remote. */
	if (remote == local)
		return (-EINVAL);

	return (_hal_portal_allow(portalid, local, remote));
}

/*============================================================================*
 * hal_portal_open()                                                          *
 *============================================================================*/

/**
 * @brief Internal hal_portal_open().
 *
 * @param local  ID of the local NoC node.
 * @param remote ID of the remote NoC node.
 *
 * @returns Upon successful completion, the ID of the target portal is
 * returned. Upon failure, a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
static int _hal_portal_open(int local, int remote)
{
	int portalid;       /* ID of  portal         */
	int portal_fd;      /* Portal NoC Connector. */
	int sync_fd;        /* Sync NoC connector.   */
	char pathname[128]; /* NoC connector name.   */

	/* Allocate portal. */
	if ((portalid = portal_alloc()) < 0)
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
			(noc_is_cnode(remote) || noc_is_cnode(local)) ?
									noctag_portal(remote) : 127
	);

	/* Open NoC connector. */
	if ((sync_fd = mppa_open(pathname, O_RDONLY)) == -1)
		goto error2;

	portals[portalid].portal_fd = portal_fd;
	portals[portalid].sync_fd = sync_fd;
	portals[portalid].remote = remote;
	portals[portalid].local = local;
	portal_set_wronly(portalid);

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
 * @note This function is @b NOT thread safe.
 */
int hal_portal_open(int remote)
{
	int local;

	/* Invalid node ID. */
	if (remote < 0)
		return (-EINVAL);

	local = hal_get_node_id();

	/* Bad remote. */
	if (remote == local)
		return (-EINVAL);

	return (_hal_portal_open(local, remote));
}

/*============================================================================*
 * hal_portal_read()                                                          *
 *============================================================================*/

/**
 * @brief Internal hal_portal_read()
 *
 * @param portalid ID of target portal.
 * @param buf      Location from where data should be written.
 * @param n        Number of bytes to read.
 *
 * @returns Upon successful completion, the number of bytes read is
 * returned. Upon failure, a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
static int _hal_portal_read(int portalid, void *buf, size_t n)
{
	uint64_t mask;
	mppa_aiocb_t aiocb;
	size_t nread;

	/* Setup read operation. */
	mppa_aiocb_ctor(&aiocb, portals[portalid].portal_fd, buf, n);
	if (mppa_aio_read(&aiocb) == -1)
		return (-EINVAL);

	/* Unblock remote. */
	mask = 1 << noc_get_node_num(portals[portalid].local);
	if (mppa_write(portals[portalid].sync_fd, &mask, sizeof(uint64_t)) == -1)
		return (-EAGAIN);

	/* Wait read operation to complete. */
	nread = mppa_aio_wait(&aiocb);
	mppa_close(portals[portalid].sync_fd);
	portals[portalid].sync_fd = -1;

	return (nread);
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
 * @note This function is @b NOT thread safe.
 */
int hal_portal_read(int portalid, void *buf, size_t n)
{
	/* Invalid portal ID.*/
	if (!portal_is_valid(portalid))
		return (-EINVAL);

	/* Bad portal. */
	if (!portal_is_used(portalid))
		return (-EINVAL);

	/* Bad portal.*/
	if (portal_is_wronly(portalid))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid read size. */
	if (n < 1)
		return (-EINVAL);
	
	return (_hal_portal_read(portalid, buf, n));
}

/*============================================================================*
 * hal_portal_write()                                                         *
 *============================================================================*/

/**
 * @brief Internal hal_portal_write()
 *
 * @param portalid ID of target portal.
 * @param buf      Location from where data should be read.
 * @param n        Number of bytes to write.
 *
 * @returns Upon successful the number of bytes written is returned.
 * Upon failure, a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
static int _hal_portal_write(int portalid, const void *buf, size_t n)
{
	uint64_t mask;
	size_t nwrite;

	/* Wait for remote to be ready. */
	mask = 1 << noc_get_node_num(portals[portalid].remote);
	if (mppa_ioctl(portals[portalid].sync_fd, MPPA_RX_SET_MATCH, ~mask) == -1)
		return (-EINVAL);

	if (mppa_read(portals[portalid].sync_fd, &mask, sizeof(uint64_t)) == -1)
		return (-EAGAIN);

	/* Write. */
	nwrite = mppa_pwrite(portals[portalid].portal_fd, buf, n, 0);

	return (nwrite);
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
 * @note This function is @b NOT thread safe.
 */
int hal_portal_write(int portalid, const void *buf, size_t n)
{
	/* Invalid portal ID.*/
	if (!portal_is_valid(portalid))
		return (-EINVAL);

	/* Bad portal. */
	if (!portal_is_used(portalid))
		return (-EINVAL);

	/* Bad portal. */
	if (!portal_is_wronly(portalid))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid write size. */
	if (n < 1)
		return (-EINVAL);

	return (_hal_portal_write(portalid, buf, n));
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
 * @note This function is @b NOT thread safe.
 */
int hal_portal_close(int portalid)
{
	/* Invalid portal.*/
	if (!portal_is_valid(portalid))
		return (-EINVAL);

	/* Bad portal. */
	if (!portal_is_used(portalid))
		return (-EINVAL);

	/* Bad portal. */
	if (!portal_is_wronly(portalid))
		return (-EINVAL);

	/* Close underlying portal. */
	if (mppa_close(portals[portalid].portal_fd) < 0)
		return (-EINVAL);

	/* Close underlying sync connector. */
	if (mppa_close(portals[portalid].sync_fd) < 0)
		goto error;

	portal_free(portalid);

	return (0);

error:
	printf("[PANIC] failed cannot close portal\n");
	while (1);
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
 * @note This function is @b NOT thread safe.
 */
int hal_portal_unlink(int portalid)
{
	/* Invalid portal.*/
	if (!portal_is_valid(portalid))
		return (-EINVAL);

	/* Bad portal. */
	if (!portal_is_used(portalid))
		return (-EINVAL);

	/* Bad portal. */
	if (portal_is_wronly(portalid))
		return (-EINVAL);

	/* Close underlying portal. */
	if (mppa_close(portals[portalid].portal_fd) < 0)
		return (-EINVAL);

	/* Close underlying sync connector. */
	if (portals[portalid].sync_fd != -1)
	{
		if (mppa_close(portals[portalid].sync_fd) < 0)
			goto error;
	}

	portals[portalid].sync_fd = -1;
	portal_free(portalid);

	return (0);

error:
	printf("[PANIC] failed cannot unlink portal\n");
	while (1);
	return (-EINVAL);
}
