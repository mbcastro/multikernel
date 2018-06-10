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

#include <stdio.h>

/**
 * @brief Size (in bytes) of a mailbox message.
 */
#define MAILBOX_MSG_SIZE 64

/*============================================================================*
 * hal_mailbox_create()                                                       *
 *============================================================================*/

/**
 * @brief Creates a mailbox.
 *
 * @param local CPU ID of the target.
 *
 * @returns Upon successful completion, the ID of the newly created
 * mailbox is returned. Upon failure, a negative error code is
 * returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int hal_mailbox_create(int coreid)
{
	int fd;             /* NoC connector.              */
	int mbxid;          /* ID of mailbix.              */
	char remotes[128];  /* IDs of remote clusters.     */
	char pathname[128]; /* NoC connector name.         */
	int noctag;         /* NoC tag used for transfers. */
	
	noc_remotes(remotes, coreid);
	noctag = noctag_mailbox(coreid);

	/* Open NoC connector. */
	sprintf(pathname,
			"/mppa/rqueue/%d:%d/[%s]:%d/1.%d",
			coreid,
			noctag,
			remotes,
			noctag,
			MAILBOX_MSG_SIZE
	);
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
 * @param DMA channel of remote cluster.
 *
 * @returns Upon successful completion, the ID of the target mailbox
 * is returned. Upon failure, a negative error code is returned
 * instead.
 *
 * @note This function is @b NOT thread safe.
 */
int hal_mailbox_open(int clusterid)
{
	int fd;             /* NoC connector.              */
	char remotes[128];  /* IDs of remote clusters.     */
	char pathname[128]; /* NoC connector name.         */
	int noctag;         /* NoC tag used for transfers. */

	/* Allocate a mailbox. */
	if ((mbxid = mailbox_alloc()) < 0)
		return (-EAGAIN);

	noc_remotes(remotes, remote);

	noctag = noctag_mailbox(remote);
	snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/rqueue/%d:%d/[%s]:%d/1.%d",
			remote,
			noctag,
			remotes,
			noctag,
			MAILBOX_MSG_SIZE
	);
	if ((fd = mppa_open(pathname, O_WRONLY)) == -1)
		goto error0;

	/* Set DMA interface for IO cluster. */
	if (k1_is_iocluster(clusterid))
	{
		if (mppa_ioctl(fd, MPPA_TX_SET_INTERFACE, remote%NR_IOCLUSTER_DMA) == -1)
			goto error0;
	}

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailboxes[mbxid].flags |= MAILBOX_WRONLY;

	return (mbxid);

error0:
	mailbox_free(mbxid);
	return (-EAGAIN);
}

/*============================================================================*
 * hal_mailbox_unlink()                                                       *
 *============================================================================*/

int hal_mailbox_unlink(int);

/*============================================================================*
 * hal_mailbox_close()                                                        *
 *============================================================================*/

int hal_mailbox_close(int)
{
}

/*============================================================================*
 * hal_mailbox_write()                                                        *
 *============================================================================*/

int hal_mailbox_write(int, const void *, size_t)
{
}

/*============================================================================*
 * hal_mailbox_read()                                                         *
 *============================================================================*/

/**
 * @brief Reads data from a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 * @param buf   Location from where data should be written.
 *
 * @returns Upon successful completion, the number of bytes read is
 * returned. Upon failure, a negative error code is returned instead.
 */
size_t hal_mailbox_read(int mbxid, void *buf)
{
	size_t nread;

	nread = mppa_read(mbxid, buf, MAILBOX_MSG_SIZE);

	return (nread);
}
