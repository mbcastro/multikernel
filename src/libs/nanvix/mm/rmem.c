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
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <nanvix/const.h>
#include <nanvix/syscalls.h>
#include <nanvix/pm.h>
#include <nanvix/mm.h>

/**
 * @brief Remote memory server connection.
 */
struct
{
	int initialized; /**< Is the connection initialized? */
	int outbox;      /**< Output mailbox for requests.   */
	int outportal;   /**< Outpt portal for data.         */
} server;

/*============================================================================*
 * memread()                                                                  *
 *============================================================================*/

/**
 * @brief Reads data from the remote memory.
 *
 * @param addr Remote address.
 * @param bug  Location where the data should be written to.
 * @param n    Number of bytes to read.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int memread(uint64_t addr, void *buf, size_t n)
{
	struct rmem_message msg;

	/* Build operation header. */
	msg.source = sys_get_node_num();
	msg.op = RMEM_READ;
	msg.blknum = addr;
	msg.size = n;

	/* Send operation header. */
	assert(mailbox_write(server.outbox, &msg, MAILBOX_MSG_SIZE) == 0);

	/* Send data. */
	assert(sys_portal_allow(get_inportal(), RMEM_SERVER_NODE) == 0);
	assert(sys_portal_read(get_inportal(), buf, n) == (int) n);

	return (0);
}

/*============================================================================*
 * memwrite()                                                                 *
 *============================================================================*/

/**
 * @brief Writes data to the remote memory.
 *
 * @param addr Remote address.
 * @param bug  Location where the data should be read from.
 * @param n    Number of bytes to write.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int memwrite(uint64_t addr, const void *buf, size_t n)
{
	struct rmem_message msg;
	
	/* Invalid write size. */
	if ((addr >= RMEM_SIZE) || ((addr + n) > RMEM_SIZE))
		return (-EINVAL);

	/* Build operation header. */
	msg.source = sys_get_node_num();
	msg.op = RMEM_WRITE;
	msg.blknum = addr;
	msg.size = n;

	/* Send operation header. */
	assert(mailbox_write(server.outbox, &msg, MAILBOX_MSG_SIZE) == 0);

	/* Send data. */
	assert(portal_write(server.outportal, buf, n) == (int) n);

	return (0);
}

/*============================================================================*
 * meminit()                                                                  *
 *============================================================================*/

/**
 * @brief Initializes the RMA engine.
 */
int meminit(void)
{
	/* Nothing to do.  */
	if (server.initialized)
		return (0);

	/* Open output mailbox */
	if ((server.outbox = mailbox_open("/rmem")) < 0)
	{
		printf("[nanvix][rmem] cannot open outbox to server\n");
		return (server.outbox);
	}

	/* Open underlying IPC connectors. */
	if ((server.outportal = portal_open("/rmem")) < 0)
	{
		printf("[nanvix][rmem] cannot open outportal to server\n");
		return (server.outportal);
	}

	server.initialized = 1;

	return (0);
}
