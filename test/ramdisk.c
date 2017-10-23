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

/* Number of block devices. */
#define NR_BLKDEV 2

/*
 * Block devices table.
 */
static const char *bdevsw[NR_BLKDEV] = {
	RAMDISK_NAME, /* /dev/ramdisk */
	NULL          /* /dev/hdd     */
};

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

	channel = nanvix_ipc_connect(RAMDISK_NAME);

	/* Build request. */
	request.type = RAMDISK_MSG_READ_REQUEST;
	request.content.read_req.minor = MINOR(dev);
	request.content.read_req.blknum = blknum;

	nanvix_ipc_send(channel, &request, sizeof(struct ramdisk_message));

	nanvix_ipc_receive(channel, &reply, sizeof(struct ramdisk_message));

	nanvix_ipc_close(channel);

	/* Parse reply. */
	switch (reply.type)
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

	channel = nanvix_ipc_connect(RAMDISK_NAME);

	/* Build request. */
	request.type = RAMDISK_MSG_WRITE_REQUEST;
	request.content.write_req.minor = MINOR(dev);
	request.content.write_req.blknum = blknum;
	kmemcpy(request.content.write_req.data, buf, BLOCK_SIZE);

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

/**
 * RAM Disk device driver unit test.
 */
int main(int argc, char **argv)
{
	char buf[BLOCK_SIZE];

	((void) argc);
	((void) argv);

	for (int i = 1; i < 2; i++)
	{
		/* Fill buffer, */
		for (int i = 0; i < BLOCK_SIZE; i++)
			buf[i] = i%sizeof(char);

		bdev_writeblk(RAMDISK_MAJOR, buf, 0);

		kprintf("sending data");

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


