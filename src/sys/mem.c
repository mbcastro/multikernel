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
	struct bdev_message request;
	struct bdev_message reply;

	if (dest & (BLOCK_SIZE - 1))
		return (-EINVAL);

	p = src;
	
	for (size_t i = 0; i < size; i += BLOCK_SIZE)
	{
		size_t n;
		dev_t dev;
		unsigned blknum;
		int channel;

		channel = nanvix_ipc_connect(BDEV_NAME, 0);

		dev = ((dest + i)/BLOCK_SIZE)/(RAMDISK_SIZE/BLOCK_SIZE);
		blknum = ((dest + i)/BLOCK_SIZE)%(RAMDISK_SIZE/BLOCK_SIZE);

		n = ((size - i) < BLOCK_SIZE) ? size - i : BLOCK_SIZE;

		/* Build request. */
		request.type = BDEV_MSG_WRITEBLK_REQUEST;
		request.content.writeblk_req.dev = dev;
		request.content.writeblk_req.blknum = blknum;
		kmemcpy(request.content.writeblk_req.data, p, n);

		nanvix_ipc_send(channel, &request, sizeof(struct bdev_message));

		/* Parse ackowledge message. */
		nanvix_ipc_receive(channel, &reply, sizeof(struct bdev_message));
		if (reply.type == BDEV_MSG_ERROR)
		{
			kpanic("memwrite error");

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
	struct bdev_message request;
	struct bdev_message reply;

	if (src & (BLOCK_SIZE - 1))
		return (-EINVAL);

	p = dest;
	
	for (size_t i = 0; i < size; i += BLOCK_SIZE)
	{
		size_t n;
		dev_t dev;
		unsigned blknum;
		int channel;

		channel = nanvix_ipc_connect(BDEV_NAME, 0);

		dev = ((src + i)/BLOCK_SIZE)/(RAMDISK_SIZE/BLOCK_SIZE);
		blknum = ((src + i)/BLOCK_SIZE)%(RAMDISK_SIZE/BLOCK_SIZE);

		n = ((size - i) < BLOCK_SIZE) ? size - i : BLOCK_SIZE;

		/* Build request. */
		request.type = BDEV_MSG_READBLK_REQUEST;
		request.content.readblk_req.dev = dev;
		request.content.readblk_req.blknum = blknum;

		nanvix_ipc_send(channel, &request, sizeof(struct bdev_message));

		/* Parse ackowledge message. */
		nanvix_ipc_receive(channel, &reply, sizeof(struct bdev_message));
		if (reply.type == BDEV_MSG_ERROR)
		{
			return (NANVIX_FAILURE);
}

		kmemcpy(p, &request.content.readblk_rep.data, n);
		
		p += n;

		nanvix_ipc_close(channel);
	}

	return (0);
}

extern void memclose(void)
{
}
