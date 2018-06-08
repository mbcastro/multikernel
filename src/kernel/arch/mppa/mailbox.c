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
#include <nanvix/name.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Number of mailboxes.
 */
#define NR_MAILBOX 256

/**
 * @brief Mailbox flags.
 */
/**@{*/
#define MAILBOX_USED   (1 << 0)
#define MAILBOX_WRONLY (1 << 1)
/**@}*/

/**
 * @brief Mailbox.
 */
struct mailbox
{
	int fd;    /**< File descriptor of noC connector. */
	int flags; /**< Flags.                            */
};

/**
 * @brief table of mailboxes.
 */
static struct mailbox mailboxes[NR_MAILBOX];

/*=======================================================================*
 * mailbox_alloc()                                                       *
 *=======================================================================*/

/**
 * @brief Allocates a mailbox.
 *
 * @return Upon successful completion, the ID of the newly allocated
 * mailbox is returned. Upon failure, -1 is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
static int mailbox_alloc(void)
{
	/* Search for a free mailbox. */
	for (int i = 0; i < NR_MAILBOX; i++)
	{
		/* Found. */
		if (!(mailboxes[i].flags & MAILBOX_USED))
		{
			mailboxes[i].flags |= MAILBOX_USED;
			return (i);
		}
	}

	printf("[NAME] mailbox table overflow\n");

	return (-1);
}

/*=======================================================================*
 * mailbox_free()                                                        *
 *=======================================================================*/

/**
 * @brief Frees a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @note This function is @b NOT thread safe.
 */
static void mailbox_free(int mbxid)
{
	/* Sanity check. */
	assert((mbxid >= 0) && (mbxid < NR_MAILBOX));
	assert(mailboxes[mbxid].flags & MAILBOX_USED);

	mailboxes[mbxid].flags = 0;
	mppa_close(mailboxes[mbxid].fd);
}

/*=======================================================================*
 * mailbox_noctag()                                                      *
 *=======================================================================*/

/**
 * @brief Computes the mailbox NoC tag for a cluster.
 *
 * @param local Id of target cluster.
 */
static int mailbox_noctag(int local, int type)
{
	static int OFFSET = 50;

	if ((local >= CCLUSTER0) && (local <= CCLUSTER15))
	{
		if (type)
			return (OFFSET + 16 + local);
		return (16 + local);
	}
	else if ((local >= IOCLUSTER0) && (local < (IOCLUSTER0 + NR_IOCLUSTER_DMA)))
	{
		if (type)
			return (OFFSET + 16 + 16 + 0);
		return (16 + 16 + 0);
	}
	else if ((local >= IOCLUSTER1) && (local < (IOCLUSTER1 + NR_IOCLUSTER_DMA)))
	{
		if (type)
			return (OFFSET + 16 + 16 + 1);
		return (16 + 16 + 1);
	}
	return (0);
}

/*======================================================================*
* mailbox_create()                                                      *
*=======================================================================*/

/**
* @brief Creates a mailbox.
*
* @param name Mailbox name.
*
* @returns Upon successful completion, the ID of the new mailbox is
* returned. Upon failure, a negative error code is returned instead.
*/
int mailbox_create(char *name)
{
	int local; /* ID of local cluster. */

	/* Invalid mailbox name. */
	if (name == NULL)
	return (-EINVAL);

	local = name_cluster_dma(name);
	assert(name_cluster_id(name) == k1_get_cluster_id());

	return (_mailbox_create(local, STD));
}

/*======================================================================*
* mailbox_open()                                                        *
*=======================================================================*/

/**
* @brief Opens a mailbox.
*
* @param name Mailbox name.
*
* @returns Upon successful completion, the ID of the target mailbox is
* returned. Upon failure, a negative error code is returned instead.
*/
int mailbox_open(char *name)
{
	int local;  /* ID of remote cluster. */
	int remote; /* ID of remote cluster. */

	/* Invalid mailbox name. */
	if (name == NULL)
	return (-EINVAL);

	remote = name_cluster_dma(name);
	local = k1_get_cluster_id();

	assert(name_cluster_id(name) != local);

	return (_mailbox_open(remote, STD));
}

/*=======================================================================*
 * _mailbox_create()                                                     *
 *=======================================================================*/

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
int _mailbox_create(int local, int type)
{
	int fd;             /* File descriptor for NoC connector. */
	int mbxid;          /* ID of mailbix.                     */
	char remotes[128];  /* IDs of remote clusters.            */
	char pathname[128]; /* NoC connector name.                */
	int noctag;         /* NoC tag used for transfers.        */

	if (local >= IOCLUSTER0 && local < IOCLUSTER0 + NR_IOCLUSTER_DMA)
		assert(local == k1_get_cluster_id() + local%IOCLUSTER0);
	else if (local >= IOCLUSTER1 && local < IOCLUSTER1 + NR_IOCLUSTER_DMA)
		assert(local == k1_get_cluster_id() + local%IOCLUSTER1);
	else
		assert(local == k1_get_cluster_id());

	/* Allocate mailbox. */
	if ((mbxid = mailbox_alloc()) < 0)
		return (-EAGAIN);

	name_remotes(remotes, local);
	noctag = mailbox_noctag(local, type);

	/* Open NoC connector. */
	sprintf(pathname,
			"/mppa/rqueue/%d:%d/[%s]:%d/1.%d",
			local,
			noctag,
			remotes,
			noctag,
			MAILBOX_MSG_SIZE
	);
	if ((fd = mppa_open(pathname, O_RDONLY)) == -1)
	{
		mailbox_free(mbxid);
		return (-EAGAIN);
	}

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailboxes[mbxid].flags &= ~(MAILBOX_WRONLY);

	return (mbxid);
}

/*=======================================================================*
 * _mailbox_open()                                                       *
 *=======================================================================*/

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
int _mailbox_open(int remote, int type)
{
	int local;          /* ID of local cluster.               */
	int fd;             /* File descriptor for NoC connector. */
	int mbxid;          /* ID of mailbix.                     */
	char remotes[128];  /* IDs of remote clusters.            */
	char pathname[128]; /* NoC connector name.                */
	int noctag;         /* NoC tag used for transfers.        */

	local = k1_get_cluster_id();
	assert(remote != local);

	/* Allocate a mailbox. */
	if ((mbxid = mailbox_alloc()) < 0)
		return (-EAGAIN);

	name_remotes(remotes, remote);

	noctag = mailbox_noctag(remote, type);
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
	if (k1_is_iocluster(local))
	{
		if (mppa_ioctl(fd, MPPA_TX_SET_INTERFACE, local%NR_IOCLUSTER_DMA) == -1)
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

/*=======================================================================*
 * mailbox_read()                                                        *
 *=======================================================================*/

/**
 * @brief Reads data from a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 * @param buf   Location from where data should be written.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int mailbox_read(int mbxid, void *buf)
{
	/* Invalid mailbox ID.*/
	if ((mbxid < 0) || (mbxid >= NR_MAILBOX))
		return (-EINVAL);

	/*  Invalid mailbox. */
	if (!(mailboxes[mbxid].flags & MAILBOX_USED))
		return (-EINVAL);

	/* Operation no supported. */
	if (mailboxes[mbxid].flags & MAILBOX_WRONLY)
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	assert(mppa_read(mailboxes[mbxid].fd, buf, MAILBOX_MSG_SIZE)
	                                       == MAILBOX_MSG_SIZE);

	return (0);
}

/*=======================================================================*
 * mailbox_write()                                                       *
 *=======================================================================*/

/**
 * @brief Writes data to a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 * @param buf   Location from where data should be read.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int mailbox_write(int mbxid, const void *buf)
{
	/* Invalid mailbox ID.*/
	if ((mbxid < 0) || (mbxid >= NR_MAILBOX))
		return (-EINVAL);

	/*  Invalid mailbox. */
	if (!(mailboxes[mbxid].flags & MAILBOX_USED))
		return (-EINVAL);

	/* Operation no supported. */
	if (!(mailboxes[mbxid].flags & MAILBOX_WRONLY))
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	assert(mppa_write(mailboxes[mbxid].fd, buf, MAILBOX_MSG_SIZE)
	                                         == MAILBOX_MSG_SIZE);

	return (0);
}

/*=======================================================================*
 * mailbox_close()                                                       *
 *=======================================================================*/

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
int mailbox_close(int mbxid)
{
	/* Invalid mailbox ID.*/
	if ((mbxid < 0) || (mbxid >= NR_MAILBOX))
		return (-EINVAL);

	/*  Invalid mailbox. */
	if (!(mailboxes[mbxid].flags & MAILBOX_USED))
		return (-EINVAL);

	mailbox_free(mbxid);

	return (0);
}

/*=======================================================================*
 * mailbox_unlink()                                                      *
 *=======================================================================*/

/**
 * @brief Destroys a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int mailbox_unlink(int mbxid)
{
	/* Invalid mailbox ID.*/
	if ((mbxid < 0) || (mbxid >= NR_MAILBOX))
		return (-EINVAL);

	/*  Invalid mailbox. */
	if (!(mailboxes[mbxid].flags & MAILBOX_USED))
		return (-EINVAL);

	mailbox_free(mbxid);

	return (0);
}
