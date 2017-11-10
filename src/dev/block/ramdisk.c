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

#include <nanvix/klib.h>
#include <nanvix/ipc.h>
#include <nanvix/vfs.h>
#include <nanvix/dev.h>
#include <nanvix/ramdisk.h>

/**
 * @brief Maximum number of simultaneous connections.
 */
#define NR_CONNECTIONS 16

/**
 * @brief RAM Disk.
 */
static char *ramdisk;

/**
 * @brief Reads a block from a RAM Disk device.
 *
 * @param buf    Buffer.
 * @param blknum Block number.
 *
 * @return Upon successful completion zero is returned. Upon failure
 * a negative error code is returned instead.
 */
static ssize_t ramdisk_readblk(char *buf, unsigned blknum)
{	
	unsigned off;    /* Offset.    */
	const char *ptr; /* Readpoint. */

	/* Invalid offset. */
	if ((off = (blknum << BLOCK_SIZE_LOG2)) > RAMDISK_SIZE)
		return (-EFBIG);
	
	ptr = &ramdisk[off];
	
	kmemcpy(buf, ptr, BLOCK_SIZE);
	
	return (0);
}

/**
 * @brief Writes a block to a RAM disk device.
 *
 * @param buf    Buffer.
 * @param blknum Block number.
 *
 * @return Upon successful completion zero is returned, upon failure
 * non-zero is returned instead.
 */
static ssize_t ramdisk_writeblk(const char *buf, unsigned blknum)
{	
	char *ptr;    /* Readpoint. */
	unsigned off; /* Offset.    */

	/* Invalid offset. */
	if ((off = (blknum << BLOCK_SIZE_LOG2)) > RAMDISK_SIZE)
	{
		kpanic("%d", off);
		return (-EFBIG);
	}

	ptr = &ramdisk[off];
	
	kmemcpy(ptr, buf, BLOCK_SIZE);
	
	return (0);
}

/**
 * @brief handles a request.
 *
 * @param request Request.
 * @param reply   Reply.
 */
static void ramdisk_handle(struct bdev_msg *request, struct bdev_msg *reply)
{
	int ret;

	switch (request->type)
	{
		/* Write request. */
		case BDEV_MSG_WRITEBLK_REQUEST:
		{
			char *buf;
			unsigned blknum;

			/* Extract request parameters. */
			buf = request->content.writeblk_req.data;
			blknum = request->content.writeblk_req.blknum;

			kdebug("[ramdisk] write request (%d)", blknum);
			
			ret = ramdisk_writeblk(buf, blknum);

			/* Build reply. */
			reply->type = (ret) ? 
				BDEV_MSG_ERROR : BDEV_MSG_WRITEBLK_REPLY;
		} break;

		/* Read request. */
		case BDEV_MSG_READBLK_REQUEST:
		{
			char *buf;
			unsigned blknum;

			/* Extract request parameters. */
			buf = reply->content.readblk_rep.data;
			blknum = request->content.readblk_req.blknum;

			kdebug("[ramdisk] read request (%d)", blknum);
			
			ret = ramdisk_readblk(buf, blknum);

			/* Build reply. */
			reply->type = (ret) ? 
				BDEV_MSG_ERROR : BDEV_MSG_READBLK_REPLY;
		} break;

		default:
			kdebug("[ramdisk] bad request");
			reply->type = BDEV_MSG_ERROR;
			ret = -EINVAL;
			break;
	}

	/* Build reply. */
	switch (reply->type)
	{
		case BDEV_MSG_WRITEBLK_REQUEST:
		case BDEV_MSG_READBLK_REQUEST:
			reply->content.writeblk_rep.n = BLOCK_SIZE;
		break;

		default:
			reply->content.error_rep.code = -ret;
	}
}

/**
 * @brief RAM Disk device driver.
 */
int main(int argc, char **argv)
{
	int channel;

	/* Invalid number of arguments. */
	if (argc != 2)
	{
		kprintf("invalid number of arguments");
		kprintf("Usage: ramdisk <pathname>");
		return (NANVIX_FAILURE);
	}

	ramdisk = kmalloc(RAMDISK_SIZE);

	channel = nanvix_ipc_create(argv[1], NR_CONNECTIONS, 0);

	kdebug("[ramdisk] server running");

	while (1)
	{
		int client;
		struct bdev_msg reply;
		struct bdev_msg request;

		client = nanvix_ipc_open(channel);
		kdebug("[ramdisk] client connected");

		nanvix_ipc_receive(client, &request, sizeof(struct bdev_msg));
		kdebug("[ramdisk] serving client");

		ramdisk_handle(&request, &reply);

		nanvix_ipc_send(client, &reply, sizeof(struct bdev_msg));
		kdebug("[ramdisk] replying client");

		nanvix_ipc_close(client);
		kdebug("[ramdisk] client disconnected");
	}

	nanvix_ipc_unlink(channel);

	kfree(ramdisk);

	return (NANVIX_SUCCESS);
}

