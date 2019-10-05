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

#define __NEED_NAME_CLIENT

#include <nanvix/runtime/stdikc.h>
#include <nanvix/servers/name.h>
#include <nanvix/sys/portal.h>
#include <nanvix/sys/noc.h>
#include <nanvix/limits.h>
#include <ulibc/assert.h>
#include <ulibc/stdio.h>
#include <ulibc/string.h>
#include <posix/errno.h>

/**
 * @brief Portal flags.
 */
/**@{*/
#define PORTAL_USED   (1 << 0) /**< Used?	   */
#define PORTAL_WRONLY (1 << 1) /**< Write-only? */
/**@}*/

/**
 * @brief Table of portals.
 */
static struct
{
	int portalid;                    /**< Underlying unnamed portal. */
	int flags;                       /**< Flags.                     */
	int owner;                       /**< Owner node.                */
	char name[NANVIX_PROC_NAME_MAX]; /**< Name.                      */
} portals[NANVIX_PORTAL_MAX];

/*============================================================================*
 * nanvix_portal_is_valid()                                                   *
 *============================================================================*/

/**
 * @brief Asserts whether or not a portal is valid.
 *
 * @param id ID of the target portal.
 *
 * @returns One if the portal is valid, and zero otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static inline int nanvix_portal_is_valid(int id)
{
	return ((id >=0) && (id < NANVIX_PORTAL_MAX));
}

/*============================================================================*
 * nanvix_portal_is_used()                                                    *
 *============================================================================*/

/**
 * @brief Asserts whether or not a portal is in use.
 *
 * @param id ID of the target portal.
 *
 * @returns One if the portal is in use, and zero otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static inline int nanvix_portal_is_used(int id)
{
	return (portals[id].flags & PORTAL_USED);
}

/*============================================================================*
 * nanvix_portal_is_wronly()                                                  *
 *============================================================================*/

/**
 * @brief Asserts whether or not a portal is write-only.
 *
 * @param id ID of the target portal.
 *
 * @returns One if the portal is write-only and false otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static inline int nanvix_portal_is_wronly(int id)
{
	return (portals[id].flags & PORTAL_WRONLY);
}

/*===========================================================================*
 * nanvix_portal_clear_flags()                                               *
 *===========================================================================*/

/**
 * @brief Clears the flags of a portal.
 *
 * @param id ID of the target portal.
 *
 * @note This function is @b NOT thread safe.
 */
static inline void nanvix_portal_clear_flags(int id)
{
	portals[id].flags = 0;
}

/*============================================================================*
 * nanvix_portal_set_used()                                                   *
 *============================================================================*/

/**
 * @brief Sets a portal as in use.
 *
 * @param id ID of the target portal.
 *
 * @note This function is @b NOT thread safe.
 */
static inline void nanvix_portal_set_used(int id)
{
	portals[id].flags |= PORTAL_USED;
}

/*============================================================================*
 * nanvix_portal_set_wronly()                                                 *
 *============================================================================*/

/**
 * @brief Sets a portal as write-only.
 *
 * @param id ID of the target portal.
 *
 * @note This function is @b NOT thread safe.
 */
static inline void nanvix_portal_set_wronly(int id)
{
	portals[id].flags |= PORTAL_WRONLY;
}

/*============================================================================*
 * nanvix_portal_alloc()                                                      *
 *============================================================================*/

/**
 * @brief Allocates a portal.
 *
 * @return Upon successful completion, the ID of the newly allocated
 * portal is returned. Upon failure, -1 is returned instead.
 *
 * @note this function is @b not thread safe.
 */
static int nanvix_portal_alloc(void)
{
	/* Search for a free portal. */
	for (int i = 0; i < NANVIX_PORTAL_MAX; i++)
	{
		/* Found. */
		if (!nanvix_portal_is_used(i))
		{
			nanvix_portal_set_used(i);
			return (i);
		}
	}

	kprintf("[nanvix][runtime][ipc][portal] portal table overflow\n");

	return (-1);
}

/*============================================================================*
 * __nanvix_portal_setup()                                                    *
 *============================================================================*/

/**
 * @brief input portals.
 */
static int inportals[NANVIX_NODES_NUM];

/**
 * @brief Is the named portals facility initialized?
 */
static int initialized[NANVIX_NODES_NUM] = { 0 , };

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_portal_setup(int local)
{
	int portalid;

	/* Invalid local. */
	if ((local < 0) || (local > NANVIX_NODES_NUM))
		return (-EINVAL);

	/* Bad local. */
	if (local != knode_get_num())
		return (-EINVAL);

	/* Nothing to do. */
	if (initialized[local])
		return (0);

	/* Create underlying unnamed input portal. */
	if ((portalid = kportal_create(local)) < 0)
		return (portalid);

	/* Initialize named portals facility. */
	inportals[local] = portalid;
	initialized[local] = 1;

	return (0);
}

/*============================================================================*
 * portals_are_initialized()                                                  *
 *============================================================================*/

/**
 * @brief Asserts whether or not the named portal facility was
 * initialized in the calling node.
 *
 * @returns One if the named portal facility was initialized, and zero
 * otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static int portals_are_initialized(void )
{
	int nodenum;

	nodenum = knode_get_num();

	return (initialized[nodenum]);
}

/*============================================================================*
 * get_inportal()                                                             *
 *============================================================================*/

/**
 * @brief Returns the underlying unnamed input portal.
 */
int get_inportal(void)
{
	int local;

	/* Uninitialized named portal facility. */
	if (!portals_are_initialized())
		return (-EINVAL);

	local = knode_get_num();

	return (inportals[local]);
}

/*============================================================================*
 * __nanvix_portal_cleanup()                                                  *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_portal_cleanup(void)
{
	int ret;
	int local;

	/* Uninitialized named portal facility. */
	if (!portals_are_initialized())
		return (-EINVAL);

	local = knode_get_num();

	/* Destroy underlying unnamed input portal. */
	if ((ret = kportal_unlink(inportals[local])) < 0)
		return (ret);

	initialized[local] = 0;

	return (0);
}

/*============================================================================*
 * nanvix_portal_free()                                                       *
 *============================================================================*/

/**
 * @brief Frees a portal.
 *
 * @param id ID of the target portal.
 *
 * @note This function is @b NOT thread safe.
 */
static void nanvix_portal_free(int id)
{
	nanvix_portal_clear_flags(id);
}

/*============================================================================*
 * nanvix_portal_create()                                                     *
 *============================================================================*/

/**
 * @brief Creates a portal.
 *
 * @param name Portal name.
 *
 * @returns Upon successful completion, the ID of the new portal is
 * returned. Upon failure, a negative error code is returned instead.
 */
int nanvix_portal_create(const char *name)
{
	int id;       /* Portal ID.                 */
	int nodenum;  /* NoC node.                  */
	int portalid; /* Underlying unnamed portal. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Check name length. */
	if (nanvix_strlen(name) > MAILBOX_MSG_SIZE)
		return (-EINVAL);

	/* Runtime not initialized. */
	if ((portalid = get_inportal()) < 0)
		return (-EAGAIN);

	/* Allocate portal. */
	if ((id = nanvix_portal_alloc()) < 0)
		return (-EAGAIN);

	nodenum = knode_get_num();

	/* Link name. */
	if (name_link(nodenum, name) != 0)
		goto error0;

	/* Initialize portal. */
	portals[id].portalid = portalid;
	portals[id].owner = nodenum;
	nanvix_strcpy(portals[id].name, name);

	return (id);

error0:
	nanvix_portal_free(id);
	return (-EAGAIN);
}

/*============================================================================*
 * nanvix_portal_allow()                                                      *
 *============================================================================*/

/**
 * @brief Enables read operations on a portal.
 *
 * @param id	   ID of the target portal.
 * @param nodenum  Target node.
 *
 * @returns Upons successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @note This function is NOT thread-safe.
 */
int nanvix_portal_allow(int id, int nodenum)
{
	/* Invalid portal ID.*/
	if (!nanvix_portal_is_valid(id))
		return (-EINVAL);

	/*  Bad portal. */
	if (!nanvix_portal_is_used(id))
		return (-EINVAL);

	/* Operation no supported. */
	if (nanvix_portal_is_wronly(id))
		return (-ENOTSUP);

	/* Not the owner. */
	if (portals[id].owner != knode_get_num())
		return (-EINVAL);

	return (kportal_allow(portals[id].portalid, nodenum));
}

/*============================================================================*
 * nanvix_portal_open()                                                       *
 *============================================================================*/

/**
 * @brief Opens a portal.
 *
 * @param name Portal name.
 *
 * @returns Upon successful completion, the ID of the target portal is
 * returned. Upon failure, a negative error code is returned instead.
 */
int nanvix_portal_open(const char *name)
{
	int id;	   /* Portal ID.				 */
	int nodenum;  /* NoC node.				  */
	int portalid; /* Underlying unnamed portal. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Resolve name. */
	if ((nodenum = name_lookup(name)) < 0)
		return (-EAGAIN);

	/* Allocate a portal. */
	if ((id = nanvix_portal_alloc()) < 0)
		return (-EAGAIN);

	/* Open underlying unnamed portal. */
	if ((portalid = kportal_open(knode_get_num(), nodenum)) < 0)
		goto error0;

	/* Initialize portal. */
	portals[id].portalid = portalid;
	portals[id].owner = knode_get_num();
	nanvix_portal_set_wronly(id);

	return (id);

error0:
	nanvix_portal_free(id);
	return (-EAGAIN);
}

/*============================================================================*
 * nanvix_portal_read()                                                       *
 *============================================================================*/

/**
 * @brief Reads data from a portal.
 *
 * @param id  ID of the target portal.
 * @param buf Location from where data should be written.
 * @param n   Number of bytes to write.
 *
 * @returns See kportal_aread().
 */
int nanvix_portal_read(int id, void *buf, size_t n)
{
	int ret;

	/* Invalid portal ID.*/
	if (!nanvix_portal_is_valid(id))
		return (-EINVAL);

	/*  Bad portal. */
	if (!nanvix_portal_is_used(id))
		return (-EINVAL);

	/* Operation no supported. */
	if (nanvix_portal_is_wronly(id))
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid buffer size. */
	if (n < 1)
		return (-EINVAL);

	/* Not the owner. */
	if (portals[id].owner != knode_get_num())
		return (-EINVAL);

	ret = kportal_aread(portals[id].portalid, buf, n);

	return (ret);
}

/*============================================================================*
 * nanvix_portal_write()                                                      *
 *============================================================================*/

/**
 * @brief Writes data to a portal.
 *
 * @param id  ID of the target portal.
 * @param buf Location from where data should be read.
 * @param n   Number of bytes to write.
 *
 * @returns See kportal_awrite();
 */
int nanvix_portal_write(int id, const void *buf, size_t n)
{
	int ret;

	/* Invalid portal ID.*/
	if (!nanvix_portal_is_valid(id))
		return (-EINVAL);

	/* Bad portal. */
	if (!nanvix_portal_is_used(id))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!nanvix_portal_is_wronly(id))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid buffer. */
	if (n < 1)
		return (-EINVAL);

	/* Not the owner. */
	if (portals[id].owner != knode_get_num())
		return (-EINVAL);

	ret = kportal_awrite(portals[id].portalid, buf, n);

	return (ret);
}

/*============================================================================*
 * nanvix_portal_close()                                                      *
 *============================================================================*/

/**
 * @brief Closes a portal.
 *
 * @param id ID of the target portal.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int nanvix_portal_close(int id)
{
	int err;

	/* Invalid portal ID.*/
	if (!nanvix_portal_is_valid(id))
		return (-EINVAL);

	/* Bad portal. */
	if (!nanvix_portal_is_used(id))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!nanvix_portal_is_wronly(id))
		return (-EINVAL);

	/* Not the owner. */
	if (portals[id].owner != knode_get_num())
		return (-EINVAL);

	/* Close underlying unnamed portal. */
	if ((err = kportal_close(portals[id].portalid)) != 0)
		return (err);

	nanvix_portal_free(id);

	return (0);
}

/*============================================================================*
 * nanvix_portal_unlink()                                                     *
 *============================================================================*/

/**
 * @brief Destroys a portal.
 *
 * @param id ID of the target portal.
 *
 * @returns Upon successful completion zero, is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int nanvix_portal_unlink(int id)
{
	/* Invalid portal ID.*/
	if (!nanvix_portal_is_valid(id))
		return (-EINVAL);

	/* Bad portal. */
	if (!nanvix_portal_is_used(id))
		return (-EINVAL);

	/*  Invalid portal. */
	if (nanvix_portal_is_wronly(id))
		return (-EINVAL);

	/* Unlink name. */
	if (name_unlink(portals[id].name) != 0)
		return (-EAGAIN);

	/* Not the owner. */
	if (portals[id].owner != knode_get_num())
		return (-EINVAL);

	/*
	 * Underlying unnamed mailbox is
	 * destroyed when unloading the runtime system
	 */

	nanvix_portal_free(id);

	return (0);
}
