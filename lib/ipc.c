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

#include <nanvix/ipc.h>
#include <nanvix/hal.h>
#include <nanvix/klib.h>
#include <nanvix/name.h>

#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

/**
 * @brief Number of communication channels.
 */
#define NR_CHANNELS 128

/**
 * @brief Flags for a channel.
 */
/**@{*/
#define CHANNEL_VALID 1
/**@}*/

/**
 * @brief IPC channel.
 */
struct channel
{
	int flags;  /**< Status            */
	int local;  /**< Local socket ID.  */  
	int remote; /**< Remote socket id. */
};

/**
 * @brief Table of channels.
 */
static struct channel channels[NR_CHANNELS];

/**
 * @brief Asserts if an IPC channel is valid.
 *
 * @param id ID of the target IPC channel.
 *
 * @returns If the target channel is valid, one is
 * returned. Otherwise, zero is returned instead.
 */
static int nanvix_ipc_channel_is_valid(int id)
{
	/* Sanity check */
	assert(id >= 0);
	assert(id < NR_CHANNELS);

	return (channels[id].flags & CHANNEL_VALID);
}

/**
 * @brief Allocates an IPC channel.
 *
 * @returns Upon successful completion, the ID of the
 * target channel is returned. Otherwise -1 is returned
 * instead.
 */
static int nanvix_ipc_channel_get(void)
{
	for (int i = 0; i < NR_CHANNELS; i++)
	{
		/* Free channel found. */
		if (!(channels[i].flags & CHANNEL_VALID))
		{
			channels[i].flags |= CHANNEL_VALID;
			return (i);
		}
	}

	return (-1);
}

/**
 * @brief Releases an IPC channel.
 * 
 * @param id ID of the target IPC channel.
 */
static void nanvix_ipc_channel_put(int id)
{
	/* Sanity check */
	assert(id >= 0);
	assert(id < NR_CHANNELS);

	channels[id].flags = 0;
}

/**
 * @brief Creates an IPC channel.
 *
 * @param name  IPC channel name.
 * @param max   Maximum number of simultaneous connections.
 * @param flags IPC channel flags.
 * 
 * @returns Upon successful completion, the ID of the
 * target channel is returned. Otherwise -1 is returned
 * instead.
 */
int nanvix_ipc_create(const char *name, int max, int flags)
{
	int id;
	struct sockaddr_in local;
	struct nanvix_process_addr addr;

	assert(name != NULL);
	assert(max > 0);

	kdebug("[ipc] creating channel");

	/* Gets a free channel. */
	if ((id = nanvix_ipc_channel_get()) == -1)
		return (-1);

	/* Create local socket. */
	channels[id].local = socket(AF_INET, SOCK_STREAM | (flags & SOCK_NONBLOCK), 0);
	if (channels[id].local == -1)
		goto error0;

	/* Build socket. */
	nanvix_lookup(name, &addr);
	kmemset(&local, 0, sizeof(struct sockaddr_in));
	local.sin_family = AF_INET;
	local.sin_port = addr.port;
	local.sin_addr.s_addr = INADDR_ANY;

	/* Bind local socket. */
	if (bind(channels[id].local, (struct sockaddr *)&local, sizeof(struct sockaddr_in)) == -1)
		goto error1;

	/* Listen connections on local socket. */
	if (listen(channels[id].local, NANVIX_IPC_MAX) == -1)
		goto error1;

	return (id);

error1:
	close(channels[id].local);
error0:
	nanvix_ipc_channel_put(id);
	perror("cannot nanvix_ipc_create()");
	return (-1);
}
/**
 * @brief Opens an IPC channel.
 *
 * @param id ID of the target IPC channel.
 *
 * @returns Upon successful completion, the ID of the
 * target channel is returned. Otherwise -1 is returned
 * instead.
 */
int nanvix_ipc_open(int id)
{
	int id2;


	/* Sanity check. */
	if (!nanvix_ipc_channel_is_valid(id))
		return (-1);

	/* Gets a free channel. */
	if ((id2 = nanvix_ipc_channel_get()) == -1)
		return (-1);

	if ((channels[id2].remote = accept(channels[id].local, NULL, NULL)) == -1)
		goto error0;

	channels[id2].local = channels[id].local;

	kdebug("[ipc] openning channel %d", channels[id2].remote);

	return (id2);

error0:
	nanvix_ipc_channel_put(id2);

	kdebug("[ipc] cannot open channel");
	return (-1);
}


/**
 * Conects to an IPC channel.
 *
 * @param name  IPC channel name.
 * @param flags IPC channel flags.
 * 
 * @returns Upon successful completion, the ID of the
 * target channel is returned. Otherwise -1 is returned
 * instead.
 */
int nanvix_ipc_connect(const char *name, int flags)
{
	int id;
	struct sockaddr_in remote;
	struct nanvix_process_addr addr;

	/* Gets a free channel. */
	if ((id = nanvix_ipc_channel_get()) == -1)
		return (-1);

	kdebug("[ipc] connecting to channel %s using %d", name, id);

	/* Create remote socket. */
	channels[id].remote = socket(AF_INET, SOCK_STREAM, 0);
	if (channels[id].remote == -1)
		goto error0;

	/* Initialize socket. */
	kmemset(&remote, 0, sizeof(struct sockaddr_in));
	remote.sin_family = AF_INET;
	nanvix_lookup(name, &addr);
	kmemcpy(&remote.sin_addr.s_addr, &addr.addr, sizeof(in_addr_t));
	remote.sin_port = addr.port;

	/* Connect to socket. */
	if (connect(channels[id].remote, (struct sockaddr *)&remote, sizeof(struct sockaddr_in)) == -1)
		goto error1;

	channels[id].flags |= (flags & CHANNEL_NONBLOCK) ? 2 : 0;

	return (id);

error1:
	close(channels[id].remote);
error0:
	nanvix_ipc_channel_put(id);
	kdebug("cannot connect to channel");
	return (-1);
}

/**
 * @brief Closes an IPC channel.
 *
 * @param id ID of the target IPC channel.
 *
 * @return Upon successful completion zero is returned. Upon failure, non-zero
 * is returned instead.
 */
int nanvix_ipc_close(int id)
{

	/* Sanity check. */
	if (!nanvix_ipc_channel_is_valid(id))
		return (-1);

	/* Close underlying remote socket. */
	if (close(channels[id].remote) == -1)
		return (-1);
	kdebug("[ipc] closing channel");

	nanvix_ipc_channel_put(id);

	return (0);
}

/**
 * @brief Unlinks an IPC channel.
 *
 * @param id ID of the target IPC channel.
 *
 * @return Upon successful completion zero is returned. Upon failure, non-zero
 * is returned instead.
 */
int nanvix_ipc_unlink(int id)
{
	/* Sanity check. */
	if (!nanvix_ipc_channel_is_valid(id))
		return (-1);

	/* Close IPC channel. */
	if (close(channels[id].local) == -1)
		return (-1);
	kdebug("unlinking channel...");

	nanvix_ipc_channel_put(id);

	return (0);
}

/**
 * @brief Sends data over an IPC channel.
 *
 * @param id  ID of the target IPC channel.
 * @param buf Data.
 * @param n   Number of bytes.
 *
 * @return Upon successful completion zero is returned. Upon failure, non-zero
 * is returned instead.
 */
int nanvix_ipc_send(int id, const void *buf, size_t n)
{
	ssize_t ret;

	/* Sanity check. */
	if (!nanvix_ipc_channel_is_valid(id))
		return (-1);

	if (channels[id].flags & 2)
	{
		fcntl(channels[id].remote, F_SETFL, O_NONBLOCK); /* Change the socket into non-blocking state	*/
        fcntl(channels[id].remote, F_SETFL, O_NONBLOCK); /* Change the socket into non-blocking state	*/
		channels[id].flags &= ~2;
	}

	if ((ret = send(channels[id].remote, buf, n, 0)) == -1)
		return (-1);

	kdebug("[ipc] sending %d bytes", ret);

	return (0);
}

/**
 * @brief receives data from an IPC channel.
 *
 * @param id  ID of the target IPC channel.
 * @param buf Target buffer.
 * @param n   Number of bytes.
 *
 * @return Upon successful completion zero is returned. Upon failure, non-zero
 * is returned instead.
 */
int nanvix_ipc_receive(int id, void *buf, size_t n)
{
	ssize_t ret;

	/* Sanity check. */
	if (!nanvix_ipc_channel_is_valid(id))
		return (-1);

	if (channels[id].flags & 2)
	{
		fcntl(channels[id].remote, F_SETFL, O_NONBLOCK); /* Change the socket into non-blocking state	*/
        fcntl(channels[id].remote, F_SETFL, O_NONBLOCK); /* Change the socket into non-blocking state	*/
		channels[id].flags &= ~2;
	}

	if ((ret = recv(channels[id].remote, buf, n, 0)) == -1)
		return (-1);


	kdebug("[ipc] receiving data %d bytes", ret);

	return (0);
}
