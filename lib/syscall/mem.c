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

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include <nanvix/dev.h>
#include <nanvix/ramdisk.h>
#include <nanvix/klib.h>
#include <nanvix/ipc.h>

/**
 * @brief Remote memory address.
 */
struct memaddr
{
	dev_t dev;       /**< Device ID.    */
	unsigned blknum; /**< Block number. */
};

/**
 * @brief Maps a linear address into a remote memory address.
 *
 * @param addr Linear address.
 *
 * @returns Remote memory address.
 */
static struct memaddr memmap(unsigned addr)
{
	struct memaddr memaddr;

	memaddr.dev = (addr)/(RAMDISK_SIZE/BLOCK_SIZE);
	memaddr.blknum = (addr)%(RAMDISK_SIZE/BLOCK_SIZE);

	kprintf("dev = %d %d", addr, memaddr.dev);

	return (memaddr);
}

/**
 * @brief Writes to remote memory.
 *
 * @param src  Pointer to source memory area.
 * @param dest Target memory area.
 * @param size Write size.
 *
 * @returns Upon successful completion, zero is returned;
 * otherwise a negative error code is returned instead.
 */
int memwrite(const void *src, unsigned dest, size_t size)
{
	const char *p;

	p = src;
	
	for (size_t i = 0; i < size; i += BLOCK_SIZE)
	{
		size_t n;
		int channel;
		struct memaddr memaddr;
		struct rmem_msg_header header;
		struct rmem_msg_payload payload;

		channel = nanvix_ipc_connect(BDEV_NAME);

		memaddr = memmap(dest + i);

		n = ((size - i) < BLOCK_SIZE) ? size - i : BLOCK_SIZE;

		/* Send header. */
		header.opcode = RMEM_MSG_WRITEBLK_REQUEST;
		header.param.rw.dev = memaddr.dev;
		header.param.rw.blknum = memaddr.blknum;
		nanvix_ipc_send(channel, &header, sizeof(struct rmem_msg_header));

		/* Send payload. */
		kmemcpy(&payload.data, p, n);
		nanvix_ipc_send(channel, &payload, sizeof(struct rmem_msg_payload));

		/* Parse reply. */
		nanvix_ipc_receive(channel, &header, sizeof(struct rmem_msg_header));
		if (header.opcode == RMEM_MSG_ERROR)
		{
			kdebug("memwrite error %d", header.param.err.num);

			nanvix_ipc_close(channel);
			return (NANVIX_FAILURE);
		}

		p += n;

		nanvix_ipc_close(channel);
	}

	return (0);
}

/**
 * @brief Reads from remote memory.
 *
 * @param src  Source memory area.
 * @param dest Pointer to target memory area.
 * @param size Read size.
 *
 * @returns Upon successful completion, zero is returned;
 * otherwise a negative error code is returned instead.
 */
int memread(void *dest, unsigned src, size_t size)
{
	char *p;

	p = dest;
	
	for (size_t i = 0; i < size; i += BLOCK_SIZE)
	{
		size_t n;
		int channel;
		struct memaddr memaddr;
		struct rmem_msg_header header;
		struct rmem_msg_payload payload;

		channel = nanvix_ipc_connect(BDEV_NAME);

		memaddr = memmap(src + i);

		n = ((size - i) < BLOCK_SIZE) ? size - i : BLOCK_SIZE;

		/* Send header. */
		header.opcode = RMEM_MSG_READBLK_REQUEST;
		header.param.rw.dev = memaddr.dev;
		header.param.rw.blknum = memaddr.blknum;
		nanvix_ipc_send(channel, &header, sizeof(struct rmem_msg_header));

		/* Parse reply. */
		nanvix_ipc_receive(channel, &header, sizeof(struct rmem_msg_header));
		if (header.opcode == RMEM_MSG_ERROR)
		{
			kdebug("memread error %d", header.param.err.num);

			nanvix_ipc_close(channel);
			return (NANVIX_FAILURE);
		}

		/* Receive payload. */
		nanvix_ipc_receive(channel, &payload, sizeof(struct rmem_msg_payload));
		kmemcpy(p, payload.data, n);
		
		p += n;

		nanvix_ipc_close(channel);
	}

	return (0);
}
