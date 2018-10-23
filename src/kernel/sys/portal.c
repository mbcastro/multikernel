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

#define __NEED_HAL_NOC_
#define __NEED_HAL_PORTAL_
#include <nanvix/hal.h>
#include <nanvix/klib.h>

/**
 * @brief Creates a portal.
 *
 * @param nodenum Target NoC node.
 *
 * @returns Upon successful completion, the ID of a newly created
 * portal is returned. Upon failure, a negative error code is returned
 * instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_portal_create(int nodenum)
{
	/* Invalid node. */
	if ((nodenum < 0) || (nodenum >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	return (hal_portal_create(hal_noc_nodes[nodenum]));
}

/**
 * @brief Enables read operations from a nodenum.
 *
 * @param portalid ID of the target portal.
 * @param nodenum  Target NoC node.
 *
 * @returns Upons successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_portal_allow(int portalid, int nodenum)
{
	/* Invalid node. */
	if ((nodenum < 0) || (nodenum >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	return (hal_portal_allow(portalid, hal_noc_nodes[nodenum]));
}

/**
 * @brief Opens a portal.
 *
 * @param nodenum Target NoC node.
 *
 * @returns Upon successful completion, the ID of the target portal is
 * returned. Upon failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_portal_open(int nodenum)
{
	/* Invalid node. */
	if ((nodenum < 0) || (nodenum >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	return (hal_portal_open(hal_noc_nodes[nodenum]));
}

/**
 * @brief Destroys a portal.
 *
 * @param portalid ID of the target portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_portal_unlink(int portalid)
{
	return (hal_portal_unlink(portalid));
}

/**
 * @brief Closes a portal.
 *
 * @param portalid ID of target portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_portal_close(int portalid)
{
	return (hal_portal_close(portalid));
}

/**
 * @brief Waits for an asynchronous operation on a portal to complete.
 *
 * @param portalid ID of target portal.
 *
 * @returns Upon successful completion, the number of bytes
 * read/written is returned. Upon failure, a negative error code is
 * returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
ssize_t sys_portal_wait(int portalid)
{
	return (hal_portal_wait(portalid));
}

/**
 * @brief Reads data asynchronously from a portal.
 *
 * @param portalid ID of target portal.
 * @param buf      Location from where data should be written.
 * @param n        Number of bytes to read.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_portal_aread(int portalid, void *buf, size_t n)
{
	/* Invalid buffer size. */
	if ((n < 1) || (n > HAL_PORTAL_MAX_SIZE))
		return (-EINVAL);

	return (hal_portal_aread(portalid, buf, n));
}

/**
 * @brief Reads data from a portal.
 *
 * @param portalid ID of target portal.
 * @param buf      Location from where data should be written.
 * @param n        Number of bytes to read.
 *
 * @returns Upon successful completion, the number of bytes read is
 * returned. Upon failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
ssize_t sys_portal_read(int portalid, void *buf, size_t n)
{
	/* Invalid buffer size. */
	if ((n < 1) || (n > HAL_PORTAL_MAX_SIZE))
		return (-EINVAL);

	return (hal_portal_read(portalid, buf, n));
}

/**
 * @brief Writes data asynchronously to a portal.
 *
 * @param portalid ID of target portal.
 * @param buf      Location from where data should be read.
 * @param n        Number of bytes to write.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_portal_awrite(int portalid, const void *buf, size_t n)
{
	/* Invalid buffer size. */
	if ((n < 1) || (n > HAL_PORTAL_MAX_SIZE))
		return (-EINVAL);

	return (hal_portal_awrite(portalid, buf, n));
}

/**
 * @brief Writes data to a portal.
 *
 * @param portalid ID of target portal.
 * @param buf      Location from where data should be read.
 * @param n        Number of bytes to write.
 *
 * @returns Upon successful completion, the number of bytes written is
 * returned. Upon failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
ssize_t sys_portal_write(int portalid, const void *buf, size_t n)
{
	/* Invalid buffer size. */
	if ((n < 1) || (n > HAL_PORTAL_MAX_SIZE))
		return (-EINVAL);

	return (hal_portal_write(portalid, buf, n));
}

/**
 * @brief Performs control operations in a portal.
 *
 * @param portalid Target portal.
 * @param request  Request.
 *
 * @param Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int sys_portal_ioctl(int portalid, unsigned request, ...)
{
	int ret;
	va_list args;

	va_start(args, request);
	ret = hal_portal_ioctl(portalid, request, args);
	va_end(args);

	return (ret);
}

