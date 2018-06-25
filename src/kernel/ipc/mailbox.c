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

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include <nanvix/hal.h>
#include <nanvix/name.h>
#include <nanvix/pm.h>
#include <nanvix/arch/mppa.h>

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
	int fd;     /**< NoC connector. */
	int flags;  /**< Flags.         */
	int nodeid; /**< NoC node ID.   */
};

/**
 * @brief table of mailboxes.
 */
static struct mailbox mailboxes[HAL_NR_MAILBOX];

/*============================================================================*
 * mailbox_is_valid()                                                         *
 *============================================================================*/

/**
 * @brief Asserts whether or not a mailbox is valid.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @returns One if the mailbox is valid, and zero otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static inline int mailbox_is_valid(int mbxid)
{
	return ((mbxid >=0) && (mbxid < HAL_NR_MAILBOX));
}

/*============================================================================*
 * mailbox_is_used()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a mailbox is in use.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @returns One if the mailbox is in use, and zero otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static inline int mailbox_is_used(int mbxid)
{
	return (mailboxes[mbxid].flags & MAILBOX_USED);
}

/*============================================================================*
 * mailbox_is_wronly()                                                        *
 *============================================================================*/

/**
 * @brief Asserts whether or not a mailbox is write-only.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @returns One if the mailbox is write-only and false otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static inline int mailbox_is_wronly(int mbxid)
{
	return (mailboxes[mbxid].flags & MAILBOX_WRONLY);
}

/*===========================================================================*
 * mailbox_clear_flags()                                                     *
 *===========================================================================*/

/**
 * @brief Clears the flags of a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @note This function is @b NOT thread safe.
 */
static inline void mailbox_clear_flags(int mbxid)
{
	mailboxes[mbxid].flags = 0;
}

/*============================================================================*
 * mailbox_set_used()                                                         *
 *============================================================================*/

/**
 * @brief Sets a mailbox as in use.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @note This function is @b NOT thread safe.
 */
static inline void mailbox_set_used(int mbxid)
{
	mailboxes[mbxid].flags |= MAILBOX_USED;
}

/*============================================================================*
 * mailbox_set_wronly()                                                       *
 *============================================================================*/

/**
 * @brief Sets a mailbox as write-only.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @note This function is @b NOT thread safe.
 */
static inline void mailbox_set_wronly(int mbxid)
{
	mailboxes[mbxid].flags |= MAILBOX_WRONLY;
}

/*============================================================================*
 * mailbox_alloc()                                                            *
 *============================================================================*/

/**
 * @brief Allocates a mailbox.
 *
 * @return Upon successful completion, the ID of the newly allocated
 * mailbox is returned. Upon failure, -1 is returned instead.
 *
 * @note this function is @b not thread safe.
 */
static int mailbox_alloc(void)
{
	/* Search for a free mailbox. */
	for (int i = 0; i < HAL_NR_MAILBOX; i++)
	{
		/* Found. */
		if (!mailbox_is_used(i))
		{
			mailbox_set_used(i);
			return (i);
		}
	}

	printf("[NAME] mailbox table overflow\n");

	return (-1);
}

/*============================================================================*
 * mailbox_free()                                                             *
 *============================================================================*/

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
	assert(mailbox_is_valid(mbxid));
	assert(mailbox_is_used(mbxid));

	mailbox_clear_flags(mbxid);
}

/*============================================================================*
 * mailbox_create()                                                           *
 *============================================================================*/

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
	int fd;     /* NoC connector. */
	int nodeid; /* NoC node ID.   */
	int mbxid;  /* ID of mailbix. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Allocate mailbox. */
	if ((mbxid = mailbox_alloc()) < 0)
		return (-EAGAIN);

	nodeid = hal_get_node_id();
	printf("test - before name_link node %d\n", nodeid);
	/* Create underlying HW channel and link name. */
	if (name_link(nodeid, name) != 0)
		goto error0;
	printf("test - after name_link node %d\n", nodeid);

	/* Get NoC connector. */
	fd = get_inbox();

	/* Invalid NoC connector. */
	if (fd < 0)
		goto error1;

	printf("test - created nodeid:%d client: %d\n", nodeid, fd);
	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;

	return (mbxid);

error1:
	name_unlink(name);

error0:
	name_unlink(name);
	mailbox_free(mbxid);
	return (-EAGAIN);
}

/*============================================================================*
 * mailbox_open()                                                             *
 *============================================================================*/

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
	int fd;     /* NoC connector. */
	int coreid; /* Core ID.       */
	int mbxid;  /* ID of mailbix. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Resolve name, */
	if ((coreid = name_lookup(name)) < 0)
		return (-EAGAIN);

	/* Allocate a mailbox. */
	if ((mbxid = mailbox_alloc()) < 0)
		return (-EAGAIN);

	/* Open underlying HW channel. */
	if ((fd = hal_mailbox_open(coreid)) == -1)
		goto error0;

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailbox_set_wronly(mbxid);

	return (mbxid);

error0:
	mailbox_free(mbxid);
	return (-EAGAIN);
}

/*============================================================================*
 * mailbox_read()                                                             *
 *============================================================================*/

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
	if (mailbox_is_valid(mbxid))
		return (-EINVAL);

	/*  Bad mailbox. */
	if (!mailbox_is_used(mbxid))
		return (-EINVAL);

	/* Operation no supported. */
	if (mailbox_is_wronly(mbxid))
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	return (hal_mailbox_read(mailboxes[mbxid].fd, buf, HAL_MAILBOX_MSG_SIZE));
}

/*============================================================================*
 * mailbox_write()                                                            *
 *============================================================================*/

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
	if ((mbxid < 0) || (mbxid >= HAL_NR_MAILBOX))
		return (-EINVAL);

	/*  Bad mailbox. */
	if (!(mailboxes[mbxid].flags & MAILBOX_USED))
		return (-EINVAL);

	/* Operation no supported. */
	if (!(mailboxes[mbxid].flags & MAILBOX_WRONLY))
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	return (0);
}

/*============================================================================*
 * mailbox_close()                                                            *
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
int mailbox_close(int mbxid)
{
	/* Invalid mailbox ID.*/
	if (!mailbox_is_valid(mbxid))
		return (-EINVAL);

	/* Bad mailbox. */
	if (!mailbox_is_used(mbxid))
		return (-EINVAL);

	/*  Invalid mailbox. */
	if (mailbox_is_wronly(mbxid))
		return (-EINVAL);

	hal_mailbox_close(mailboxes[mbxid].fd);

	mailbox_free(mbxid);

	return (0);
}

/*============================================================================*
 * mailbox_unlink()                                                           *
 *============================================================================*/

/**
 * @brief Destroys a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @returns Upon successful completion zero, is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int mailbox_unlink(int mbxid)
{
	int r;	/* Return value */

	/* Invalid mailbox ID.*/
	if (!mailbox_is_valid(mbxid))
		return (-EINVAL);
		printf("test 1- unlink \n");

	/* Bad mailbox. */
	if (!mailbox_is_used(mbxid))
		return (-EINVAL);
		printf("test 2- unlink \n" );

	/*  Invalid mailbox. */
	if (mailbox_is_wronly(mbxid))
		return (-EINVAL);
		printf("test 3- unlink \n" );

	r = hal_mailbox_unlink(mailboxes[mbxid].fd);
	printf("test 4- unlink mailbox: %d fd: %d return: %d\n", mbxid, mailboxes[mbxid].fd, r);
	mailbox_free(mbxid);

	return (r);
}
