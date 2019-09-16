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

#include <nanvix/servers/rmem.h>
#include <nanvix/servers/spawn.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/runtime/mailbox.h>
#include <nanvix/runtime/portal.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/portal.h>
#include <nanvix/sys/mutex.h>
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
} server;

/*============================================================================*
 * nanvix_rmemalloc()                                                         *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int nanvix_rmemalloc(void)
{
	struct rmem_message msg;

	/* Build operation header. */
	msg.header.source = processor_node_get_num(core_get_id());
	msg.header.opcode = RMEM_MEMALLOC;

	/* Send operation header. */
	nanvix_assert(nanvix_mailbox_write(server.outbox, &msg, sizeof(struct rmem_message)) == 0);
	nanvix_assert(kmailbox_read(stdinbox_get(), &msg, sizeof(struct rmem_message)) == sizeof(struct rmem_message));

	return msg.blknum;
}

/*============================================================================*
 * nanvix_rmemfree()                                                          *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int nanvix_rmemfree(uint64_t blknum)
{
	struct rmem_message msg;

	/* Invalid read. */
	if ((blknum >= RMEM_SIZE/RMEM_BLOCK_SIZE))
		return (-EINVAL);

	/* Build operation header. */
	msg.header.source = processor_node_get_num(core_get_id());
	msg.header.opcode = RMEM_MEMFREE;
	msg.blknum = blknum;

	/* Send operation header. */
	nanvix_assert(nanvix_mailbox_write(server.outbox, &msg, sizeof(struct rmem_message)) == 0);

	return (0);
}

/*============================================================================*
 * nanvix_rmemread()                                                          *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int nanvix_rmemread(uint64_t addr, void *buf, size_t n)
{
	struct rmem_message msg;

	/* Invalid read. */
	if ((addr >= RMEM_SIZE) || ((addr + n) > RMEM_SIZE))
		return (-EINVAL);

	/* Null read. */
	if (buf == NULL)
		return (-EINVAL);

	/* Bad addr to read. */
	if ((addr % RMEM_BLOCK_SIZE) != 0)
	    return (-EFAULT);

	/* Bad size to read. */
	if ((n % RMEM_BLOCK_SIZE) != 0)
	    return (-EFAULT);

	/* Nothing to do. */
	if (n == 0)
		return (0);

	/* Build operation header. */
	msg.header.source = processor_node_get_num(core_get_id());
	msg.header.opcode = RMEM_READ;

	msg.blknum = addr;
	msg.size = n;

	/* Send operation header. */
	nanvix_assert(nanvix_mailbox_write(server.outbox, &msg, sizeof(struct rmem_message)) == 0);

	/* Recieve data. */
	nanvix_assert(kportal_allow(stdinportal_get(), RMEM_SERVER_NODE) == 0);
	nanvix_assert(kportal_read(stdinportal_get(), buf, n) == (int) n);

	return (0);
}

/*============================================================================*
 * nanvix_rmemwrite()                                                         *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int nanvix_rmemwrite(uint64_t addr, const void *buf, size_t n)
{
	struct rmem_message msg;

	/* Invalid write. */
	if ((addr >= RMEM_SIZE) || ((addr + n) > RMEM_SIZE))
		return (-EINVAL);

	/* Null write. */
	if (buf == NULL)
		return (-EINVAL);

	/* Bad addr to read. */
	if ((addr % RMEM_BLOCK_SIZE) != 0)
	    return (-EFAULT);

	/* Bad size to read. */
	if ((n % RMEM_BLOCK_SIZE) != 0)
	    return (-EFAULT);

	/* Nothing to do. */
	if (n == 0)
		return (0);

	/* Build operation header. */
	msg.header.source = processor_node_get_num(core_get_id());
	msg.header.opcode = RMEM_WRITE;
	msg.blknum = addr;
	msg.size = n;

	/* Send operation header. */
	nanvix_assert(nanvix_mailbox_write(server.outbox, &msg, sizeof(struct rmem_message)) == 0);

	/* Send data. */
	nanvix_assert(nanvix_portal_write(server.outportal, buf, n) == (int) n);

	return (0);
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
	if (portal_close(server.outportal) < 0)
	{
		nanvix_printf("[nanvix][rmem] cannot close outportal to server\n");
		return (-EAGAIN);
	}

	server.initialized = 0;

	return (0);
}
