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

#include <errno.h>
#include <stdio.h>

#include <nanvix/hal.h>

#include "mppa.h" 

/*============================================================================*
 * hal_mailbox_create()                                                       *
 *============================================================================*/

/**
 * @brief Creates a mailbox.
 *
 * @param nodeid ID of the target NoC node.
 *
 * @returns Upon successful completion, the ID of the newly created
 * mailbox is returned. Upon failure, a negative error code is
 * returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int hal_mailbox_create(int nodeid)
{
	int fd;             /* NoC connector.              */
	char remotes[128];  /* IDs of remote NoC nodes.    */
	char pathname[128]; /* NoC connector name.         */
	int noctag;         /* NoC tag used for transfers. */

#ifdef _HAS_GET_CORE_ID_	
	/* Invalid core ID. */
	if (nodeid != noc_get_node_id())
		return (-EINVAL);
#endif

	noc_remotes(remotes, nodeid);
	noctag = noctag_mailbox(nodeid);

	/* Build pathname for NoC connector. */
	sprintf(pathname,
			"/mppa/rqueue/%d:%d/[%s]:%d/1.%d",
			nodeid,
			noctag,
			remotes,
			noctag,
			MAILBOX_MSG_SIZE
	);

	/* Open NoC connector. */
	if ((fd = mppa_open(pathname, O_RDONLY)) == -1)
		return (-EAGAIN);

	return (fd);
}

/*============================================================================*
 * hal_mailbox_open()                                                         *
 *============================================================================*/

/**
 * @brief Opens a mailbox.
 *
 * @param nodeid ID of the target NoC node.
 *
 * @returns Upon successful completion, the ID of the target mailbox
 * is returned. Upon failure, a negative error code is returned
 * instead.
 *
 * @note This function is @b NOT thread safe.
 */
int hal_mailbox_open(int nodeid)
{
	int fd;             /* NoC connector.              */
	char remotes[128];  /* IDs of remote NoC nodes.    */
	char pathname[128]; /* NoC connector name.         */
	int noctag;         /* NoC tag used for transfers. */

	/* Invalid core ID. */
	if (nodeid < 0)
		return (-EINVAL);

#ifdef _HAS_GET_CORE_ID_	
	/* Invalid core ID. */
	if (nodeid == noc_get_node_id())
		return (-EINVAL);
#endif

	noc_remotes(remotes, nodeid);
	noctag = noctag_mailbox(nodeid);

	/* Build pathname for NoC connector. */
	sprintf(pathname,
			"/mppa/rqueue/%d:%d/[%s]:%d/1.%d",
			nodeid,
			noctag,
			remotes,
			noctag,
			MAILBOX_MSG_SIZE
	);

	/* Open NoC connector. */
	if ((fd = mppa_open(pathname, O_WRONLY)) == -1)
		goto error0;

	/* Set DMA interface for IO cluster. */
	if (k1_is_iocluster(nodeid))
	{
		if (mppa_ioctl(fd, MPPA_TX_SET_INTERFACE, noc_get_dma(nodeid)) == -1)
			goto error0;
	}

	return (fd);

error0:
	mppa_close(fd);
	return (-EAGAIN);
}

/*============================================================================*
 * hal_mailbox_unlink()                                                       *
 *============================================================================*/

/**
 * @brief Destroys a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int hal_mailbox_unlink(int mbxid)
{
	/* Invalid mailbox. */
	if (mbxid < 0)
		return (-EINVAL);

	return (mppa_close(mbxid));
}

/*============================================================================*
 * hal_mailbox_close()                                                        *
 *============================================================================*/

/**
 * @brief Closes a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int hal_mailbox_close(int mbxid)
{
	/* Invalid mailbox. */
	if (mbxid < 0)
		return (-EINVAL);

	return (mppa_close(mbxid));
}

/*============================================================================*
 * hal_mailbox_write()                                                        *
 *============================================================================*/

/**
 * @brief Writes data to a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 * @param buf   Buffer where the data should be read from.
 * @param n     Number of bytes to write.
 *
 * @returns Upon successful completion, the number of bytes
 * successfully written is returned. Upon failure, a negative error
 * code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
size_t hal_mailbox_write(int mbxid, const void *buf, size_t n)
{
	size_t nwrite;

	/* Invalid mailbox. */
	if (mbxid < 0)
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid write size. */
	if (n != MAILBOX_MSG_SIZE)
		return (-EINVAL);

	nwrite = mppa_write(mbxid, buf, n);

	return (nwrite);
}

/*============================================================================*
 * hal_mailbox_read()                                                         *
 *============================================================================*/

/**
 * @brief Reads data from a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 * @param buf   Buffer where the data should be written to.
 * @param n     Number of bytes to read.
 *
 * @returns Upon successful completion, the number of bytes
 * successfully read is returned. Upon failure, a negative error code
 * is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
size_t hal_mailbox_read(int mbxid, void *buf, size_t n)
{
	size_t nread;

	/* Invalid mailbox. */
	if (mbxid < 0)
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid read size. */
	if (n != MAILBOX_MSG_SIZE)
		return (-EINVAL);

	nread = mppa_read(mbxid, buf, n);

	return (nread);
}
