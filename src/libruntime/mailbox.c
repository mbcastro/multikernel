/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define __NEED_NAME_CLIENT

#include <nanvix/runtime/stdikc.h>
#include <nanvix/servers/name.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/noc.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

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
static int named_inboxes[NANVIX_NODES_NUM];

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

/**
 * @brief Input HAL mailbox.
 */
static int inboxes[NANVIX_NODES_NUM];

/**
 *
 * @brief Is the inbox initialized ?
 */
static int initialized[NANVIX_NODES_NUM] = { 0, };

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

	nodenum = knode_get_num();

	return (initialized[nodenum]);
}

/*============================================================================*
 * nanvix_mailbox_is_valid()                                                  *
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
static inline int nanvix_mailbox_is_valid(int mbxid)
{
	return ((mbxid >=0) && (mbxid < NANVIX_MAILBOX_MAX));
}

/*============================================================================*
 * nanvix_mailbox_is_used()                                                   *
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
static inline int nanvix_mailbox_is_used(int mbxid)
{
	return (mailboxes[mbxid].flags & MAILBOX_USED);
}

/*============================================================================*
 * nanvix_mailbox_is_wronly()                                                 *
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
static inline int nanvix_mailbox_is_wronly(int mbxid)
{
	return (mailboxes[mbxid].flags & MAILBOX_WRONLY);
}

/*===========================================================================*
 * nanvix_mailbox_clear_flags()                                              *
 *===========================================================================*/

/**
 * @brief Clears the flags of a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @note This function is @b NOT thread safe.
 */
static inline void nanvix_mailbox_clear_flags(int mbxid)
{
	mailboxes[mbxid].flags = 0;
}

/*============================================================================*
 * nanvix_mailbox_set_used()                                                  *
 *============================================================================*/

/**
 * @brief Sets a mailbox as in use.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @note This function is @b NOT thread safe.
 */
static inline void nanvix_mailbox_set_used(int mbxid)
{
	mailboxes[mbxid].flags |= MAILBOX_USED;
}

/*============================================================================*
 * nanvix_mailbox_set_wronly()                                                *
 *============================================================================*/

/**
 * @brief Sets a mailbox as write-only.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @note This function is @b NOT thread safe.
 */
static inline void nanvix_mailbox_set_wronly(int mbxid)
{
	mailboxes[mbxid].flags |= MAILBOX_WRONLY;
}

/*============================================================================*
 * nanvix_mailbox_alloc()                                                     *
 *============================================================================*/

/**
 * @brief Allocates a mailbox.
 *
 * @return Upon successful completion, the ID of the newly allocated
 * mailbox is returned. Upon failure, -1 is returned instead.
 *
 * @note this function is @b not thread safe.
 */
static int nanvix_mailbox_alloc(void)
{
	/* Search for a free mailbox. */
	for (int i = 0; i < NANVIX_MAILBOX_MAX; i++)
	{
		/* Found. */
		if (!nanvix_mailbox_is_used(i))
		{
			nanvix_mailbox_set_used(i);
			return (i);
		}
	}

	uprintf("[NAME] mailbox table overflow\n");

	return (-1);
}

/*============================================================================*
 * nanvix_mailbox_free()                                                      *
 *============================================================================*/

/**
 * @brief Frees a mailbox.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @note This function is @b NOT thread safe.
 */
static void nanvix_mailbox_free(int mbxid)
{
	nanvix_mailbox_clear_flags(mbxid);
}

/*============================================================================*
 * nanvix_mailbox_create()                                                    *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
*/
int nanvix_mailbox_create(const char *name)
{
	int fd;      /* NoC connector. */
	int nodenum; /* NoC node.      */
	int mbxid;   /* ID of mailbix. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Check name length. */
	if (ustrlen(name) > KMAILBOX_MESSAGE_SIZE)
		return (-EINVAL);

	/* Runtime not initialized. */
	if ((fd = stdinbox_get()) < 0)
		return (-EAGAIN);

	/* Allocate mailbox. */
	if ((mbxid = nanvix_mailbox_alloc()) < 0)
		return (-EAGAIN);

	nodenum = knode_get_num();

	/* Link name. */
	if (name_link(nodenum, name) != 0)
		goto error0;

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailboxes[mbxid].owner = nodenum;
	ustrcpy(mailboxes[mbxid].name, name);

	/* Initialize named inbox. */
	named_inboxes[core_get_id()] = mbxid;

	return (mbxid);

error0:
	nanvix_mailbox_free(mbxid);
	return (-EAGAIN);
}

/*============================================================================*
 * nanvix_mailbox_open()                                                      *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
*/
int nanvix_mailbox_open(const char *name)
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
	if ((mbxid = nanvix_mailbox_alloc()) < 0)
		return (-EAGAIN);

	/* Open underlying HW channel. */
	if ((fd = kmailbox_open(nodenum)) < 0)
		goto error0;

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailboxes[mbxid].owner = knode_get_num();
	nanvix_mailbox_set_wronly(mbxid);

	return (mbxid);

error0:
	nanvix_mailbox_free(mbxid);
	return (-EAGAIN);
}

/*============================================================================*
 * mailbox_read()                                                             *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_mailbox_read(int mbxid, void *buf, size_t n)
{
	/* Invalid mailbox ID.*/
	if (!nanvix_mailbox_is_valid(mbxid))
		return (-EINVAL);

	/*  Bad mailbox. */
	if (!nanvix_mailbox_is_used(mbxid))
		return (-EINVAL);

	/* Not the owner. */
	if (mailboxes[mbxid].owner != knode_get_num())
		return (-EPERM);

	/* Operation no supported. */
	if (nanvix_mailbox_is_wronly(mbxid))
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Read. */
	if (kmailbox_read(mailboxes[mbxid].fd, buf, n) < 0)
		return (-EINVAL);

	return (0);
}

/*============================================================================*
 * mailbox_write()                                                            *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_mailbox_write(int mbxid, const void *buf, size_t n)
{
	/* Invalid mailbox ID.*/
	if (!nanvix_mailbox_is_valid(mbxid))
		return (-EINVAL);

	/* Bad mailbox. */
	if (!nanvix_mailbox_is_used(mbxid))
		return (-EINVAL);

	/* Not the owner. */
	if (mailboxes[mbxid].owner != knode_get_num())
		return (-EPERM);

	/*  Invalid mailbox. */
	if (!nanvix_mailbox_is_wronly(mbxid))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Write. */
	if (kmailbox_write(mailboxes[mbxid].fd, buf, n) < 0)
		return (-EINVAL);

	return (0);
}

/*============================================================================*
 * mailbox_close()                                                            *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_mailbox_close(int mbxid)
{
	int r;		/* Return value. */

	/* Invalid mailbox ID.*/
	if (!nanvix_mailbox_is_valid(mbxid))
		return (-EINVAL);

	/* Bad mailbox. */
	if (!nanvix_mailbox_is_used(mbxid))
		return (-EINVAL);

	/* Not the owner. */
	if (mailboxes[mbxid].owner != knode_get_num())
		return (-EPERM);

	/*  Invalid mailbox. */
	if (!nanvix_mailbox_is_wronly(mbxid))
		return (-EINVAL);

	if ((r = kmailbox_close(mailboxes[mbxid].fd)) != 0)
		return (r);

	nanvix_mailbox_free(mbxid);

	return (0);
}

/*============================================================================*
 * mailbox_unlink()                                                           *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_mailbox_unlink(int mbxid)
{
	/* Invalid mailbox ID.*/
	if (!nanvix_mailbox_is_valid(mbxid))
		return (-EINVAL);

	/* Bad mailbox. */
	if (!nanvix_mailbox_is_used(mbxid))
		return (-EINVAL);

	/* Not the owner. */
	if (mailboxes[mbxid].owner != knode_get_num())
		return (-EPERM);

	/*  Invalid mailbox. */
	if (nanvix_mailbox_is_wronly(mbxid))
		return (-EINVAL);

	/* Unlink name. */
	if (name_unlink(mailboxes[mbxid].name) != 0)
		return (-EAGAIN);

	nanvix_mailbox_free(mbxid);

	return (0);
}

/*============================================================================*
 * nanvix_mailbox_get_inbox()                                                 *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_mailbox_get_inbox(void)
{
	int local;

	/* Uninitialized named mailbox facility. */
	if (!mailboxes_are_initialized())
	{
		errno = EINVAL;
		return (-1);
	}

	local = knode_get_num();

	return (inboxes[local]);
}

/*============================================================================*
 * nanvix_mailbox_setup()                                                     *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_mailbox_setup(void)
{
	int local;

	local = knode_get_num();

	/* Nothing to do. */
	if (initialized[local])
		return (0);

	/* Initialize named mailboxes facility. */
	inboxes[local] = stdinbox_get();
	initialized[local] = 1;

	return (0);
}

/*============================================================================*
 * nanvix_mailbox_cleanup()                                                   *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_mailbox_cleanup(void)
{
	int local;

	/* Uninitialized named mailbox facility. */
	if (!mailboxes_are_initialized())
		return (-EINVAL);

	local = knode_get_num();

	initialized[local] = 0;

	return (0);
}
