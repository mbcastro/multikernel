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

#include <mppaipc.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_MAILBOX_
#include <nanvix/hal.h>
#include <nanvix/klib.h>

#include "noc.h"

#define DUP_NOC_FD

/**
 * @brief Mailbox flags.
 */
/**@{*/
#define MAILBOX_FLAGS_USED   (1 << 0) /**< Mailbox in use?  */
#define MAILBOX_FLAGS_WRONLY (1 << 1) /**< Write only mode? */
#define MAILBOX_FLAGS_BUSY   (1 << 2) /**< Busy?            */
/**@}*/

/**
 * @brief Table of mailboxes.
 */
static struct 
{
	int fd;     /**< Underlying file descriptor. */
	int flags;  /**< Flags.                      */
} mailboxes[HAL_NR_MAILBOX];

/**
 * @brief Mailbox module lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*============================================================================*
 * mppa256_mailbox_lock()                                                     *
 *============================================================================*/

/**
 * @brief Locks MPPA-256 mailbox module.
 */
static void mppa256_mailbox_lock(void)
{
	pthread_mutex_lock(&lock);
}

/*============================================================================*
 * mppa256_mailbox_unlock()                                                   *
 *============================================================================*/

/**
 * @brief Unlocks MPPA-256 mailbox module.
 */
static void mppa256_mailbox_unlock(void)
{
	pthread_mutex_unlock(&lock);
}

/*============================================================================*
 * mailbox_is_valid()                                                         *
 *============================================================================*/

/**
 * @brief Asserts whether or not a mailbox is valid.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @returns One if the target mailbox is valid, and false
 * otherwise.
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
static int mailbox_is_valid(int mbxid)
{
	return ((mbxid >= 0) && (mbxid < HAL_NR_MAILBOX));
}

/*============================================================================*
 * mailbox_is_used()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a mailbox is used.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @returns One if the target mailbox is used, and false
 * otherwise.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static int mailbox_is_used(int mbxid)
{
	return (mailboxes[mbxid].flags & MAILBOX_FLAGS_USED);
}

/*============================================================================*
 * mailbox_is_wronly()                                                        *
 *============================================================================*/

/**
 * @brief Asserts whether or not a mailbox is a write-only.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @returns One if the target mailbox is write-only, and false
 * otherwise.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static int mailbox_is_wronly(int mbxid)
{
	return (mailboxes[mbxid].flags & MAILBOX_FLAGS_WRONLY);
}

/*============================================================================*
 * mailbox_set_used()                                                         *
 *============================================================================*/

/**
 * @brief Sets a mailbox as used.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static void mailbox_set_used(int mbxid)
{
	mailboxes[mbxid].flags |= MAILBOX_FLAGS_USED;
}

/*============================================================================*
 * mailbox_set_wronly()                                                       *
 *============================================================================*/

/**
 * @brief Sets a mailbox as a write-only.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static void mailbox_set_wronly(int mbxid)
{
	mailboxes[mbxid].flags |= MAILBOX_FLAGS_WRONLY;
}

/*============================================================================*
 * mailbox_is_busy()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a mailbox is busy.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @returns One if the target mailbox is busy one, and
 * false otherwise.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static int mailbox_is_busy(int mbxid)
{
	return (mailboxes[mbxid].flags & MAILBOX_FLAGS_BUSY);
}

/*============================================================================*
 * mailbox_set_busy()                                                         *
 *============================================================================*/

/**
 * @brief Sets a mailbox as busy.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static void mailbox_set_busy(int mbxid)
{
	mailboxes[mbxid].flags |= MAILBOX_FLAGS_BUSY;
}

/*============================================================================*
 * mailbox_clear_busy()                                                       *
 *============================================================================*/

/**
 * @brief Clears the busy flag of a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static void mailbox_clear_busy(int mbxid)
{
	mailboxes[mbxid].flags &= ~MAILBOX_FLAGS_BUSY;
}

/*============================================================================*
 * mailbox_clear_flags()                                                      *
 *============================================================================*/

/**
 * @brief Clears the flags of a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static void mailbox_clear_flags(int mbxid)
{
	mailboxes[mbxid].flags = 0;
}

/*============================================================================*
 * mailbox_alloc()                                                            *
 *============================================================================*/

/**
 * @brief Allocates a mailbox.
 *
 * @param nodeid ID of target NoC node.
 *
 * @returns Upon successful completion, the ID of a newly allocated
 * mailbox is returned. Upon failure, -1 is returned instead.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static int mailbox_alloc(int nodeid)
{
	int mbxid;

	/* Check for double allocation. */
	if (noc_is_cnode(hal_get_node_id()))
		mbxid = hal_get_node_num(nodeid);
	else
	{
		mbxid = hal_get_core_id()*HAL_NR_NOC_NODES + 
			hal_get_node_num(nodeid);
	}

	/* Allocate. */
	if (mailbox_is_used(mbxid))
		return (-1);
	mailbox_set_used(mbxid);

	return (mbxid);
}

/*============================================================================*
 * mailbox_free()                                                             *
 *============================================================================*/

/**
 * @brief Frees a mailbox.
 *
 * @param mbxid ID of the target mailboxhronization point.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static void mailbox_free(int mbxid)
{
	mailbox_clear_flags(mbxid);
}

/*============================================================================*
 * hal_mailbox_create()                                                       *
 *============================================================================*/

/**
 * @brief See hal_mailbox_create().
 */
static int mppa256_mailbox_create(int remote)
{
	int mbxid;          /* Mailbox ID.                 */
	int fd;             /* NoC connector.              */
	char remotes[128];  /* IDs of remote NoC nodes.    */
	char pathname[128]; /* NoC connector name.         */
	int noctag;         /* NoC tag used for transfers. */

	/* Allocate a mailbox. */
	if ((mbxid = mailbox_alloc(remote)) < 0)
		goto error0;

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
		goto error1;

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailbox_clear_busy(mbxid);

	return (mbxid);

error1:
	mailbox_free(mbxid);
error0:
	return (-EAGAIN);
}

/**
 * @brief Creates a mailbox.
 *
 * @param remote ID of the target remote NoC node.
 *
 * @returns Upon successful completion, the ID of the newly created
 * mailbox is returned. Upon failure, a negative error code is
 * returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_mailbox_create(int remote)
{
	int mbxid;

	/* Invalid NoC node ID. */
	if (remote != hal_get_node_id())
		return (-EINVAL);

	mppa256_mailbox_lock();
		mbxid = mppa256_mailbox_create(remote);
	mppa256_mailbox_unlock();

	return (mbxid);
}

/*============================================================================*
 * hal_mailbox_open()                                                         *
 *============================================================================*/

/**
 * @brief See hal_mailbox_open().
 */
static int mppa256_mailbox_open(int nodeid)
{
	int mbxid;          /* Mailbox ID.                 */
	int fd;             /* NoC connector.              */
	char remotes[128];  /* IDs of remote NoC nodes.    */
	char pathname[128]; /* NoC connector name.         */
	int noctag;         /* NoC tag used for transfers. */

	/* Allocate a mailbox. */
	if ((mbxid = mailbox_alloc(nodeid)) < 0)
		goto error0;

#ifdef DUP_NOC_FD

	/*
	 * Check if we need to duplicate
	 * the underlying dile descriptor.
	 */
	if (noc_is_ionode(hal_get_node_id()))
	{
		int ncores;
		int mycoreid;
		
		ncores = hal_get_num_cores();
		mycoreid = hal_get_core_id();

		/* Check if already opened. */
		for (int i = 0; i < ncores; i++)
		{
			int j;

			/*
			 * We have just allocated the 
			 * mailbox.
			 */
			if (mycoreid == i)
				continue;

			j = i*HAL_NR_NOC_NODES+hal_get_node_num(nodeid);

			/* Not used. */
			if (!mailbox_is_used(j))
				continue;

			/* Input mailbox. */
			if (!mailbox_is_wronly(j))
				continue;

			fd = mailboxes[j].fd;
			goto done;
		}
	}

#endif

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
		goto error1;

	/* Set DMA interface for IO cluster. */
	if (noc_is_ionode(hal_get_node_id()))
	{
		if (mppa_ioctl(fd, MPPA_TX_SET_INTERFACE, noc_get_dma(hal_get_node_id())) == -1)
			goto error2;
	}

#ifdef DUP_NOC_FD
done:
#endif

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailbox_set_wronly(mbxid);
	mailbox_clear_busy(mbxid);

	return (mbxid);

error2:
	mppa_close(fd);
error1:
	mailbox_free(mbxid);
error0:
	return (-EAGAIN);
}

/**
 * @brief Opens a mailbox.
 *
 * @param nodeid ID of the target NoC node.
 *
 * @returns Upon successful completion, the ID of the target mailbox
 * is returned. Upon failure, a negative error code is returned
 * instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_mailbox_open(int nodeid)
{
	int mbxid;

	/* Invalid NoC node ID. */
	if (nodeid < 0)
		return (-EINVAL);

	/* Invalid NoC node ID. */
	if (nodeid == hal_get_node_id())
		return (-EINVAL);

	mppa256_mailbox_lock();
		mbxid = mppa256_mailbox_open(nodeid);
	mppa256_mailbox_unlock();
	
	return (mbxid);
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
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_mailbox_unlink(int mbxid)
{
	/* Invalid mailbox. */
	if (!mailbox_is_valid(mbxid))
		goto error0;

again:

	mppa256_mailbox_lock();

		/* Bad mailbox. */
		if (!mailbox_is_used(mbxid))
			goto error1;

		/* Bad mailbox. */
		if (mailbox_is_wronly(mbxid))
			goto error1;

		/* Busy mailbox. */
		if (mailbox_is_busy(mbxid))
		{
			mppa256_mailbox_unlock();
			goto again;
		}

		if (mppa_close(mailboxes[mbxid].fd) < 0)
			goto error1;

		mailbox_free(mbxid);

	mppa256_mailbox_unlock();

	return (0);

error1:
	mppa256_mailbox_unlock();
error0:
	return (-EAGAIN);
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
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_mailbox_close(int mbxid)
{
	/* Invalid mailbox. */
	if (!mailbox_is_valid(mbxid))
		goto error0;

again:

	mppa256_mailbox_lock();

		/* Bad mailbox. */
		if (!mailbox_is_used(mbxid))
			goto error1;

		/* Bad mailbox. */
		if (!mailbox_is_wronly(mbxid))
			goto error1;

		/* Busy mailbox. */
		if (mailbox_is_busy(mbxid))
		{
			mppa256_mailbox_unlock();
			goto again;
		}

		/* Set mailbox as busy. */
		mailbox_set_busy(mbxid);

	/*
	 * Release lock, since we may sleep below.
	 */
	mppa256_mailbox_unlock();

#ifdef DUP_NOC_FD

	/*
	 * Check if the underlying file
	 * descriptor has been duplicated.
	 */
	if (noc_is_ionode(hal_get_node_id()))
	{
		int fd;

		fd = mailboxes[mbxid].fd;

		/* Check if already opened. */
		for (int i = 0; i < HAL_NR_MAILBOX; i++)
		{
			/* Skip this one. */
			if (i == mbxid)
				continue;

			/* Not used. */
			if (!mailbox_is_used(i))
				continue;

			/* Input mailbox. */
			if (!mailbox_is_wronly(i))
				continue;

			/* Not this output mailbox. */
			if (fd != mailboxes[i].fd)
				continue;

			goto done;
		}
	}

#endif
	
	if (mppa_close(mailboxes[mbxid].fd) < 0)
		goto error1;


#ifdef DUP_NOC_FD
done:
#endif

	mppa256_mailbox_lock();
		mailbox_free(mbxid);
		mailbox_clear_busy(mbxid);
	mppa256_mailbox_unlock();

	return (0);

error1:
	mppa256_mailbox_lock();
		mailbox_clear_busy(mbxid);
	mppa256_mailbox_unlock();
error0:
	return (-EAGAIN);
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
 * @note This function is thread-safe.
 */
size_t hal_mailbox_write(int mbxid, const void *buf, size_t n)
{
	size_t nwrite;

	/* Invalid mailbox. */
	if (!mailbox_is_valid(mbxid))
		goto error0;

	/* Invalid buffer. */
	if (buf == NULL)
		goto error0;

	/* Invalid write size. */
	if (n != HAL_MAILBOX_MSG_SIZE)
		goto error0;

again:

	mppa256_mailbox_lock();

		/* Bad mailbox. */
		if (!mailbox_is_used(mbxid))
			goto error1;

		/* Bad mailbox. */
		if (!mailbox_is_wronly(mbxid))
			goto error1;

		/* Busy mailbox. */
		if (mailbox_is_busy(mbxid))
		{
			mppa256_mailbox_unlock();
			goto again;
		}

		/* Set mailbox as busy. */
		mailbox_set_busy(mbxid);

	/*
	 * Release lock, since we may sleep below.
	 */
	mppa256_mailbox_unlock();

	nwrite = mppa_write(mailboxes[mbxid].fd, buf, n);

	mppa256_mailbox_lock();
		mailbox_clear_busy(mbxid);
	mppa256_mailbox_unlock();

	return (nwrite);

error1:
	mppa256_mailbox_unlock();
error0:
	return (-EAGAIN);
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
 * @note This function is thread-safe.
 */
size_t hal_mailbox_read(int mbxid, void *buf, size_t n)
{
	size_t nread;

	/* Invalid mailbox. */
	if (!mailbox_is_valid(mbxid))
		goto error0;

	/* Invalid buffer. */
	if (buf == NULL)
		goto error0;

	/* Invalid read size. */
	if (n != HAL_MAILBOX_MSG_SIZE)
		goto error0;

again:

	mppa256_mailbox_lock();

		/* Bad mailbox. */
		if (!mailbox_is_used(mbxid))
			goto error1;

		/* Bad mailbox. */
		if (mailbox_is_wronly(mbxid))
			goto error1;

		/* Busy mailbox. */
		if (mailbox_is_busy(mbxid))
		{
			mppa256_mailbox_unlock();
			goto again;
		}

		/* Set mailbox as busy. */
		mailbox_set_busy(mbxid);

	/*
	 * Release lock, since we may sleep below.
	 */
	mppa256_mailbox_unlock();

	nread = mppa_read(mailboxes[mbxid].fd, buf, n);
	
	mppa256_mailbox_lock();
		mailbox_clear_busy(mbxid);
	mppa256_mailbox_unlock();

	return (nread);

error1:
	mppa256_mailbox_unlock();
error0:
	return (-EAGAIN);
}
