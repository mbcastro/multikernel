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
#include <stdio.h>

#include <nanvix/hal.h>

#include "mppa.h"

/*============================================================================*
 * hal_mailbox_create()                                                       *
 *============================================================================*/

/**
 * @brief Creates a mailbox.
 *
 * @param remote ID of the target remote NoC node.
 *
 * @returns Upon successful completion, the ID of the newly created
 * mailbox is returned. Upon failure, a negative error code is
 * returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int hal_mailbox_create(int remote)
{
	int fd;             /* NoC connector.              */
	char remotes[128];  /* IDs of remote NoC nodes.    */
	char pathname[128]; /* NoC connector name.         */
	int noctag;         /* NoC tag used for transfers. */

	/* Invalid NoC node ID. */
	if (remote != hal_get_node_id())
		return (-EINVAL);

	noc_get_remotes(remotes, remote);
	noctag = noctag_mailbox(remote);

	/* Build pathname for NoC connector. */
	sprintf(pathname,
			"/mppa/rqueue/%d:%d/[%s]:%d/1.%d",
			remote,
			noctag,
			remotes,
			noctag,
			HAL_MAILBOX_MSG_SIZE
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

	/* Invalid NoC node ID. */
	if (nodeid < 0)
		return (-EINVAL);

	/* Invalid NoC node ID. */
	if (nodeid == hal_get_node_id())
		return (-EINVAL);

	noc_get_remotes(remotes, nodeid);
	noctag = noctag_mailbox(nodeid);

	/* Build pathname for NoC connector. */
	sprintf(pathname,
			"/mppa/rqueue/%d:%d/[%s]:%d/1.%d",
			nodeid,
			noctag,
			remotes,
			noctag,
			HAL_MAILBOX_MSG_SIZE
	);

	/* Open NoC connector. */
	if ((fd = mppa_open(pathname, O_WRONLY)) == -1)
		goto error0;

	/* Set DMA interface for IO cluster. */
	if (noc_is_ionode(hal_get_node_id()))
	{
		if (mppa_ioctl(fd, MPPA_TX_SET_INTERFACE, noc_get_dma(hal_get_node_id())) == -1)
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
	if (n != HAL_MAILBOX_MSG_SIZE)
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
	if (n != HAL_MAILBOX_MSG_SIZE)
		return (-EINVAL);

	nread = mppa_read(mbxid, buf, n);

	return (nread);
}
