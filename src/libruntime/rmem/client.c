/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define __RMEM_SERVICE

#include <nanvix/servers/rmem.h>
#include <nanvix/servers/spawn.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/runtime/mailbox.h>
#include <nanvix/runtime/portal.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/portal.h>
#include <nanvix/sys/mutex.h>
#include <nanvix/sys/noc.h>
#include <ulibc/assert.h>
#include <ulibc/string.h>
#include <ulibc/stdio.h>
#include <posix/errno.h>
#include <posix/stdbool.h>

/**
 * @brief Remote memory server connection.
 */
static struct
{
	int initialized; /**< Is the connection initialized? */
	int outbox;      /**< Output mailbox for requests.   */
	int outportal;   /**< Output portal for data.        */
} server = {
	0, -1, -1,
};

/*============================================================================*
 * nanvix_rmem_alloc()                                                        *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
rpage_t nanvix_rmem_alloc(void)
{
	struct rmem_message msg;

	/* Build operation header. */
	msg.header.source = knode_get_num();
	msg.header.opcode = RMEM_ALLOC;

	/* Send operation header. */
	nanvix_assert(
		nanvix_mailbox_write(
			server.outbox,
			&msg, sizeof(struct rmem_message)
		) == 0
	);

	/* Receive reply. */
	nanvix_assert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct rmem_message)
		) == sizeof(struct rmem_message)
	);

	return (msg.blknum);
}

/*============================================================================*
 * nanvix_rmem_free()                                                         *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int nanvix_rmem_free(rpage_t blknum)
{
	struct rmem_message msg;

	/* Invalid block number. */
	if ((blknum == RMEM_NULL) || (blknum >= RMEM_NUM_BLOCKS))
		return (-EINVAL);

	/* Build operation header. */
	msg.header.source = knode_get_num();
	msg.header.opcode = RMEM_MEMFREE;
	msg.blknum = blknum;

	/* Send operation header. */
	nanvix_assert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct rmem_message)
		) == 0
	);

	/* Receive reply. */
	nanvix_assert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct rmem_message)
		) == sizeof(struct rmem_message)
	);

	return (msg.errcode);
}

/*============================================================================*
 * nanvix_rmem_read()                                                         *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
size_t nanvix_rmem_read(rpage_t blknum, void *buf)
{
	struct rmem_message msg;

	/* Invalid block number. */
	if ((blknum == RMEM_NULL) || (blknum >= RMEM_NUM_BLOCKS))
		return (0);

	/* Invalid buffer. */
	if (buf == NULL)
		return (0);

	/* Build operation header. */
	msg.header.source = knode_get_num();
	msg.header.opcode = RMEM_READ;

	msg.blknum = blknum;

	/* Send operation header. */
	nanvix_assert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct rmem_message)
		) == 0
	);

	/* Receive data. */
	nanvix_assert(
		kportal_allow(
			stdinportal_get(),
			RMEM_SERVER_NODE
		) == 0
	);
	nanvix_assert(
		kportal_read(
			stdinportal_get(),
			buf,
			RMEM_BLOCK_SIZE
		) == RMEM_BLOCK_SIZE
	);

	/* Receive reply. */
	nanvix_assert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct rmem_message)
		) == sizeof(struct rmem_message)
	);

	return ((msg.errcode < 0) ? 0 : RMEM_BLOCK_SIZE);
}

/*============================================================================*
 * nanvix_rmem_write()                                                        *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
size_t nanvix_rmem_write(rpage_t blknum, const void *buf)
{
	struct rmem_message msg;

	/* Invalid block number. */
	if ((blknum == RMEM_NULL) || (blknum >= RMEM_NUM_BLOCKS))
		return (0);

	/* Invalid buffer. */
	if (buf == NULL)
		return (0);

	/* Build operation header. */
	msg.header.source = knode_get_num();
	msg.header.opcode = RMEM_WRITE;
	msg.blknum = blknum;

	/* Send operation header. */
	nanvix_assert(
		nanvix_mailbox_write(
			server.outbox,
			&msg, sizeof(struct rmem_message)
		) == 0
	);

	/* Send data. */
	nanvix_assert(
		nanvix_portal_write(
			server.outportal,
			buf,
			RMEM_BLOCK_SIZE
		) == RMEM_BLOCK_SIZE
	);

	/* Receive reply. */
	nanvix_assert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct rmem_message)
		) == sizeof(struct rmem_message)
	);

	return ((msg.errcode < 0) ? 0 : RMEM_BLOCK_SIZE);
}

/*============================================================================*
 * nanvix_rmem_setup()                                                        *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_rmem_setup(void)
{
	/* Nothing to do.  */
	if (server.initialized)
		return (0);

	/* Open output mailbox */
	if ((server.outbox = nanvix_mailbox_open("/rmem")) < 0)
	{
		nanvix_printf("[nanvix][rmem] cannot open outbox to server\n");
		return (server.outbox);
	}

	/* Open underlying IPC connectors. */
	if ((server.outportal = nanvix_portal_open("/rmem")) < 0)
	{
		nanvix_printf("[nanvix][rmem] cannot open outportal to server\n");
		return (server.outportal);
	}

	server.initialized = 1;

	return (0);
}

/*============================================================================*
 * nanvix_rmem_cleanup()                                                      *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_rmem_cleanup(void)
{
	/* Nothing to do.  */
	if (!server.initialized)
		return (0);

	/* Close output mailbox */
	if (nanvix_mailbox_close(server.outbox) < 0)
	{
		nanvix_printf("[nanvix][rmem] cannot close outbox to server\n");
		return (-EAGAIN);
	}

	/* Close underlying IPC connectors. */
	if (nanvix_portal_close(server.outportal) < 0)
	{
		nanvix_printf("[nanvix][rmem] cannot close outportal to server\n");
		return (-EAGAIN);
	}

	server.initialized = 0;

	return (0);
}
