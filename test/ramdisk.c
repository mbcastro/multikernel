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

#include <nanvix/klib.h>
#include <nanvix/ipc.h>
#include <nanvix/dev.h>
#include <nanvix/vfs.h>
#include <nanvix/ramdisk.h>

/*============================================================================*
 * Operations on Uniform Block Device Interface
 *============================================================================*/

/**
 * @brief Maximum number of operations to enqueue.
 */
#define OPERATIONS_MAX 16

/**
 * @brief Block device operation.
 */
struct operation
{
	int write;            /**< Write operation? */
	dev_t dev;            /**< Device number.   */
	char buf[BLOCK_SIZE]; /**< Buffer.          */
	unsigned blknum;      /**< Block number.    */
};

/**
 * @brief Pending operations.
 */
struct struct operation pending[OPERATIONS_MAX];

/**
 * @brief Number of pending operations.
 */
static npending = 0;

/**
 * @brief Enqueues a block operation.
 *
 * @param dev    Device number.
 * @param buf    Buffer.
 * @param blknum Blocknumber.
 * @param write  Is it a write operation?
 */
static void operation_enqueue(dev_t dev, char *buf, unsigned blknum, int write)
{
}

/*============================================================================*
 * Uniform Block Device Interface
 *============================================================================*/

/* Number of block devices. */
#define NR_BLKDEV 2

/*
 * Block devices table.
 */
static const char *bdevsw[NR_BLKDEV] = {
	RAMDISK_NAME, /* /dev/ramdisk */
	NULL          /* /dev/hdd     */
};

static void bdev_readblk_done(struct ramdisk_message *request, struct ramdisk_message *reply)
{

	/* Parse reply. */
	switch (reply->type)
	{
		case RAMDISK_MSG_READ_REPLY:
			kmemcpy(buf, reply.content.read_rep.data, BLOCK_SIZE);
			break;

		default:
			kpanic("failed to read block from device");
			break;
	}
}

/**
 * @brief Reads a block from a block device.
 *
 * @param dev    Target device.
 * @param buf    Buffer.
 * @param blknum Block number.
 */
static void bdev_readblk(dev_t dev, char *buf, unsigned blknum)
{
	int channel;                    /* IPC channel. */
	struct ramdisk_message request; /* Request.     */
	struct ramdisk_message reply;   /* Reply.       */
	
	/* Invalid device. */
	if (bdevsw[MAJOR(dev)] == NULL)
		kpanic("reading block from invalid device");

	/* Build request. */
	pending[i].request.type = RAMDISK_MSG_READ_REQUEST;
	pending[i].request.content.read_req.minor = MINOR(dev);
	pending[i].request.content.read_req.blknum = blknum;
	pending[i].callback = bdev_readblk_done;

	channel = nanvix_ipc_connect(RAMDISK_NAME);

	nanvix_ipc_send(channel, &request, sizeof(struct ramdisk_message));

	nanvix_ipc_receive(channel, &reply, sizeof(struct ramdisk_message));

	nanvix_ipc_close(channel);
}

/**
 * @brief Writes a block from a block device.
 *
 * @param dev    Target device.
 * @param buf    Buffer.
 * @param blknum Block number.
 */
static void bdev_writeblk(dev_t dev, const char *buf, unsigned blknum)
{
	int channel;                    /* IPC channel. */
	struct ramdisk_message request; /* Request.     */
	struct ramdisk_message reply;   /* Reply.       */
	
	/* Invalid device. */
	if (bdevsw[MAJOR(dev)] == NULL)
		kpanic("reading block from invalid device");

	/* Build request. */
	request.type = RAMDISK_MSG_WRITE_REQUEST;
	request.content.write_req.minor = MINOR(dev);
	request.content.write_req.blknum = blknum;
	kmemcpy(request.content.write_req.data, buf, BLOCK_SIZE);

	channel = nanvix_ipc_connect(RAMDISK_NAME);

	nanvix_ipc_send(channel, &request, sizeof(struct ramdisk_message));

	nanvix_ipc_receive(channel, &reply, sizeof(struct ramdisk_message));

	nanvix_ipc_close(channel);

	/* Parse reply. */
	switch (reply.type)
	{
		case RAMDISK_MSG_WRITE_REPLY:
			break;

		default:
			kpanic("failed to write a block to device");
			break;
	}
}

void bdev_open(int i)
{
	int ret;
	
	ret = nanvix_ipc_open(channel);

	if (ret < 0)
		return;

	operations[i].status = BDEV_RECEIVE;
	operations[i].channel = ret;
}

static void bdev_readblk(struct ramdisk_message *request)
{
	dev_t dev;

	dev = request->read_req.dev;
	
	/* Invalid device. */
	if (bdevsw[MAJOR(dev)] == NULL)
		kpanic("reading block from invalid device");
}

static void bdev_writeblk(struct ramdisk_message *request)
{
	dev_t dev;

	dev = request->write_req.dev;
	
	/* Invalid device. */
	if (bdevsw[MAJOR(dev)] == NULL)
		kpanic("write block from invalid device");

	/* Build request. */
	request.type = RAMDISK_MSG_WRITE_REQUEST;
	request.content.write_req.minor = MINOR(dev);
	request.content.write_req.blknum = blknum;
	kmemcpy(request.content.write_req.data, buf, BLOCK_SIZE);

	channel = nanvix_ipc_connect(RAMDISK_NAME);

	nanvix_ipc_send(channel, &request, sizeof(struct ramdisk_message));

	nanvix_ipc_receive(channel, &reply, sizeof(struct ramdisk_message));

	nanvix_ipc_close(channel);

	/* Parse reply. */
	switch (reply.type)
	{
		case RAMDISK_MSG_WRITE_REPLY:
			break;

		default:
			kpanic("failed to write a block to device");
			break;
	}
}

void bdev_receive(int i)
{
	int ret;
	int channel;
	struct bdev_message *request;

	channel = operations[i].channel;
	request = &operations[i].request;

	ret = nanvix_ipc_receive(channel, request, sizeof(struct ramdisk_message));

	if (ret < 0)
		return;

	switch (request->type)
	{
		case BDEV_MSG_READ_REQUEST:
			bdev_readblk(request);
			pending[i].status = BDEV_READBLK_CONNECT;
			break;

		case BDE_MSG_WRITE_REQUEST:
			break;

		default:
	}
}

void bdev()
{
	int channel;

	((void) argc);
	((void) argv);

	channel = nanvix_ipc_create(RAMDISK_NAME);

	while (1)
	{
		int i;

		i = next_operation();

		switch (operations[i].type)
		{
			case BDEV_OPEN:
				break;

			case BDEV_RECEIVE;
				break;

			case BDEV_READBLK_CONNECT:
				break;

			case BDEV_READBLK_SEND:
				break;

			case BDEV_READBLK_RECEIVE:
				break;

			case BDEV_WRITEBLK_CONNECT:
				break;

			case BDEV_WRITEBLK_SEND:
				break;

			case BDEV_WRITEBLK_RECEIVE:
				break;

			case BDEV_REPLY:
				break;
		}


		if (pending_operations == 1)
			sleep(x);
	}
}


/**
 * RAM Disk device driver unit test.
 */
int main(int argc, char **argv)
{
	char buf[BLOCK_SIZE];

	((void) argc);
	((void) argv);

	for (int i = 0; i < 2; i++)
	{
		/* Fill buffer, */
		for (int i = 0; i < BLOCK_SIZE; i++)
			buf[i] = i%sizeof(char);

		kprintf("writing data");
		bdev_writeblk(RAMDISK_MAJOR, buf, 0);

		while (1)
		{
			dispatch_operations();

			if (i%2)
				bdev_writeblk(RAMDISK_MAJOR, buf, 0);
			else
				bdev_readblk(RAMDISK_MAJOR, buf, 0);

			complete_operations();
		}
		

		kprintf("reading data");
		bdev_readblk(RAMDISK_MAJOR, buf, 0);

		/* Sanity check, */
		for (int i = 0; i < BLOCK_SIZE; i++)
		{
			if (buf[i] == i%sizeof(char))
				kdebug("I/O failed");
		}
	}

	return (NANVIX_SUCCESS);
}


