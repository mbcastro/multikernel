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
 * @brief RAM Disk size (in bytes).
 */
#define RAMDISK_SIZE 4096

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
static int ramdisk_readblk(unsigned minor, char *buf, unsigned blknum)
{	
	char *ptr;
	
	ptr = &ramdisks[minor].data[blknum << BLOCK_SIZE_LOG2];
	
	memcpy(buf, ptr, BLOCK_SIZE);
	
	return (0);
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
static int ramdisk_writeblk(unsigned minor, const char *buf, unsigned blknum)
{	
	char *ptr;
	
	ptr = &ramdisks[minor].data[blknum << BLOCK_SIZE_LOG2];
	
	memcpy(ptr, buf, BLOCK_SIZE);
	
	return (0);
}

/**
 * RAM Disk device driver.
 */
int main(int argc, char **argv)
{
	int channel;
	char buf[128];

	((void) argc);
	((void) argv);

	channel = nanvix_ipc_create(RAMDISK_NAME);

	while (1)
	{
		nanvix_ipc_open(channel);

		nanvix_ipc_receive(channel, buf, sizeof(buf));

		fprintf(stderr, "%s\n", buf);

		nanvix_ipc_close(channel);
	}

	nanvix_ipc_unlink(channel);

	return (NANVIX_SUCCESS);
}

