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

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#define __NEED_HAL_NOC_
#define __NEED_HAL_CORE_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_PERFORMANCE_
#include <hal.h>
#include <klib.h>
#include <resource.h>

/**
 * @brief Table of mailboxes.
 */
static struct mailbox
{
	struct resource rsrc; /**< Underlying resource.        */
	mqd_t fd;             /**< Underlying file descriptor. */
	char pathname[128];   /**< Name of underlying mqueue.  */
	int nodeid;           /**< ID of underlying node.      */
	int refcount;         /**< Reference counter.          */
	size_t volume;        /**< Amount of data transferred. */
	uint64_t latency;     /**< Transfer latency.           */
} mailboxes[HAL_NR_MAILBOX];

/**
 * @brief Resource pool for mailboxes.
 */
static const struct resource_pool pool = {
	mailboxes,
	HAL_NR_MAILBOX,
	sizeof(struct mailbox)
};

/**
 * @brief Sync module lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Default message queue attribute.
 */
static struct mq_attr mq_attr = {
	.mq_maxmsg = HAL_NR_NOC_NODES,
	.mq_msgsize = HAL_MAILBOX_MSG_SIZE
};

/*============================================================================*
 * unix_mailbox_lock()                                                        *
 *============================================================================*/

/**
 * @brief Locks Unix mailbox module.
 */
static void unix_mailbox_lock(void)
{
	pthread_mutex_lock(&lock);
}

/*============================================================================*
 * unix_mailbox_unlock()                                                      *
 *============================================================================*/

/**
 * @brief Unlocks Unix mailbox module.
 */
static void unix_mailbox_unlock(void)
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
 * hal_mailbox_create()                                                       *
 *============================================================================*/

/**
 * @brief See hal_mailbox_create().
 */
static int unix_mailbox_create(int remote)
{
	int mbxid;      /* Mailbox ID.         */
	int fd;         /* NoC connector.      */
	char *pathname; /* NoC connector name. */

	/* Check if input mailbox was already created. */
	for (int i = 0; i < HAL_NR_MAILBOX; i++)
	{
		/* Skip invalid entries. */
		if (!resource_is_used(&mailboxes[i].rsrc))
			continue;

		/* Skip invalid entries. */
		if (!resource_is_readable((&mailboxes[i].rsrc)))
			continue;

		/* Found. */
		if (mailboxes[i].nodeid == remote)
			return (-EEXIST);
	}

	/* Allocate a mailbox. */
	if ((mbxid = resource_alloc(&pool)) < 0)
		goto error0;

	pathname = mailboxes[mbxid].pathname;

	/* Build pathname for NoC connector. */
	sprintf(pathname,
		"/mailbox-%d",
		remote	
	);

	/* Open NoC connector. */
	if ((fd = mq_open(pathname, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR, &mq_attr)) == -1)
		goto error1;

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailboxes[mbxid].nodeid = remote;
	mailboxes[mbxid].refcount = 1;
	mailboxes[mbxid].latency = 0;
	mailboxes[mbxid].volume = 0;
	resource_set_rdonly(&mailboxes[mbxid].rsrc);
	resource_set_notbusy(&mailboxes[mbxid].rsrc);

	return (mbxid);

error1:
	resource_free(&pool, mbxid);
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

	unix_mailbox_lock();
		mbxid = unix_mailbox_create(remote);
	unix_mailbox_unlock();

	return (mbxid);
}

/*============================================================================*
 * hal_mailbox_open()                                                         *
 *============================================================================*/

/**
 * @brief See hal_mailbox_open().
 */
static int unix_mailbox_open(int remote)
{
	int mbxid;      /* Mailbox ID.         */
	int fd;         /* NoC connector.      */
	char *pathname; /* NoC connector name. */

	/* Allocate a mailbox. */
	if ((mbxid = resource_alloc(&pool)) < 0)
		goto error0;

	pathname = mailboxes[mbxid].pathname;

	/* Build pathname for NoC connector. */
	sprintf(pathname,
		"/mailbox-%d",
		remote
	);

	/* Open NoC connector. */
	if ((fd = mq_open(pathname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR, &mq_attr)) == -1)
		goto error1;

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailboxes[mbxid].nodeid = remote;
	mailboxes[mbxid].refcount = 1;
	mailboxes[mbxid].latency = 0;
	mailboxes[mbxid].volume = 0;
	resource_set_wronly(&mailboxes[mbxid].rsrc);
	resource_set_notbusy(&mailboxes[mbxid].rsrc);

	return (mbxid);

error1:
	resource_free(&pool, mbxid);
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

again:

	unix_mailbox_lock();

	/*
	 * Check if we should just duplicate
	 * the underlying file descriptor.
	 */
	for (int i = 0; i < HAL_NR_MAILBOX; i++)
	{
		/* Skip unused mailboxes. */
		if (!resource_is_used(&mailboxes[i].rsrc))
			continue;

		/* Skip input mailboxes. */
		if (resource_is_readable(&mailboxes[i].rsrc))
			continue;

		/* Not this node ID. */
		if (nodeid != mailboxes[i].nodeid)
			continue;

		/*
		 * Found, but mailbox is busy
		 * We have to wait a bit more.
		 */
		if (resource_is_busy(&mailboxes[i].rsrc))
		{
			unix_mailbox_unlock();
			goto again;
		}

		mbxid = i;
		mailboxes[i].refcount++;
		goto out;
	}

	mbxid = unix_mailbox_open(nodeid);

out:
	unix_mailbox_unlock();
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

	unix_mailbox_lock();

		/* Bad mailbox. */
		if (!resource_is_used(&mailboxes[mbxid].rsrc))
			goto error1;

		/* Bad mailbox. */
		if (resource_is_writable(&mailboxes[mbxid].rsrc))
			goto error1;

		/* Busy mailbox. */
		if (resource_is_busy(&mailboxes[mbxid].rsrc))
		{
			unix_mailbox_unlock();
			goto again;
		}

		/* Destroy underlying message queue. */
		if (mq_close(mailboxes[mbxid].fd) < 0)
			goto error1;
		mq_unlink(mailboxes[mbxid].pathname);

		resource_free(&pool, mbxid);

	unix_mailbox_unlock();

	return (0);

error1:
	unix_mailbox_unlock();
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

	unix_mailbox_lock();

		/* Bad mailbox. */
		if (!resource_is_used(&mailboxes[mbxid].rsrc))
			goto error1;

		/* Bad mailbox. */
		if (resource_is_readable(&mailboxes[mbxid].rsrc))
			goto error1;

		/* Busy mailbox. */
		if (resource_is_busy(&mailboxes[mbxid].rsrc))
		{
			unix_mailbox_unlock();
			goto again;
		}

		/*
		 * Decrement reference counter and release
		 * the underlying file descriptor if we can.
		 */
		if (mailboxes[mbxid].refcount-- == 1)
		{
			/* Set mailbox as busy. */
			resource_set_busy(&mailboxes[mbxid].rsrc);

			/* Release lock, since we may sleep below. */
			unix_mailbox_unlock();

			if (mq_close(mailboxes[mbxid].fd) < 0)
				goto error2;

			/* Re-acquire lock. */
			unix_mailbox_lock();

			resource_free(&pool, mbxid);
		}

	unix_mailbox_unlock();

	return (0);

error2:
	unix_mailbox_lock();
		resource_set_notbusy(&mailboxes[mbxid].rsrc);
error1:
	unix_mailbox_unlock();
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
ssize_t hal_mailbox_write(int mbxid, const void *buf, size_t n)
{
	uint64_t t1, t2;

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

	unix_mailbox_lock();

		/* Bad mailbox. */
		if (!resource_is_used(&mailboxes[mbxid].rsrc))
			goto error1;

		/* Bad mailbox. */
		if (!resource_is_writable(&mailboxes[mbxid].rsrc))
			goto error1;

		/* Busy mailbox. */
		if (resource_is_busy(&mailboxes[mbxid].rsrc))
		{
			unix_mailbox_unlock();
			goto again;
		}

		/* Set mailbox as busy. */
		resource_set_busy(&mailboxes[mbxid].rsrc);

	/*
	 * Release lock, since we may sleep below.
	 */
	unix_mailbox_unlock();

	t1 = hal_timer_get();
		if (mq_send(mailboxes[mbxid].fd, buf, n, 1) == -1)
			goto error2;
	t2 = hal_timer_get();
	mailboxes[mbxid].latency = t2 - t1;

	unix_mailbox_lock();
		resource_set_notbusy(&mailboxes[mbxid].rsrc);
	unix_mailbox_unlock();

	mailboxes[mbxid].volume = n;
	return (n);

error2:
	unix_mailbox_lock();
		resource_set_notbusy(&mailboxes[mbxid].rsrc);
	unix_mailbox_unlock();
error1:
	unix_mailbox_unlock();
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
ssize_t hal_mailbox_read(int mbxid, void *buf, size_t n)
{
	ssize_t nread;
	uint64_t t1, t2;

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

	unix_mailbox_lock();

		/* Bad mailbox. */
		if (!resource_is_used(&mailboxes[mbxid].rsrc))
			goto error1;

		/* Bad mailbox. */
		if (!resource_is_readable(&mailboxes[mbxid].rsrc))
			goto error1;

		/* Busy mailbox. */
		if (resource_is_busy(&mailboxes[mbxid].rsrc))
		{
			unix_mailbox_unlock();
			goto again;
		}

		/* Set mailbox as busy. */
		resource_set_busy(&mailboxes[mbxid].rsrc);

	/*
	 * Release lock, since we may sleep below.
	 */
	unix_mailbox_unlock();
	t1 = hal_timer_get();
		if ((nread = mq_receive(mailboxes[mbxid].fd, buf, n, NULL)) == -1)
			goto error2;
	t2 = hal_timer_get();
	mailboxes[mbxid].latency = t2 - t1;

	unix_mailbox_lock();
		resource_set_notbusy(&mailboxes[mbxid].rsrc);
	unix_mailbox_unlock();

	mailboxes[mbxid].volume = nread;
	return (nread);

error2:
	unix_mailbox_lock();
		resource_set_notbusy(&mailboxes[mbxid].rsrc);
	unix_mailbox_unlock();
error1:
	unix_mailbox_unlock();
error0:
	return (-EAGAIN);
}

/*============================================================================*
 * hal_mailbox_ioctl()                                                        *
 *============================================================================*/

/**
 * @brief Performs control operations in a mailbox.
 *
 * @param mbxid   Target mailbox.
 * @param request Request.
 * @param args    Additional arguments.
 *
 * @param Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int hal_mailbox_ioctl(int mbxid, unsigned request, va_list args)
{
	((void) mbxid);
	((void) request);
	((void) args);

	return (0);
}
