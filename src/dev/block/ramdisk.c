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
#include <nanvix/vfs.h>
#include <nanvix/dev.h>
#include <nanvix/ramdisk.h>

/**
 * @brief Number of RAM Disks.
 */
#define NR_BDEVS 1

/**
 * @brief RAM Disks.
 */
static char ramdisk[BLOCK_SIZE];

/**
 * @brief Reads a block from a RAM Disk device.
 *
 * @param minor  Minor device number.
 * @param buf    Buffer.
 * @param blknum Block number.
 *
 * @return Upon successful completion zero is returned, upon failure
 * non-zero is returned instead.
 */
static ssize_t ramdisk_readblk(unsigned minor, char *buf, unsigned blknum)
{	
	char *ptr;

	((void) minor);
	
	ptr = &ramdisk[blknum << BLOCK_SIZE_LOG2];
	
	memcpy(buf, ptr, BLOCK_SIZE);
	
	return (BLOCK_SIZE);
}

/**
 * @brief Writes a block to a RAM disk device.
 *
 * @param minor  Minor device number.
 * @param buf    Buffer.
 * @param blknum Block number.
 *
 * @return Upon successful completion zero is returned, upon failure
 * non-zero is returned instead.
 */
static ssize_t ramdisk_writeblk(unsigned minor, const char *buf, unsigned blknum)
{	
	char *ptr;

	((void) minor);	

	ptr = &ramdisk[blknum << BLOCK_SIZE_LOG2];
	
	memcpy(ptr, buf, BLOCK_SIZE);
	
	return (BLOCK_SIZE);
}

/**
 * @brief handles a request.
 *
 * @param request Request.
 * @param reply   Reply.
 */
static void ramdisk_handle(struct bdev_msg *request, struct bdev_msg *reply)
{
	switch (request->type)
	{
		/* Write request. */
		case BDEV_MSG_WRITEBLK_REQUEST:
		{
			ssize_t n;
			char *buf;
			unsigned minor;
			unsigned blknum;

			/* Extract request parameters. */
			minor = request->content.writeblk_req.dev;
			buf = request->content.writeblk_req.data;
			blknum = request->content.writeblk_req.blknum;

			kdebug("[ramdisk] write request %d %d", minor, blknum);
			
			n = ramdisk_writeblk(minor, buf, blknum);

			/* Build reply. */
			reply->type = BDEV_MSG_WRITEBLK_REPLY;
			reply->content.writeblk_rep.n = n;
		} break;

		/* Read request. */
		case BDEV_MSG_READBLK_REQUEST:
		{
			ssize_t n;
			char *buf;
			unsigned minor;
			unsigned blknum;

			kdebug("[ramdisk] read request");

			/* Extract request parameters. */
			minor = request->content.readblk_req.dev;
			buf = reply->content.readblk_rep.data;
			blknum = request->content.readblk_req.blknum;
			
			n = ramdisk_readblk(minor, buf, blknum);

			/* Build reply. */
			reply->type = BDEV_MSG_READBLK_REPLY;
			reply->content.readblk_rep.n = n;
		} break;

		default:
			kdebug("[ramdisk] bad request");
			reply->type = BDEV_MSG_ERROR;
			break;
	}
}

/**
 * @brief RAM Disk device driver.
 */
int main(int argc, char **argv)
{
	int channel;

	((void) argc);

	channel = nanvix_ipc_create(argv[1], 1, 0);

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

	return (NANVIX_SUCCESS);
}

