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
 * @brief Maximum number of operations to enqueue.
 */
#define NR_CONNECTIONS 16

/**
 * Memory block cache size (in blocks).
 */
#define CACHE_SIZE 256

/**
 * Memory block cache.
 */
struct
{
	/* Flags. */
	int valid;  /**< Is valid?      */
	int dirty;  /**< Dirty block?  */

	/* Index. */
	struct
	{
		dev_t dev;       /**< Device number. */
		unsigned blknum; /**< Block number.  */
	} index;

	char data[BLOCK_SIZE]; /**< Data. */
} cache[CACHE_SIZE];

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
	unsigned blknum;     /**< Memory block number. */
	char *data;          /** Memory block data.    */
	struct rmem_msg_header header;
	struct rmem_msg_payload payload;

	dev = cache[i].index.dev;
	blknum = cache[i].index.blknum;
	data = cache[i].data;

	kdebug("[bdev] connecting to device server (%d)", dev);
	if ((server = nanvix_ipc_connect(bdevsw[dev])) < 0)
	{
		kdebug("[bdev] failed to connect to device server");
		goto error0;
	}

	kdebug("[bdev] writing back block to memory bank");

	/* Send header. */
	header.opcode = RMEM_MSG_WRITEBLK_REQUEST;
	header.param.rw.dev = dev;
	header.param.rw.blknum = blknum;
	nanvix_ipc_send(server, &header, sizeof(struct rmem_msg_header));

	/* Send payload. */
	kmemcpy(&payload.data, data, BLOCK_SIZE);
	nanvix_ipc_send(server, &payload, sizeof(struct rmem_msg_payload));

	kdebug("[bdev] waiting for device response");

	/* Parse reply. */
	nanvix_ipc_receive(server, &header, sizeof(struct rmem_msg_header));
	if (header.opcode == RMEM_MSG_ERROR)
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
static int loadblk(int i, dev_t dev, unsigned blknum)
{
	int server;          /**< Memory bank server.  */
	struct rmem_msg_header header;
	struct rmem_msg_payload payload;

	kdebug("[bdev] connecting to device server (%d)", dev);
	if ((server = nanvix_ipc_connect(bdevsw[dev])) < 0)
	{
		kdebug("[bdev] failed to connect to device server");
		goto error0;
	}

	kdebug("[bdev] loading block from memory bank");

	/* Send header. */
	header.opcode = RMEM_MSG_READBLK_REQUEST;
	header.param.rw.dev = dev;
	header.param.rw.blknum = blknum;
	nanvix_ipc_send(server, &header, sizeof(struct rmem_msg_header));

	kdebug("[bdev] waiting for device response");

	/* Parse reply. */
	nanvix_ipc_receive(server, &header, sizeof(struct rmem_msg_header));
	if (header.opcode == RMEM_MSG_ERROR)
		goto error1;

	/* Receive payload. */
	nanvix_ipc_receive(server, &payload, sizeof(struct rmem_msg_payload));
	kmemcpy(cache[i].data, payload.data, BLOCK_SIZE);

	cache[i].valid = 1;
	cache[i].dirty = 0;
	cache[i].index.dev = dev;
	cache[i].index.blknum = blknum;
	kmemcpy(cache[i].data, payload.data, BLOCK_SIZE);

	nanvix_ipc_close(server);

	return (0);

error1:
	nanvix_ipc_close(server);
error0:
	kdebug("[bdev] communication failed with device server");
	return (-EAGAIN);
}

/**
 * Chooses a memory block to be evicted from the cache.
 *
 * @returns The cache index number of the chosen memory block. If no memory
 * block can be evicted from the cache, -1 is returned instead.
 */
static int evict(void)
{
	int j;

	/* Search for a victim. */
	for (int i = 0; i < CACHE_SIZE; i++)
	{
		/* Free or clean memory block. */
		if ((!cache[i].valid) || (!cache[i].dirty))
		{
			j = i;
			break;
		}

		/* Dirty memory block. */
		j = i;
	}

	/* Write memory j back to remote memory. */
	if ((cache[j].valid) && (cache[j].dirty))
	{
		if (writeback(j) < 0)
			return (-1);
	}

	return (j);
}

/**
 * Searches for a memory block in the cache.
 *
 * @param dev    Device number of the target ylock.
 * @param blknum Number of the target memory block..
 *
 * @returns If the target memory block is found, the memory block is locked and
 * its cache index number is returned.  Otherwise, -1 is returned instead.
 */
static int getblk(dev_t dev, unsigned blknum)
{
	int j;

	/* Search memory block. */
	for (int i = 0; i < CACHE_SIZE; i++)
	{
		/* Skip invalid or locked memory blocks. */
		if (!cache[i].valid)
			continue;

		/* Found. */
		if ((cache[i].index.dev == dev) && (cache[i].index.blknum == blknum))
		{
			kdebug("[bdev] cache hit %d %d", dev, blknum);
			return (i);
		}
	}

	kdebug("[bdev] cache miss %d %d", dev, blknum);

	 j = evict();

	/* Load memory j. */
	if ((!cache[j].valid) || (cache[j].index.dev != dev) || (cache[j].index.blknum != blknum))
	{
		if (loadblk(j, dev, blknum) < 0)
			return (-1);
	}

	return (j);
}

static void bdev(int channel)
{
	unsigned blknum;
	dev_t dev;               /* Device.             */
	int block;
	struct rmem_msg_header header;
	struct rmem_msg_payload payload;

	nanvix_ipc_receive(channel, &header, sizeof(struct rmem_msg_header));

	/* Read a block. */
	if ((header.opcode == RMEM_MSG_READBLK_REQUEST) || (header.opcode == RMEM_MSG_WRITEBLK_REQUEST))
	{
		dev = header.param.rw.dev;
		blknum = header.param.rw.blknum;
	}

	else
	{
		kdebug("[bdev] bad request");
		header.opcode = RMEM_MSG_ERROR;
		header.param.err.num = -EINVAL;
		goto error;
	}

	/* Inlocked device. */
	if ((dev >= NR_BLKDEV) || (bdevsw[dev] == NULL))
	{
		kdebug("[bdev] bad request");
		header.opcode = RMEM_MSG_ERROR;
		header.param.err.num = -EINVAL;
		goto error;
	}

	if ((block = getblk(dev, blknum)) < 0)
	{
		kdebug("[bdev] failed to replace blocks");
		header.opcode = RMEM_MSG_ERROR;
		header.param.err.num = -EAGAIN;
		goto error;
	}

	if (header.opcode == RMEM_MSG_READBLK_REQUEST)
	{
		kdebug("[bdev] serving read request");

		/* Serve request. */
		kmemcpy(payload.data, cache[block].data, BLOCK_SIZE);

		kdebug("[bdev] replying client");

		/* Send header. */
		header.opcode = RMEM_MSG_READBLK_REPLY;
		nanvix_ipc_send(channel, &header, sizeof(struct rmem_msg_header));

		/* Send payload*/
		nanvix_ipc_send(channel, &payload, sizeof(struct rmem_msg_payload));
	}
	
	else
	{
		nanvix_ipc_receive(channel, &payload, sizeof(struct rmem_msg_payload));

		kdebug("[bdev] serving write request");

		/* Serve request. */
		cache[block].dirty = 1;
		kmemcpy(cache[block].data, payload.data, BLOCK_SIZE);

		kdebug("[bdev] replying client");

		/* Send header. */
		header.opcode = RMEM_MSG_WRITEBLK_REPLY;
		nanvix_ipc_send(channel, &header, sizeof(struct rmem_msg_header));
	}

	return;

error:
	kdebug("[bdev] replying client");
	nanvix_ipc_send(channel, &header, sizeof(struct rmem_msg_header));
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

	/* Initialize cache. */
	for (int i = 0; i < CACHE_SIZE; i++)
		cache[i].valid = 0;

	kdebug("[bdev] starting server");
	channel = nanvix_ipc_create(argv[1], NR_CONNECTIONS);

	while (1)
	{
		kdebug("[bdev] accepting client");
		int client = nanvix_ipc_open(channel);

		bdev(client);


		kdebug("[bdev] disconnecting client");
		nanvix_ipc_close(client);
	}

	nanvix_ipc_close(channel);

	return (NANVIX_SUCCESS);
}

