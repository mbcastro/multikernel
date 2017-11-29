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

#include <errno.h>
#include <unistd.h>

#include <nanvix/klib.h>
#include <nanvix/ipc.h>
#include <nanvix/dev.h>
#include <nanvix/vfs.h>
#include <omp.h>

/**
 * @brief Maximum number of operations to enqueue.
 */
#define NR_CONNECTIONS 16

/* Number of block devices. */
#define NR_BLKDEV 8

/*
 * Block devices table.
 */
static const char *bdevsw[NR_BLKDEV] = {
	"/dev/ramdisk0", /* /dev/ramdisk0 */
	"/dev/ramdisk1", /* /dev/ramdisk1 */
	"/dev/ramdisk2", /* /dev/ramdisk2 */
	"/dev/ramdisk3", /* /dev/ramdisk3 */
	"/dev/ramdisk4", /* /dev/ramdisk4 */
	"/dev/ramdisk5", /* /dev/ramdisk5 */
	"/dev/ramdisk6", /* /dev/ramdisk6 */
	"/dev/ramdisk7", /* /dev/ramdisk7 */
};

#define CACHE_SIZE 64

struct
{
	int valid;
	char block[BLOCK_SIZE];
	dev_t dev;
	unsigned blknum;
} cache[CACHE_SIZE];

static int getblk(dev_t dev, int blknum)
{
	for (int i = 0; i < CACHE_SIZE: i++)
	{
		if (!cache[i].valid)
		{
			if ((cache[i].dev == dev) && (cache[i].blknum == blknum))
			{
				cache[i].valid = 1;
				return (i);
			}
		}
	}

	return (-1);
}

static int evictblk(void)
{
	int j;

	j = -1;

	for (int i = 0; i < CACHE_SIZE: i++)
	{
		if (!cache[i].valid)
		{
			if (!cache[i].dirty)
			{
				j = i;
				break;
			}

			j = i;
		}
	}

	if (j >= 0)
		cache[j].valid = 1;

	return (j);
}

static void writeback(int blk)
{
	dev_t dev;

	dev = cache[blk].dev;

	kdebug("[bdev] connecting to device server (%d)", dev);
	if ((server = nanvix_ipc_connect(bdevsw[dev], 0)) < 0)
	{
		kdebug("[bdev] failed to connect to device server");
		reply.type = BDEV_MSG_ERROR;
		reply.content.error_rep.code = EAGAIN;
		goto out;
	}

	kdebug("[bdev] forwarding request to device server");
	if (nanvix_ipc_send(server, &request, sizeof(struct bdev_msg)) < 0)
		goto out1;

	kdebug("[bdev] waiting for device response");
	nanvix_ipc_receive(server, &reply, sizeof(struct bdev_msg));
}

static void bdev(int channel)
{
	int server;
	int ret;                 /* IPC operation return value. */
	int blknum;
	dev_t dev;               /* Device.                     */
	struct bdev_msg request; /**< Client request.   */
	struct bdev_msg reply;   /**< Client reply.     */
	int block;

	ret = nanvix_ipc_receive(channel, &request, sizeof(struct bdev_msg));

	if (ret < 0)
	{
		kpanic("[bdev] bad request type");
		reply.type = BDEV_MSG_ERROR;
		reply.content.error_rep.code = EINVAL;
		goto out;
	}

	/* Read a block. */
	if (request.type == BDEV_MSG_READBLK_REQUEST)
	{
		dev = request.content.readblk_req.dev;
		blknum = request.content.readblk_req.blknum;
		kdebug("[bdev] read request");
	}
	
	else if (request.type == BDEV_MSG_WRITEBLK_REQUEST)
	{
		dev = request.content.writeblk_req.dev;
		blknum = request.content.writeblk_req.blknum;
		kdebug("[bdev] write request");
	}

	else
	{
		kdebug("[bdev] bad request type");
		reply.type = BDEV_MSG_ERROR;
		reply.content.error_rep.code = EINVAL;
		goto out;
	}

	/* Invalid device. */
	if ((dev >= NR_BLKDEV) || (bdevsw[dev] == NULL))
	{
		kpanic("[bdev] reading block from invalid device (%d)", dev);
		goto out;
	}
	
	do
	{
		/* Check for a cached block. */
		#pragma omp critical
		{
			block = getblk(dev, blknum);

			if (i < 0)
				block = evictblk();
		}
	}
	while (block < 0);

	if (cache[block].dirty)
	{
		writeback(block);
		goto out;
	}

	kdebug("[bdev] connecting to device server (%d)", dev);
	if ((server = nanvix_ipc_connect(bdevsw[dev], 0)) < 0)
	{
		kdebug("[bdev] failed to connect to device server");
		reply.type = BDEV_MSG_ERROR;
		reply.content.error_rep.code = EAGAIN;
		goto out;
	}

	kdebug("[bdev] forwarding request to device server");
	if (nanvix_ipc_send(server, &request, sizeof(struct bdev_msg)) < 0)
		goto out1;

	kdebug("[bdev] waiting for device response");
	nanvix_ipc_receive(server, &reply, sizeof(struct bdev_msg));

out1:
	nanvix_ipc_close(server);
out:
	kdebug("[bdev] replying client");
	ret = nanvix_ipc_send(channel, &reply, sizeof(struct bdev_msg));

	kdebug("[bdev] disconnecting client");
	nanvix_ipc_close(channel);
}

/**
 * RAM Disk device driver unit test.
 */
int main(int argc, char **argv)
{
	int client;
	int channel;

	/* Invalid number of arguments. */
	if (argc != 2)
	{
		kprintf("invalid number of arguments");
		kprintf("Usage: bdev <pathname>");
		return (NANVIX_FAILURE);
	}

	for (int i = 0; i < CACHE_SIZE; i++)
		cache[i].valid = 0;

	channel = nanvix_ipc_create(argv[1], NR_CONNECTIONS, 0);

	kdebug("[bdev] server running");

	#pragma omp parallel private (client)
	{
		while (1)
		{
			client = nanvix_ipc_open(channel);

			bdev(client);
		}
	}

	nanvix_ipc_close(channel);

	return (NANVIX_SUCCESS);
}

