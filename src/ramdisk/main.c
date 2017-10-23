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
#include <nanvix/ramdisk.h>

/**
 * @brief Number of RAM Disks.
 */
#define NR_RAMDISKS 4

/**
 * @brief RAM Disks.
 */
static struct
{
	char data[RAMDISK_SIZE]; /**< Data. */
} ramdisks[NR_RAMDISKS];

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
	
	ptr = &ramdisks[minor].data[blknum << BLOCK_SIZE_LOG2];
	
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
	
	ptr = &ramdisks[minor].data[blknum << BLOCK_SIZE_LOG2];
	
	memcpy(ptr, buf, BLOCK_SIZE);
	
	return (BLOCK_SIZE);
}

/**
 * @brief handles a request.
 *
 * @param request Request.
 * @param reply   Reply.
 */
static void ramdisk(struct ramdisk_message *request, struct ramdisk_message *reply)
{
	switch (request->type)
	{
		/* Write request. */
		case RAMDISK_MSG_WRITE_REQUEST:
		{
			ssize_t n;
			char *buf;
			unsigned minor;
			unsigned blknum;

			/* Extract request parameters. */
			minor = request->content.write_req.minor;
			buf = request->content.write_req.data;
			blknum = request->content.write_req.blknum;
			
			n = ramdisk_writeblk(minor, buf, blknum);

			/* Build reply. */
			reply->type = RAMDISK_MSG_WRITE_REPLY;
			reply->content.write_rep.n = n;
		} break;

		/* Read request. */
		case RAMDISK_MSG_READ_REQUEST:
		{
			ssize_t n;
			char *buf;
			unsigned minor;
			unsigned blknum;

			/* Extract request parameters. */
			minor = request->content.read_req.minor;
			buf = reply->content.read_rep.data;
			blknum = request->content.read_req.blknum;
			
			n = ramdisk_readblk(minor, buf, blknum);

			/* Build reply. */
			reply->type = RAMDISK_MSG_READ_REPLY;
			reply->content.read_rep.n = n;
		} break;

		default:
			reply->type = RAMDISK_MSG_ERROR;
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
	((void) argv);

	channel = nanvix_ipc_create(RAMDISK_NAME);

	while (1)
	{
		struct ramdisk_message reply;
		struct ramdisk_message request;

		nanvix_ipc_open(channel);

		nanvix_ipc_receive(channel, &request, sizeof(struct ramdisk_message));

		ramdisk(&request, &reply);

		nanvix_ipc_send(channel, &reply, sizeof(struct ramdisk_message));

		nanvix_ipc_close(channel);
	}

	nanvix_ipc_unlink(channel);

	return (NANVIX_SUCCESS);
}

