/*
 * Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include <nanvix/dev.h>
#include <nanvix/ramdisk.h>
#include <nanvix/klib.h>
#include <nanvix/ipc.h>

/**
 * @brief Remote memory address.
 */
struct memaddr
{
	dev_t dev;       /**< Device ID.    */
	unsigned blknum; /**< Block number. */
};

/**
 * @brief Maps a linear address into a remote memory address.
 *
 * @param addr Linear address.
 *
 * @returns Remote memory address.
 */
static struct memaddr memmap(uint64_t addr)
{
	struct memaddr memaddr;

	memaddr.dev = (addr/BLOCK_SIZE)/(RAMDISK_SIZE/BLOCK_SIZE);
	memaddr.blknum = (addr/BLOCK_SIZE)%(RAMDISK_SIZE/BLOCK_SIZE);

	return (memaddr);
}

/**
 * @brief Writes to remote memory.
 *
 * @param src  Pointer to source memory area.
 * @param dest Target memory area.
 * @param size Write size.
 *
 * @returns Upon successful completion, zero is returned;
 * otherwise a negative error code is returned instead.
 */
int memwrite(const void *src, uint64_t dest, size_t size)
{
	const char *p;

	if (dest & (BLOCK_SIZE - 1))
		return (-EINVAL);

	p = src;
	
	for (size_t i = 0; i < size; i += BLOCK_SIZE)
	{
		size_t n;
		int channel;
		struct memaddr memaddr;
		struct bdev_msg msg;

		channel = nanvix_ipc_connect(BDEV_NAME, 0);

		memaddr = memmap(dest + i);

		n = ((size - i) < BLOCK_SIZE) ? size - i : BLOCK_SIZE;

		/* Build msg. */
		msg.type = BDEV_MSG_WRITEBLK_REQUEST;
		msg.content.writeblk_req.dev = memaddr.dev;
		msg.content.writeblk_req.blknum = memaddr.blknum;
		kmemcpy(msg.content.writeblk_req.data, p, n);

		nanvix_ipc_send(channel, &msg, sizeof(struct bdev_msg));

		/* Parse ackowledge message. */
		nanvix_ipc_receive(channel, &msg, sizeof(struct bdev_msg));
		if (msg.type == BDEV_MSG_ERROR)
		{
			kpanic("memwrite error");

			nanvix_ipc_close(channel);
			return (NANVIX_FAILURE);
		}

		p += n;

		nanvix_ipc_close(channel);
	}

	return (0);
}

/**
 * @brief Reads from remote memory.
 *
 * @param src  Source memory area.
 * @param dest Pointer to target memory area.
 * @param size Read size.
 *
 * @returns Upon successful completion, zero is returned;
 * otherwise a negative error code is returned instead.
 */
int memread(void *dest, uint64_t src, size_t size)
{
	char *p;

	if (src & (BLOCK_SIZE - 1))
		return (-EINVAL);

	p = dest;
	
	for (size_t i = 0; i < size; i += BLOCK_SIZE)
	{
		size_t n;
		int channel;
		struct memaddr memaddr;
		struct bdev_msg msg;

		channel = nanvix_ipc_connect(BDEV_NAME, 0);

		memaddr = memmap(src + i);

		n = ((size - i) < BLOCK_SIZE) ? size - i : BLOCK_SIZE;

		/* Build msg. */
		msg.type = BDEV_MSG_READBLK_REQUEST;
		msg.content.readblk_req.dev = memaddr.dev;
		msg.content.readblk_req.blknum = memaddr.blknum;

		nanvix_ipc_send(channel, &msg, sizeof(struct bdev_msg));

		/* Parse ackowledge message. */
		nanvix_ipc_receive(channel, &msg, sizeof(struct bdev_msg));
		if (msg.type == BDEV_MSG_ERROR)
		{
			kpanic("memread error");

			nanvix_ipc_close(channel);
			return (NANVIX_FAILURE);
		}

		kmemcpy(p, &msg.content.readblk_rep.data, n);
		
		p += n;

		nanvix_ipc_close(channel);
	}

	return (0);
}
