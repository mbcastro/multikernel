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
#include <stdlib.h>
#include <sys/time.h>

#include <nanvix/klib.h>
#include <nanvix/ipc.h>
#include <nanvix/dev.h>

/**
 * @brief Number of messages to exchange.
 */
#define NR_MESSAGES 128

double mysecond()
{
        struct timeval tp;

        gettimeofday(&tp, NULL);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

dev_t dev;

/**
 * @brief Unit test client.
 *
 * @returns Upon successful NANVIX_SUCCESS is returned. Upon failure
 * NANVIX_FAILURE is returned instead.
 */
static int client(void)
{
	int channel;
	double max = 0.0;

	for (int k = 0; k < 1024; k++)
	{
		/* Send messages, */
		for (int i = 0; i < NR_MESSAGES; i++)
		{
			double t1, t2, bandwidth;
			struct bdev_message request;
			struct bdev_message reply;

			t1 = mysecond();

			channel = nanvix_ipc_connect(BDEV_NAME, 0);

			/* Build write request. */
			request.type = BDEV_MSG_WRITEBLK_REQUEST;
			request.content.writeblk_req.dev = dev;
			request.content.writeblk_req.blknum = i;
			for (int j = 0; j < BLOCK_SIZE; j++)
				request.content.writeblk_req.data[j] = i%256;

			nanvix_ipc_send(channel, &request, sizeof(struct bdev_message));

			/* Parse ackowledge message. */
			nanvix_ipc_receive(channel, &reply, sizeof(struct bdev_message));
			if (reply.type == BDEV_MSG_ERROR)
				return (NANVIX_FAILURE);

			t2 = mysecond();

			bandwidth = BLOCK_SIZE/(1024*(t2-t1));
			if (bandwidth > max)
				max = bandwidth;

			nanvix_ipc_close(channel);

			channel = nanvix_ipc_connect(BDEV_NAME, 0);

			/* Build read request. */
			request.type = BDEV_MSG_READBLK_REQUEST;
			request.content.readblk_req.dev = dev;
			request.content.readblk_req.blknum = i;

			nanvix_ipc_send(channel, &request, sizeof(struct bdev_message));

			/* Parse acknowledge message. */
			nanvix_ipc_receive(channel, &reply, sizeof(struct bdev_message));
			if (reply.type == BDEV_MSG_ERROR)
				return (NANVIX_FAILURE);
			for (int j = 0; j < BLOCK_SIZE; j++)
			{
				if (reply.content.readblk_rep.data[j] != i%256)
				{
						kdebug("[bdev.test] checksum failed %c", reply.content.readblk_rep.data[j]);
					return (NANVIX_FAILURE);
				}
			}
			nanvix_ipc_close(channel);
		}
	}

	fprintf(stdout, "[info] [bdev.test] max bandwidth: %lf MB/s\n", max);

	return (NANVIX_SUCCESS);
}

/**
 * @brief IPC library unit test
 */
int main(int argc, char **argv)
{
	int ret;

	dev = (unsigned) atoi(argv[1]);

	((void) argc);
	((void) argv);

	/* Server */
	ret = client();

	if (ret == NANVIX_SUCCESS)
		kprintf("bdev test passed");
	else
		kprintf("bdev test FAILED");

	return (NANVIX_SUCCESS);
}
