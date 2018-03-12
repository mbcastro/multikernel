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

#include <stdio.h>
#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>

/**
 * @brief Magic number used for checksum.
 */
#define MAGIC 0xdeadbeef

/**
 * @brief Unit test server.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static int server(void)
{
	unsigned msg1 = MAGIC;
	unsigned msg2 = ~MAGIC;

	nanvix_connector_receive(msg2, sizeof(unsigned));
	nanvix_connector_send(CLUSTER1, &msg2, sizeof(unsigned));

	return (msg1 == msg2);
}

/**
 * @brief Unit test client.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static int client(void)
{
	unsigned msg1 = MAGIC;
	unsigned msg2 = ~MAGIC;

	nanvix_connector_send(CLUSTER0, &msg1, sizeof(unsigned));
	nanvix_connector_receive(&msg2, sizeof(unsigned));

	return (msg1 == msg2);
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
		printf("missing parameters");
		printf("usage: noc.test <mode>");
		printf("  --client Client mode.");
		printf("  --server Server mode.");

		return (NANVIX_SUCCESS);
	}

	nanvix_connector_init();

	/* Server */
	ret = (!strcmp(argv[1], "--server")) ? 
		server() : client();

	printf("noc test [%s]", (ret) ? "passed" : "FAILED");

	return (EXIT_SUCCESS);
}
