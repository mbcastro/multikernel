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
#define __NEED_HAL_MAILBOX_
#include <nanvix/hal.h>
#include <nanvix/klib.h>

/**
 * @brief Creates a mailbox.
 *
 * @param nodenum Target NoC node.
 *
 * @returns Upon successful completion, the ID of the newly created
 * mailbox is returned. Upon failure, a negative error code is
 * returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_mailbox_create(int nodenum)
{
	/* Invalid node. */
	if ((nodenum < 0) || (nodenum >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	return (hal_mailbox_create(hal_noc_nodes[nodenum]));
}

/**
 * @brief Opens a mailbox.
 *
 * @param nodenum Target NoC node.
 *
 * @returns Upon successful completion, the ID of the target mailbox
 * is returned. Upon failure, a negative error code is returned
 * instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int sys_mailbox_open(int nodenum)
{
	/* Invalid node. */
	if ((nodenum < 0) || (nodenum >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	return (hal_mailbox_open(hal_noc_nodes[nodenum]));
}

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
int sys_mailbox_unlink(int mbxid)
{
	return (hal_mailbox_unlink(mbxid));
}

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
int sys_mailbox_close(int mbxid)
{
	return (hal_mailbox_close(mbxid));
}

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
size_t sys_mailbox_write(int mbxid, const void *buf, size_t n)
{
	return (hal_mailbox_write(mbxid, buf, n));
}

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
size_t sys_mailbox_read(int mbxid, void *buf, size_t n)
{
	return (hal_mailbox_read(mbxid, buf, n));
}
