/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <nanvix/spawner.h>
#include <nanvix/klib.h>
#include <nanvix/syscalls.h>
#include <nanvix/mm.h>
#include <nanvix/name.h>
#include <nanvix/pm.h>

#ifdef DEBUG_RMEM
	#define rmem_debug(fmt, ...) debug("rmem", fmt, __VA_ARGS__)
#else
	#define rmem_debug(fmt, ...) { }
#endif

/**
 * @brief Node number.
 */
static int nodenum;

/**
 * @brief Input mailbox for small messages.
 */
static int inbox;

/**
 * @brief Input portal for receiving data.
 */
static int inportal;

/**
 * @brief Remote memory.
 */
static char rmem[RMEM_SIZE];

/*============================================================================*
 * rmem_write()                                                               *
 *============================================================================*/

/**
 * @brief Handles a write request.
 *
 * @param inportal Input portal for data transfer.
 * @param remote   Remote client.
 * @param blknum   RMEM block.
 * @param size     Number of bytes to write.
 */
static inline void rmem_write(int remote, uint64_t blknum, int size)
{
	rmem_debug("write nodenum=%d blknum=%d size=%d",
		remote,
		blknum,
		size
	);

	/* Invalid write. */
	if ((blknum >= RMEM_SIZE) || (blknum + size > RMEM_SIZE))
	{
		printf("[nanvix][rmem] invalid write\n");
		return;
	}

	/* Invalid write size. */
	if (size > RMEM_BLOCK_SIZE)
	{
		printf("[nanvix][rmem] invalid write size\n");
		return;
	}

	sys_portal_allow(inportal, remote);
	sys_portal_read(inportal, &rmem[blknum], size);
}

/*============================================================================*
 * rmem_read()                                                                *
 *============================================================================*/

/**
 * @brief Handles a read request.
 *
 * @param remote Remote client.
 * @param blknum RMEM block.
 * @param size   Number of bytes to write.
 */
static inline void rmem_read(int remote, uint64_t blknum, int size)
{
	int outportal;

	rmem_debug("read nodenum=%d blknum=%d size=%d",
		remote,
		blknum,
		size
	);

	/* Invalid read. */
	if ((blknum >= RMEM_SIZE) || (blknum + size > RMEM_SIZE))
	{
		printf("[nanvix][rmem] invalid read\n");
		return;
	}

	/* Invalid read size. */
	if (size > RMEM_BLOCK_SIZE)
	{
		printf("[nanvix][rmem] invalid read size\n");
		return;
	}

	outportal = sys_portal_open(remote);
	sys_portal_write(outportal, &rmem[blknum], size);
	sys_portal_close(outportal);
}

/*============================================================================*
 * rmem_loop()                                                                *
 *============================================================================*/

/**
 * @brief Handles remote memory requests.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int rmem_loop(void)
{
	int shutdown = 0;

	while(!shutdown)
	{
		struct rmem_message msg;

		sys_mailbox_read(inbox, &msg, MAILBOX_MSG_SIZE);

		/* handle write operation. */
		switch (msg.header.opcode)
		{
			/* Write to RMEM. */
			case RMEM_WRITE:
				rmem_write(msg.header.source, msg.blknum, msg.size);
				break;

			/* Read from RMEM. */
			case RMEM_READ:
				rmem_read(msg.header.source, msg.blknum, msg.size);
				break;

			case RMEM_EXIT:
				shutdown = 1;
				break;

			/* Should not happen. */
			default:
				break;
		}
	}

	return (0);
}

/*============================================================================*
 * rmem_startup()                                                             *
 *============================================================================*/

/**
 * @brief Initializes the remote memory server.
 *
 * @param _inbox    Input mailbox.
 * @param _inportal Input portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int rmem_startup(int _inbox, int _inportal)
{
	int ret;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = sys_get_node_num();

	/* Assign input mailbox. */
	inbox = _inbox;

	/* Assign input portal. */
	inportal = _inportal;

	/* Link name. */
	sprintf(pathname, "/rmem");
	if ((ret = name_link(nodenum, pathname)) < 0)
		return (ret);

	return (0);
}

/*============================================================================*
 * rmem_shutdown()                                                            *
 *============================================================================*/

/**
 * @brief Shutdowns the remote memory server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int rmem_shutdown(void)
{
	return (0);
}

/*============================================================================*
 * rmem_server()                                                              *
 *============================================================================*/

/**
 * @brief Remote memory server.
 *
 * @param _inbox    Input mailbox.
 * @param _inportal Input portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int rmem_server(int _inbox, int _inportal)
{
	int ret;

	printf("[nanvix][rmem] booting up server\n");

	if ((ret = rmem_startup(_inbox, _inportal)) < 0)
		goto error;

	spawner_ack();
	
	printf("[nanvix][rmem] server alive\n");

	if ((ret = rmem_loop()) < 0)
		goto error;

	printf("[nanvix][rmem] shutting down server\n");

	if ((ret = rmem_shutdown()) < 0)
		goto error;

	return (0);

error:
	return (ret);
}
