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

#include <nanvix/dev.h>
#include <nanvix/ramdisk.h>
#include <nanvix/klib.h>
#include <nanvix/ipc.h>

int channel;

int memopen(void)
{
	channel = nanvix_ipc_connect(BDEV_NAME, 0);

	return (0);
}

int memwrite(const void *src, uint64_t dest, size_t size)
{
	const char *p;
	struct bdev_message request;
	struct bdev_message reply;

	p = src;
	
	for (size_t i = 0; i < size; i += BLOCK_SIZE)
	{
		size_t n;
		dev_t dev;
		unsigned blknum;

		dev = (dest + i)/RAMDISK_SIZE;
		blknum = (dest + i)%RAMDISK_SIZE;

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
			return (NANVIX_FAILURE);

		p += n;
	}

	return (0);
}

int memread(void *dest, uint64_t src, size_t size)
{
	char *p;
	struct bdev_message request;
	struct bdev_message reply;

	p = dest;
	
	for (size_t i = 0; i < size; i += BLOCK_SIZE)
	{
		size_t n;
		dev_t dev;
		unsigned blknum;

		dev = (src + i)/RAMDISK_SIZE;
		blknum = (src + i)%RAMDISK_SIZE;

		n = ((size - i) < BLOCK_SIZE) ? size - i : BLOCK_SIZE;

		/* Build request. */
		request.type = BDEV_MSG_READBLK_REQUEST;
		request.content.readblk_req.dev = dev;
		request.content.readblk_req.blknum = blknum;

		nanvix_ipc_send(channel, &request, sizeof(struct bdev_message));

		/* Parse ackowledge message. */
		nanvix_ipc_receive(channel, &reply, sizeof(struct bdev_message));
		if (reply.type == BDEV_MSG_ERROR)
			return (NANVIX_FAILURE);

		kmemcpy(p, &request.content.readblk_rep.data, n);
		
		p += n;
	}

	return (0);
}

extern void memclose(void)
{
	nanvix_ipc_close(channel);
}
