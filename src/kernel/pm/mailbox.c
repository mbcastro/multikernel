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

/**
 * Mailbox flags.
 */
/**@{*/
#define MAILBOX_FREE (1 << 0)
/**@}*/

/**
 * @brief Maximum length for a mailbox name.
 */
#define MAILBOX_NAMELEN 15

/**
 * @brief Number of mailboxes.
 */
#define NR_MAILBOX 16

/**
 * @brief Mailbox.
 */
struct mailbox
{
	char name[MAILBOX_NAMELEN]; /**< Name.  */
	char flags;                 /**< Flags. */
};

/**
 * @brief Mailbox table.
 */
static struct mailbox mailboxes[NR_MAILBOX];

/**
 * @brief Opens a mailbox.
 *
 * Opens a mailbox named @p name. If the target mailbox does not exist a new
 * one is created.
 *
 * @param name name of the target mailbox.
 *
 * @returns Upon successful completion,the identifier of the target mailbox is
 * returned. Upon failure, a negative error code is returned instead.
 */
int nanvix_mailbox_open(const char *name)
{
	/* Invalid mailbox name. */
	if (name == NULL)
		return (-EINVAL);

	/* Search for mailbox. */
	for (int i = 0; i < NR_MAILBOX; i++)
	{
		/* Found. */
		if (kstrcmp(mailboxes[i].name, name))
			return (i);
	}

	/* Search for an empty slot in the mailbox table. */
	for (int i = 0; i < NR_MAILBOX; i++)
	{
		/* Found. */
		if (mailboxes[i].flags & MAILBOX_FREE)
		{
			kstrncpy(mailboxes[i].name, name, MAILBOX_NAME);
			mailboxes[i].flags &= ~MAILBOX_FREE;
			return (i);
		}
	}

	return (-EAGAIN);
}

/**
 * @brief Sends data through a mailbox.
 *
 * Write @p n bytes from the memory area pointed to by @p buf in the mailbox
 * whose ID is @p mbxid.
 *
 * @param mbxid ID of the target mailbox.
 * @param buf   Pointer to target memory area.
 * @param n     Number of bytes to write.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_mailbox_send(int mbxid, const void *buf, size_t n)
{
	int pid;

	/* Invalid mailbox. */
	if ((mbxid < 0) || (mbxid >= NR_MAILBOX))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid buffer size. */
	if (n <= 0)
		return (-EINVAL);

	pid = nanvix_name_lookup(mailbox[mbxid].name);

	nanvix_connector_write(pid, buf, n);

	return (0);
}

/**
 * @brief Receives data from a mailbox.
 *
 * Reads @p n bytes from the mailbox whose ID is @p mbxid into the memory area
 * pointed to by @p buf.
 *
 * @param mbxid ID of the target mailbox.
 * @param buf   Pointer to target memory area.
 * @param n     Number of bytes to write.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_mailbox_receive(int mbxid, void *buf, size_t n)
{
	int pid;

	/* Invalid mailbox. */
	if ((mbxid < 0) || (mbxid >= NR_MAILBOX))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid buffer size. */
	if (n <= 0)
		return (-EINVAL);

	pid = nanvix_name_lookup(mailbox[mbxid].name);

	nanvix_connector_read(pid, buf, n);

	return (0);
}

/**
 * @brief Unlinks a mailbox.
 *
 * Destroys the mailbox whose ID is @p mbxid.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_mailbox_unlink(int mbxid)
{
	/* Invalid argument. */
	if ((mbxid < 0) || (mbxid > NR_MAILBOX))
		return (mbxid);

	mailbox[outboxid].flags = MAILBOX_FREE;

	return (0);
}
