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
static void ramdisk_handle(int client)
{
	char *buf;
	unsigned blknum;
	struct rmem_msg_header header;
	struct rmem_msg_payload payload;

	nanvix_ipc_receive(client, &header, sizeof(struct rmem_msg_header));

	switch (header.opcode)
	{
		/* Write request. */
		case RMEM_MSG_WRITEBLK_REQUEST:
		{
			int ret;

			nanvix_ipc_receive(client, &payload, sizeof(struct rmem_msg_payload));

			/* Extract request parameters. */
			blknum = header.param.rw.blknum;
			buf = payload.data;

			kdebug("[ramdisk] write request (%d)", blknum);
			
			ret = ramdisk_writeblk(buf, blknum);

			/* Build reply. */
			header.opcode = (ret) ? 
				RMEM_MSG_ERROR : RMEM_MSG_WRITEBLK_REPLY;
		} break;

		/* Read request. */
		case RMEM_MSG_READBLK_REQUEST:
		{
			int ret;

			/* Extract request parameters. */
			blknum = header.param.rw.blknum;
			buf = payload.data;

			kdebug("[ramdisk] read request (%d)", blknum);
			
			ret = ramdisk_readblk(buf, blknum);

			/* Build reply. */
			header.opcode = (ret) ? 
				RMEM_MSG_ERROR : RMEM_MSG_READBLK_REPLY;
		} break;

		default:
			kdebug("[ramdisk] bad request");
			header.opcode = RMEM_MSG_ERROR;
			header.param.err.num = -EINVAL;
			break;
	}

	kdebug("[ramdisk] replying client");

	/* Send reply. */
	nanvix_ipc_send(client, &header, sizeof(struct rmem_msg_header));
	if (header.opcode == RMEM_MSG_READBLK_REPLY)
		nanvix_ipc_send(client, &payload, sizeof(struct rmem_msg_payload));

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

	channel = nanvix_ipc_create(argv[1], NR_CONNECTIONS);

	kdebug("[ramdisk] server running");

	while (1)
	{
		int client;

		client = nanvix_ipc_open(channel);
		kdebug("[ramdisk] client connected");

		kdebug("[ramdisk] serving client");
		ramdisk_handle(client);

		nanvix_ipc_close(client);
		kdebug("[ramdisk] client disconnected");
	}

	nanvix_ipc_unlink(channel);

	kfree(ramdisk);

	return (NANVIX_SUCCESS);
}

