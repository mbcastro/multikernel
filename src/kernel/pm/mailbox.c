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
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

/**
 * @brief Mailbox flags.
 */
/**@{*/
#define MAILBOX_USED (1 << 0)
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
static struct mailbox mailboxes[NR_MAILBOX] = { {'\0', 0}, };

/**
 * @brief Mailbox names.
 */
const struct {
	int id;     /**< Cluster ID. */
	char *name; /**< Mailbox name. */
} names[NR_CCLUSTER] = {
	{ CCLUSTER0,  "/cpu0" },
	{ CCLUSTER1,  "/cpu1" },
	{ CCLUSTER2,  "/cpu2" },
	{ CCLUSTER3,  "/cpu3" },
	{ CCLUSTER4,  "/cpu4" },
	{ CCLUSTER5,  "/cpu5" },
	{ CCLUSTER6,  "/cpu6" },
	{ CCLUSTER7,  "/cpu7" },
	{ CCLUSTER8,  "/cpu8" },
	{ CCLUSTER9,  "/cpu9" },
	{ CCLUSTER10, "/cpu10" },
	{ CCLUSTER11, "/cpu11" },
	{ CCLUSTER12, "/cpu12" },
	{ CCLUSTER13, "/cpu13" },
	{ CCLUSTER14, "/cpu14" },
	{ CCLUSTER15, "/cpu15" }
};


/**
 * @brief Translates a mailbox name to a NoC connector ID.
 *
 * @returns Upon successful completion, the NoC connector ID associated to the
 * target mailbox is returned returned. Upon failure, a negative error code is
 * returned instead.
 */
static int nanvix_name_lookup(const char *name)
{
	/* Search for mailbox name. */
	for (int i = 0; i < NR_CCLUSTER; i++)
	{
		/* Found. */
		if (!strcmp(names[i].name, name))
			return (names[i].id);
	}

	return (-ENOENT);
}

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
		if (!strcmp(mailboxes[i].name, name))
			return (i);
	}

	/* Search for an empty slot in the mailbox table. */
	for (int i = 0; i < NR_MAILBOX; i++)
	{
		/* Found. */
		if (!(mailboxes[i].flags & MAILBOX_USED))
		{
			strncpy(mailboxes[i].name, name, MAILBOX_NAMELEN);
			mailboxes[i].flags |= MAILBOX_USED;
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
	int connector;

	/* Invalid mailbox. */
	if ((mbxid < 0) || (mbxid >= NR_MAILBOX))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid buffer size. */
	if (n < 1)
		return (-EINVAL);

	connector = nanvix_name_lookup(mailboxes[mbxid].name);

	nanvix_noc_send(connector, buf, n);

	return (0);
}

/**
 * @brief Receives data through a mailbox.
 *
 * Reads @p n bytes from the mailbox of the calling process into the memory
 * area pointed to by @p buf.
 *
 * @param mbxid ID of the target mailbox.
 * @param buf   Pointer to target memory area.
 * @param n     Number of bytes to read.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_mailbox_receive(void *buf, size_t n)
{
	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid buffer size. */
	if (n < 1)
		return (-EINVAL);

	nanvix_noc_receive(buf, n);

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
	/* Invalid mailbox ID. */
	if ((mbxid < 0) || (mbxid > i= NR_MAILBOX))
		return (-EINVAL);

	mailboxes[mbxid].flags = 0;

	return (0);
}
