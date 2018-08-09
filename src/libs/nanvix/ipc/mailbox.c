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
#include <string.h>

#include <nanvix/syscalls.h>
#include <nanvix/const.h>
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
 * @brief Input named mailbox.
 */
static int named_inboxes[HAL_NR_NOC_IONODES];

/**
 * @brief Table of mailboxes.
 */
static struct 
{
	int fd;                          /**< NoC connector. */
	int flags;                       /**< Flags.         */
	int owner;                       /**< Owner node.    */
	char name[NANVIX_PROC_NAME_MAX]; /**< Name.          */
} mailboxes[NANVIX_MAILBOX_MAX];

/*============================================================================*
 * mailbox_setup()                                                            *
 *============================================================================*/

/**
 * @brief Input HAL mailbox.
 */
static int inboxes[NANVIX_NR_NODES];

/**
 *
 * @brief Is the inbox initialized ?
 */
static int initialized[NANVIX_NR_NODES] = { 0, };

/**
 * @brief Initializes named mailboxes facility.
 *
 * @param local ID of local node.
 */
int mailbox_setup(int local)
{
	int mbxid;

	/* Invalid local. */
	if ((local < 0) || (local > NANVIX_NR_NODES))
		return (-EINVAL);

	/* Bad local. */
	if (local != sys_get_node_num())
		return (-EINVAL);

	/* Nothing to do. */
	if (initialized[local])
		return (0);

	/* Create underlying unnamed input mailbox. */
	if ((mbxid = sys_mailbox_create(local)) < 0)
		return (mbxid);

	/* Initialize named mailboxes facility. */
	inboxes[local] = mbxid;
	initialized[local] = 1;

	return (0);
}

/*============================================================================*
 * mailboxes_are_initialized()                                                *
 *============================================================================*/

/**
 * @brief Asserts whether or not the named mailbox facility was
 * initialized in the calling node.
 *
 * @returns One if the named mailbox facility was initialized, and zero 
 * otherwise.
 *
 * @note This function is @b NOT thread safe. 
 */
static int mailboxes_are_initialized(void )
{
	int nodenum;

	nodenum = sys_get_node_num();

	return (initialized[nodenum]);
}

/*============================================================================*
 * get_inbox()                                                                *
 *============================================================================*/

/**
 * @brief Returns the underlying unnamed input mailbox.
 */
int get_inbox(void)
{
	int local;

	/* Uninitialized named mailbox facility. */
	if (!mailboxes_are_initialized())
	{
		errno = EINVAL;
		return (-1);
	}

	local = sys_get_node_num();

	return (inboxes[local]);
}

/*============================================================================*
 * mailbox_cleanup()                                                          *
 *============================================================================*/

/**
 * @brief Deinitializes named mailboxes facility.
 */
int mailbox_cleanup(void)
{
	int ret;
	int local;

	/* Uninitialized named mailbox facility. */
	if (!mailboxes_are_initialized())
		return (-EINVAL);

	local = sys_get_node_num();

	/* Destroy underlying unnamed input mailbox. */
	if ((ret = sys_mailbox_unlink(inboxes[local])) < 0)
		return (ret);

	initialized[local] = 0;

	return (0);
}

/**
 * @brief Get named input mailbox.
 */
int get_named_inbox(void)
{
	int index;

	index = sys_get_core_id();

	return (named_inboxes[index]);
}

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
	return ((mbxid >=0) && (mbxid < NANVIX_MAILBOX_MAX));
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
	for (int i = 0; i < NANVIX_MAILBOX_MAX; i++)
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
	int fd;      /* NoC connector. */
	int nodenum; /* NoC node.      */
	int mbxid;   /* ID of mailbix. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Check name length. */
	if (strlen(name) > MAILBOX_MSG_SIZE)
		return (-EINVAL);

	/* Runtime not initialized. */
	if ((fd = get_inbox()) < 0)
		return (-EAGAIN);

	/* Allocate mailbox. */
	if ((mbxid = mailbox_alloc()) < 0)
		return (-EAGAIN);

	nodenum = sys_get_node_num();

	/* Link name. */
	if (name_link(nodenum, name) != 0)
		goto error0;

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailboxes[mbxid].owner = nodenum;
	strcpy(mailboxes[mbxid].name, name);

	/* Initialize named inbox. */
	named_inboxes[sys_get_core_id()] = mbxid;

	return (mbxid);

error0:
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
	int fd;      /* NoC connector. */
	int nodenum; /* NoC node.      */
	int mbxid;   /* ID of mailbix. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Resolve name, */
	if ((nodenum = name_lookup(name)) < 0)
		return (-EAGAIN);

	/* Allocate a mailbox. */
	if ((mbxid = mailbox_alloc()) < 0)
		return (-EAGAIN);

	/* Open underlying HW channel. */
	if ((fd = sys_mailbox_open(nodenum)) < 0)
		goto error0;

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailboxes[mbxid].owner = sys_get_node_num();
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
 * @param n     Number of bytes to write.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int mailbox_read(int mbxid, void *buf, size_t n)
{
	/* Invalid mailbox ID.*/
	if (!mailbox_is_valid(mbxid))
		return (-EINVAL);

	/*  Bad mailbox. */
	if (!mailbox_is_used(mbxid))
		return (-EINVAL);

	/* Not the owner. */
	if (mailboxes[mbxid].owner != sys_get_node_num())
		return (-EPERM);

	/* Operation no supported. */
	if (mailbox_is_wronly(mbxid))
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	if (sys_mailbox_read(mailboxes[mbxid].fd, buf, n) == n)
		return (0);

	return (-EAGAIN);
}

/*============================================================================*
 * mailbox_write()                                                            *
 *============================================================================*/

/**
 * @brief Writes data to a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 * @param buf   Location from where data should be read.
 * @param n     Number of bytes to write.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int mailbox_write(int mbxid, const void *buf, size_t n)
{
	/* Invalid mailbox ID.*/
	if (!mailbox_is_valid(mbxid))
		return (-EINVAL);

	/* Bad mailbox. */
	if (!mailbox_is_used(mbxid))
		return (-EINVAL);

	/* Not the owner. */
	if (mailboxes[mbxid].owner != sys_get_node_num())
		return (-EPERM);

	/*  Invalid mailbox. */
	if (!mailbox_is_wronly(mbxid))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	if (sys_mailbox_write(mailboxes[mbxid].fd, buf, n) == n)
		return (0);

	return (-EAGAIN);
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
	int r;		/* Return value. */

	/* Invalid mailbox ID.*/
	if (!mailbox_is_valid(mbxid))
		return (-EINVAL);

	/* Bad mailbox. */
	if (!mailbox_is_used(mbxid))
		return (-EINVAL);

	/* Not the owner. */
	if (mailboxes[mbxid].owner != sys_get_node_num())
		return (-EPERM);

	/*  Invalid mailbox. */
	if (!mailbox_is_wronly(mbxid))
		return (-EINVAL);

	if ((r = sys_mailbox_close(mailboxes[mbxid].fd)) != 0)
		return (r);

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

	/* Not the owner. */
	if (mailboxes[mbxid].owner != sys_get_node_num())
		return (-EPERM);

	/*  Invalid mailbox. */
	if (mailbox_is_wronly(mbxid))
		return (-EINVAL);

	/* Unlink name. */
	if (name_unlink(mailboxes[mbxid].name) != 0)
		return (-EAGAIN);

	mailbox_free(mbxid);

	return (0);
}
