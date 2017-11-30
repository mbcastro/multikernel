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

#include <assert.h>

#include <nanvix/klib.h>
#include <nanvix/ipc.h>

/**
 * @brief IPC server name.
 */
#define IPC_TEST_NAME "/tmp/ipc.test"

/**
 * @brief Number of messages to exchange.
 */
#define NR_MESSAGES 128

/**
 * @brief Message size (in bytes).
 */
#define MESSAGE_SIZE 4096

/**
 * @brief Unit test server.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static int server(void)
{
	int channel;            /* Communication channel. */
	char buf[MESSAGE_SIZE]; /* Buffer.                */

	channel = nanvix_ipc_create(IPC_TEST_NAME, 1);

	/* Receive messages. */
	for (int i = 0; i < NR_MESSAGES; i++)
	{
		int ack;
		int client;

		client = nanvix_ipc_open(channel);

		nanvix_ipc_receive(client, buf, sizeof(buf));

		/* Check message integrity. */
		for (int i = 0; i < MESSAGE_SIZE; i++)
		{
			if (buf[i] != (i%sizeof(char)))
				return (NANVIX_FAILURE);
		}

		/* Send ackowledge message. */
		ack = NANVIX_SUCCESS;
		nanvix_ipc_send(client, &ack, sizeof(ack));

		nanvix_ipc_close(client);
	}
	nanvix_ipc_unlink(channel);


	return (NANVIX_SUCCESS);
}

/**
 * @brief Unit test client.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static int client(void)
{
	int channel;            /* Communication channel. */
	char buf[MESSAGE_SIZE]; /* Buffer.                */

	for (int i = 0; i < MESSAGE_SIZE; i++)
		buf[i] = i%sizeof(char);

	/* Send messages, */
	for (int i = 0; i < NR_MESSAGES; i++)
	{
		int ack;

		channel = nanvix_ipc_connect(IPC_TEST_NAME);

		nanvix_ipc_send(channel, buf, sizeof(buf));

		/* Parse ackowledge message. */
		nanvix_ipc_receive(channel, &ack, sizeof(ack));
		if (ack != NANVIX_SUCCESS)
			return (NANVIX_FAILURE);

		nanvix_ipc_close(channel);
	}

	return (NANVIX_SUCCESS);
}

/**
 * @brief IPC library unit test
 */
int main(int argc, char **argv)
{
	int ret;

	/* Missing parameters. */
	if (argc < 2)
	{
		kprintf("missing parameters");
		kprintf("usage: ipc.test <mode>");
		kprintf("  --client Client mode.");
		kprintf("  --server Server mode.");

		return (NANVIX_SUCCESS);
	}

	/* Server */
	ret = (!kstrcmp(argv[1], "--server")) ? 
		server() : client();

	if (ret == NANVIX_SUCCESS)
		kprintf("ipc test passed");
	else
		kprintf("ipc test FAILED");

	return (NANVIX_SUCCESS);
}
