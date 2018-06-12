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

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include <nanvix/hal.h>
#include <nanvix/name.h>
#include <nanvix/pm.h>

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
	int fd;    /**< NoC connector. */
	int flags; /**< Flags.         */
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
	int coreid; /* Core ID.       */
	int mbxid;  /* ID of mailbix. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Allocate mailbox. */
	if ((mbxid = mailbox_alloc()) < 0)
		return (-EAGAIN);

	/* Link name. */
	name_link(coreid, name);

	/* Create underlying HW channel. */
	if ((fd = hal_mailbox_create(hal_get_cluster_id())) == -1)
		goto error1;

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;

	return (mbxid);

error1:
	name_unlink(name);
/*
error0:
	mailbox_free(mbxid);
	return (-EAGAIN);
*/
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

	return (hal_mailbox_read(mailboxes[mbxid].fd, buf, MAILBOX_MSG_SIZE));
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
	/* Invalid mailbox ID.*/
	if (!mailbox_is_valid(mbxid))
		return (-EINVAL);

	/* Bad mailbox. */
	if (!mailbox_is_used(mbxid))
		return (-EINVAL);

	/*  Invalid mailbox. */
	if (!mailbox_is_wronly(mbxid))
		return (-EINVAL);

	hal_mailbox_unlink(mailboxes[mbxid].fd);

	mailbox_free(mbxid);

	return (0);
}
