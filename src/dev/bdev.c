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
#include <nanvix/dev.h>
#include <nanvix/vfs.h>
#include <nanvix/ramdisk.h>

/**
 * @brief Maximum number of operations to enqueue.
 */
#define CLIENT_MAX 2

/**
 * @brief States for a client.
 */
/**@{*/
#define BDEV_OPEN             0
#define BDEV_RECEIVE          1
#define BDEV_READBLK_CONNECT  2
#define BDEV_READBLK_SEND     3
#define BDEV_READBLK_RECEIVE  4
#define BDEV_WRITEBLK_CONNECT 5
#define BDEV_WRITEBLK_SEND    6
#define BDEV_WRITEBLK_RECEIVE 7
#define BDEV_REPLY            8
#define BDEV_CLOSE            9
#define BDEV_ERROR            10
/**@}*/

/**
 * @brief Block device operation.
 */
struct operation
{
	int status;                         /**< Status.           */
	int client;                         /**< Client channel.   */
	int server;                         /**< Server client.    */
	struct bdev_message request;        /**< Client request.   */
	struct bdev_message reply;          /**< Client reply.     */
	struct ramdisk_message ramdisk_msg; /**< RAM disk message. */
} operations[CLIENT_MAX];

/* Number of block devices. */
#define NR_BLKDEV 4

/*
 * Block devices table.
 */
static const char *bdevsw[NR_BLKDEV] = {
	"ramdisk0", /* /dev/ramdisk0 */
	"ramdisk1", /* /dev/ramdisk1 */
	"ramdisk2", /* /dev/ramdisk2 */
	"ramdisk3", /* /dev/ramdisk3 */
};

/**
 * @brief Opens an IPC channel with a client.
 *
 * @param channel Listen channel.
 * @param op      Target operation.
 */
static void bdev_open(int channel, struct operation *op)
{
	int ret;
	
	ret = nanvix_ipc_open(channel);

	/* Try again. */
	if (ret < 0)
		return;

	kdebug("[bdev] client connected");
	kdebug("[bdev] serving client");

	/* Update operation. */
	op->status = BDEV_RECEIVE;
	op->client = ret;
}

/**
 * @brief Receives a message from a client.
 *
 * @param op Target operation.
 */
void bdev_receive(struct operation *op)
{
	int ret;                      /* IPC operation return value. */
	int channel;                  /* IPC channel.                */
	struct bdev_message *request; /* Client request.             */

	/* Extract parameters of current operation. */
	channel = op->client;
	request = &op->request;

	ret = nanvix_ipc_receive(channel, request, sizeof(struct bdev_message));

	/* Try again. */
	if (ret < 0)
		return;

	/* Handle client request. */
	switch (request->type)
	{
		/* Read a block. */
		case BDEV_MSG_READBLK_REQUEST:
			op->status = BDEV_READBLK_CONNECT;
			kdebug("[bdev] connecting to device server (%d %d)\n",
				op->request.content.readblk_req.dev,
				op->request.content.readblk_req.blknum);
			break;

		/* Write a block. */
		case BDEV_MSG_WRITEBLK_REQUEST:
			op->status = BDEV_WRITEBLK_CONNECT;
			kdebug("[bdev] connecting to device server (%d %d)\n",
				op->request.content.writeblk_req.dev,
				op->request.content.writeblk_req.blknum);
			break;

		/* Error. */
		default:
			op->status = BDEV_ERROR;
			kdebug("[bdev] unknown request type");
			break;
	}
}

/**
 * @brief Connects to a remote server to forward a read block request.
 *
 * @param op Target operation.
 */
static void bdev_readblk_connect(struct operation *op)
{
	int ret;         /* IPC operation return value. */
	dev_t dev;       /* Device.                     */
	unsigned blknum; /* Block number.               */

	/* Extract parameters of current operation. */
	dev = op->request.content.readblk_req.dev;
	blknum = op->request.content.readblk_req.blknum;
	
	/* Invalid device. */
	if ((dev >= NR_BLKDEV) || (bdevsw[dev] == NULL))
		kpanic("[bdev] reading block from invalid device");

	ret = nanvix_ipc_connect(bdevsw[dev], 0);
	
	/* Try again. */
	if (ret < 0)
		return;

	kdebug("[bdev] forwarding read request to device server");

	/* Update current operation. */
	op->server = ret;
	op->ramdisk_msg.type = RAMDISK_MSG_READ_REQUEST;
	op->ramdisk_msg.content.read_req.minor = dev;
	op->ramdisk_msg.content.read_req.blknum = blknum;
	op->status = BDEV_READBLK_SEND;
}

/**
 * @brief Connects to a remote server to forward write block request.
 *
 * @param op Target operation.
 */
static void bdev_writeblk_connect(struct operation *op)
{
	int ret;         /* IPC operation return value. */
	dev_t dev;       /* Device.                     */
	unsigned blknum; /* Block number.               */
	char *buf;       /* Buffer.                     */

	/* Extract parameters of current operation. */
	dev = op->request.content.writeblk_req.dev;
	blknum = op->request.content.writeblk_req.blknum;
	buf = op->request.content.writeblk_req.data;
	
	/* Invalid device. */
	if ((dev >= NR_BLKDEV) || (bdevsw[dev] == NULL))
		kpanic("[bdev] write block from invalid device");

	ret = nanvix_ipc_connect(bdevsw[dev], 0);
	
	/* Try again. */
	if (ret < 0)
		return;

	kdebug("[bdev] forwarding write request to device server");

	/* Update current operation. */
	op->server = ret;
	op->ramdisk_msg.type = RAMDISK_MSG_WRITE_REQUEST;
	op->ramdisk_msg.content.write_req.minor = dev;
	op->ramdisk_msg.content.write_req.blknum = blknum;
	kmemcpy(op->ramdisk_msg.content.write_req.data, buf, BLOCK_SIZE);
	op->status = BDEV_WRITEBLK_SEND;
}

/**
 * @brief Sends a request to remote server to serve read block request.
 *
 * @param op Target operation.
 */
static void bdev_readblk_send(struct operation *op)
{
	int ret;                         /* IPC operation return value. */
	int channel;                     /* IPC channel.                */
	struct ramdisk_message *request; /* Request to remote server.   */

	/* Extract parameters of current operation. */
	channel = op->server;
	request = &op->ramdisk_msg;

	ret = nanvix_ipc_send(channel, request, sizeof(struct ramdisk_message));
	
	/* Try again. */
	if (ret < 0)
		return;

	kdebug("[bdev] waiting for device response");

	/* Update current operation. */
	op->status = BDEV_READBLK_RECEIVE;
}

/**
 * @brief Sends a request to remote server to serve write block request.
 *
 * @param op Target operation.
 */
static void bdev_writeblk_send(struct operation *op)
{
	int ret;                         /* IPC operation return value. */
	int channel;                     /* IPC channel.                */
	struct ramdisk_message *request; /* Request to remote server.   */

	/* Extract parameters of current operation. */
	channel = op->server;
	request = &op->ramdisk_msg;

	ret = nanvix_ipc_send(channel, request, sizeof(struct ramdisk_message));
	
	/* Try again. */
	if (ret < 0)
		return;

	kdebug("[bdev] waiting for device response");

	/* Update current operation. */
	op->status = BDEV_WRITEBLK_RECEIVE;
}

/**
 * @brief Receives a read block request from a remote server.
 *
 * @param op Target operation.
 */
static void bdev_readblk_receive(struct operation *op)
{
	int ret;                       /* IPC operation return value. */
	int channel;                   /* IPC channel.                */
	struct ramdisk_message *reply; /* Reply from remote server.   */

	/* Extract parameters of current operation. */
	channel = op->server;
	reply = &op->ramdisk_msg;

	ret = nanvix_ipc_receive(channel, reply, sizeof(struct ramdisk_message));
	
	/* Try again. */
	if (ret < 0)
		return;

	/* Parse reply. */
	if (reply->type != RAMDISK_MSG_READ_REPLY)
	{
		op->status = BDEV_ERROR;
		return;
	}

	nanvix_ipc_close(channel);

	kdebug("[bdev] replying client");

	/* Update current operation. */
	op->reply.type = BDEV_MSG_READBLK_REPLY;
	op->reply.content.readblk_rep.n = ret;
	kmemcpy(op->reply.content.readblk_rep.data, reply->content.read_rep.data, BLOCK_SIZE);
	op->status = BDEV_REPLY;
}

/**
 * @brief Receives a read block request from a remote server.
 *
 * @param op Target operation.
 */
static void bdev_writeblk_receive(struct operation *op)
{
	int ret;                       /* IPC operation return value. */
	int channel;                   /* IPC channel.                */
	struct ramdisk_message *reply; /* Reply from remote server.   */

	/* Extract parameters of current operation. */
	channel = op->server;
	reply = &op->ramdisk_msg;

	ret = nanvix_ipc_receive(channel, reply, sizeof(struct ramdisk_message));
	
	/* Try again. */
	if (ret < 0)
		return;

	/* Parse reply. */
	if (reply->type != RAMDISK_MSG_WRITE_REPLY)
	{
		op->status = BDEV_ERROR;
		return;
	}

	nanvix_ipc_close(channel);

	kdebug("[bdev] replying client");

	/* Update current operation. */
	op->reply.type = BDEV_MSG_WRITEBLK_REPLY;
	op->reply.content.readblk_rep.n = ret;
	op->status = BDEV_REPLY;
}

/**
 * @brief Sends a reply to the remote client.
 *
 * @param op Target operation.
 */
static void bdev_reply(struct operation *op)
{
	int ret;                    /* IPC operation return value. */
	int channel;                /* IPC channel.                */
	struct bdev_message *reply; /* Reply from remote server.   */

	/* Extract parameters of current operation. */
	channel = op->client;
	reply = &op->reply;

	ret = nanvix_ipc_send(channel, reply, sizeof(struct bdev_message));
	
	/* Try again. */
	if (ret < 0)
		return;

	kdebug("[bdev] disconnecting client");

	/* Update current operation. */
	op->status = BDEV_CLOSE;
}

/**
 * @brief Closes an IPC channel with a client.
 *
 * @param op Target operation.
 */
static void bdev_close(struct operation *op)
{
	int channel;

	channel = op->client;
	
	nanvix_ipc_close(channel);

	kdebug("[bdev] client disconnected");

	/* Update current operation. */
	op->status = BDEV_OPEN;
}

/**
 * @brief Handles a block device error.
 *
 * @param op Target operation.
 */
static void bdev_error(struct operation *op)
{
	kpanic("block device error");
	op->status = BDEV_CLOSE;
}

/**
 * RAM Disk device driver unit test.
 */
int main(int argc, char **argv)
{
	int channel;

	((void) argc);
	((void) argv);

	channel = nanvix_ipc_create(BDEV_NAME, CLIENT_MAX, CHANNEL_NONBLOCK);

	kdebug("[bdev] server running");

	while (1)
	{
		for (int i = 0; i < CLIENT_MAX; i++)
		{
			struct operation *current;

			current = &operations[i];

			/* Handle current operation. */
			switch (current->status)
			{
				case BDEV_OPEN:
					bdev_open(channel, current);
					break;

				case BDEV_RECEIVE:
					bdev_receive(current);
					break;

				case BDEV_READBLK_CONNECT:
					bdev_readblk_connect(current);
					break;

				case BDEV_READBLK_SEND:
					bdev_readblk_send(current);
					break;

				case BDEV_READBLK_RECEIVE:
					bdev_readblk_receive(current);
					break;

				case BDEV_WRITEBLK_CONNECT:
					bdev_writeblk_connect(current);
					break;

				case BDEV_WRITEBLK_SEND:
					bdev_writeblk_send(current);
					break;

				case BDEV_WRITEBLK_RECEIVE:
					bdev_writeblk_receive(current);
					break;

				case BDEV_REPLY:
					bdev_reply(current);
					break;

				case BDEV_ERROR:
					bdev_error(current);
					break;

				case BDEV_CLOSE:
					bdev_close(current);
					break;
			}
		}
	}
	return (NANVIX_SUCCESS);
}


