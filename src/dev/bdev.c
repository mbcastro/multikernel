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

/*============================================================================*
 * Memory Block Cache
 *============================================================================*/

/**
 * Memory block cache size (in blocks).
 */
#define CACHE_SIZE 64

/**
 * Memory block cache.
 */
struct
{
	/* Flags. */
	int valid;  /** Is valid?      */
	int dirty;  /**< Dirty block?  */
	int locked; /**< Locked block. */

	/* Index. */
	struct
	{
		dev_t dev;       /**< Device number. */
		unsigned blknum; /**< Block number.  */
	} index;

	char data[BLOCK_SIZE]; /**< Data. */
} cache[CACHE_SIZE];


/**
 * Searches for a memory block in the cache.
 *
 * @param dev    Device number of the target ylock.
 * @param blknum Number of the target memory block..
 *
 * @returns If the target memory block is found, the memory block is locked and
 * its cache index number is returned.  Otherwise, -1 is returned instead.
 */
static int getblk(dev_t dev, int blknum)
{
	/* Search memory block. */
	for (int i = 0; i < CACHE_SIZE; i++)
	{
		/* Skip invalid or locked memory blocks. */
		if ((!cache[i].valid) || (cache[i].locked))
			continue;

		/* Found. */
		if ((cache[i].index.dev == dev) && (cache[i].index.blknum == blknum))
		{
			cache[i].locked = 1;
			return (i);
		}
	}

	return (-1);
}

/**
 * Chooses a memory block to be evicted from the cache.
 *
 * @returns The cache index number of the chosen memory block. The memory block
 * is locked. If no memory block can be evicted from the cache, -1 is returned
 * instead.
 */
static int evict(void)
{
	int j;

	j = -1;

	/* Search for a victim. */
	for (int i = 0; i < CACHE_SIZE; i++)
	{
		/* Skip locked memory blocks. */
		if (cache[i].locked)
			continue;


		/* Free or clean memory block. */
		if ((!cache[i].valid) || (!cache[i].dirty))
		{
			j = i;
			break;
		}

		/* Dirty memory block. */
		j = i;
	}

	/* Lock memory block. */
	if (j >= 0)
		cache[j].locked = 1;

	return (j);
}

/**
 * Writes a memory block back to the memory bank.
 *
 * @param i Cache index of the target memory block.
 *
 * @returns Upon successful completion zero is returned. Otherwise, a negative
 * error code is returned instead.
 */
static int writeback(int i)
{
	dev_t dev;           /**< Memory bank number.  */
	int server;          /**< Memory bank server.  */
	int blknum;          /**< Memory block number. */
	char *data;          /** Memory block data.    */
	struct bdev_msg msg; /**< Message.             */

	dev = cache[i].index.dev;
	blknum = cache[i].index.blknum;
	data = cache[i].data;

	kdebug("[bdev] connecting to device server (%d)", dev);
	if ((server = nanvix_ipc_connect(bdevsw[dev], 0)) < 0)
	{
		kdebug("[bdev] failed to connect to device server");
		goto error0;
	}

	/* Build message. */
	msg.type = BDEV_MSG_WRITEBLK_REQUEST;
	msg.content.writeblk_req.dev = dev;
	msg.content.writeblk_req.blknum = blknum;
	kmemcpy(msg.content.writeblk_req.data, data, BLOCK_SIZE);;

	kdebug("[bdev] forwarding request to device server");
	if (nanvix_ipc_send(server, &msg, sizeof(struct bdev_msg)) < 0)
		goto error1;

	kdebug("[bdev] waiting for device response");
	if (nanvix_ipc_receive(server, &msg, sizeof(struct bdev_msg)) < 0)
		goto error1;

	if (msg.type == BDEV_MSG_ERROR)
		goto error1;

	nanvix_ipc_close(server);

	cache[i].dirty = 0;

	return (0);

error1:
		kdebug("[bdev] communication failed with device server");
		nanvix_ipc_close(server);
error0:
	return (-EAGAIN);
}

/**
 * Loads a memory block from a memory bank.
 *
 * @param i   Cache index number.
 * @param dev Target memory bank number.
 * @param dev Number of target memory block.
 *
 * @returns Upon successful completion zero is returned. Otherwise, a negative
 * error code is returned instead.
 */
static int loadblk(int i, dev_t dev, int blknum)
{
	int server;          /**< Memory bank server.  */
	char *data;          /** Memory block data.    */
	struct bdev_msg msg; /**< Message.             */

	kdebug("[bdev] connecting to device server (%d)", dev);
	if ((server = nanvix_ipc_connect(bdevsw[dev], 0)) < 0)
	{
		kdebug("[bdev] failed to connect to device server");
		goto error0;
	}

	/* Build message. */
	msg.type = BDEV_MSG_READBLK_REQUEST;
	msg.content.readblk_req.dev = dev;
	msg.content.readblk_req.blknum = blknum;

	kdebug("[bdev] forwarding request to device server");
	if (nanvix_ipc_send(server, &msg, sizeof(struct bdev_msg)) < 0)
		goto error1;

	kdebug("[bdev] waiting for device response");
	if (nanvix_ipc_receive(server, &msg, sizeof(struct bdev_msg)) < 0)
		goto error1;

	if (msg.type == BDEV_MSG_ERROR)
		goto error1;

	nanvix_ipc_close(server);

	cache[i].valid = 1;
	cache[i].dirty = 0;
	cache[i].index.dev = dev;
	cache[i].index.blknum = blknum;
	kmemcpy(cache[i].data, msg.content.readblk_rep.data, BLOCK_SIZE);

	return (0);

error1:
		nanvix_ipc_close(server);
error0:
	return (-EAGAIN);
}

/**
 * @brief Maximum number of operations to enqueue.
 */
#define NR_CONNECTIONS 16

omp_lock_t lock;

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
		reply.content.error_rep.code = -EINVAL;
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
		reply.content.error_rep.code = -EINVAL;
		goto out;
	}

	/* Inlocked device. */
	if ((dev >= NR_BLKDEV) || (bdevsw[dev] == NULL))
	{
		kpanic("[bdev] reading block from inlocked device (%d)", dev);
		goto out;
	}
	
	do
	{
		omp_set_lock(&lock);
			block = getblk(dev, blknum);

			if (block < 0)
				block = evict();
		omp_unset_lock(&lock);
	} while (block < 0);

	/* Load memory block. */
	if ((cache[block].index.dev != dev) || (cache[block].index.blknum != blknum))
	{
		/* Write memory block back to remote memory. */
		if (cache[block].dirty)
		{
			if (writeback(block) < 0)
			{
				reply.type = BDEV_MSG_ERROR;
				reply.content.error_rep.code = -EAGAIN;
				goto out;
			}
		}

		loadblk(block, dev, blknum);
	}

	if (request.type == BDEV_MSG_READBLK_REQUEST)
	{
		reply.type = BDEV_MSG_READBLK_REPLY;
		kmemcpy(reply.content.readblk_rep.data, cache[block].data, BLOCK_SIZE);
		reply.content.readblk_rep.n = BLOCK_SIZE;
	}
	
	else if (request.type == BDEV_MSG_WRITEBLK_REQUEST)
	{
		reply.type = BDEV_MSG_WRITEBLK_REPLY;
		cache[block].dirty = 1;
		kmemcpy(cache[block].data,reply.content.writeblk_req.data, BLOCK_SIZE);
		reply.content.writeblk_rep.n = BLOCK_SIZE;
	}

	omp_set_lock(&lock);
	cache[block].locked = 0;
	omp_unset_lock(&lock);

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
	int channel;

	/* Inlocked number of arguments. */
	if (argc != 2)
	{
		kprintf("invalid number of arguments");
		kprintf("Usage: bdev <pathname>");
		return (NANVIX_FAILURE);
	}

	omp_init_lock(&lock);

	for (int i = 0; i < CACHE_SIZE; i++)
	{
		cache[i].locked = 0;
		cache[i].valid = 0;
	}

	channel = nanvix_ipc_create(argv[1], NR_CONNECTIONS, 0);

	kdebug("[bdev] server running");

	#pragma omp parallel private (client)
	{
		while (1)
		{
			int client = nanvix_ipc_open(channel);

			bdev(client);
		}
	}

	nanvix_ipc_close(channel);

	return (NANVIX_SUCCESS);
}

