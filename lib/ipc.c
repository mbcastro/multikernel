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

#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

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
	int flags;           /**< Status            */
	int local;           /**< Local socket ID.  */  
	int remote;          /**< Remote socket id. */
	char name[PATH_MAX]; /**< Channel name.     */
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
 * @param name IPC channel name.
 * @param max  Maximum number of simultaneous connections.
 * @param fags IPC channel flags.
 * 
 * @returns Upon successful completion, the ID of the
 * target channel is returned. Otherwise -1 is returned
 * instead.
 */
int nanvix_ipc_create(const char *name, int max, int flags)
{
	int id;
	struct sockaddr_un local;

	assert(name != NULL);
	assert(max > 0);

	kdebug("[ipc] creating channel");

	/* Gets a free channel. */
	if ((id = nanvix_ipc_channel_get()) == -1)
		return (-1);

	/* Create local socket. */
	channels[id].local = socket(AF_UNIX, SOCK_STREAM | (flags & SOCK_NONBLOCK), 0);
	if (channels[id].local == -1)
		goto error0;

	/* Build socket. */
	kmemset(&local, 0, sizeof(struct sockaddr_un));
	local.sun_family = AF_UNIX;
	kstrncpy(channels[id].name, name, sizeof(local.sun_path) - 1);
	kstrncpy(local.sun_path, name, sizeof(local.sun_path) - 1);

	/* Bind local socket. */
	unlink(name);
	if (bind(channels[id].local, (struct sockaddr *)&local, sizeof(struct sockaddr_un)) == -1)
		goto error1;

	/* Listen connections on local socket. */
	if (listen(channels[id].local, NANVIX_IPC_MAX) == -1)
		goto error2;

	return (id);

error2:
	unlink(name);
error1:
	close(channels[id].local);
error0:
	nanvix_ipc_channel_put(id);
	kpanic("cannot nanvix_ipc_create()");
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

	kdebug("[ipc] openning channel");

	/* Sanity check. */
	if (!nanvix_ipc_channel_is_valid(id))
		return (-1);

	/* Gets a free channel. */
	if ((id2 = nanvix_ipc_channel_get()) == -1)
		return (-1);

	if ((channels[id2].remote = accept(channels[id].local, NULL, NULL)) == -1)
		goto error0;

	channels[id2].local = channels[id].local;

	return (id2);

error0:
	nanvix_ipc_channel_put(id2);
	return (-1);
}


/**
 * Conects to an IPC channel.
 *
 * @param name IPC channel name.
 * 
 * @returns Upon successful completion, the ID of the
 * target channel is returned. Otherwise -1 is returned
 * instead.
 */
int nanvix_ipc_connect(const char *name)
{
	int id;
	struct sockaddr_un remote;

	kdebug("[ipc] connecting to channel");

	/* Gets a free channel. */
	if ((id = nanvix_ipc_channel_get()) == -1)
		return (-1);

	/* Create remote socket. */
	channels[id].remote = socket(AF_UNIX, SOCK_STREAM, 0);
	if (channels[id].remote == -1)
		goto error0;

	/* Initialize socket. */
	kmemset(&remote, 0, sizeof(struct sockaddr_un));
	remote.sun_family = AF_UNIX;
	kstrncpy(remote.sun_path, name, sizeof(remote.sun_path) - 1);
	kstrncpy(channels[id].name, name, sizeof(remote.sun_path) - 1);

	/* Connect to socket. */
	if (connect(channels[id].remote, (struct sockaddr *)&remote, sizeof(struct sockaddr_un)) == -1)
		goto error1;

	return (id);

error1:
	close(channels[id].remote);
error0:
	nanvix_ipc_channel_put(id);
	perror("cannot nanvix_ipc_connect()");
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
	kdebug("[ipc] closing channel");

	/* Sanity check. */
	if (!nanvix_ipc_channel_is_valid(id))
		return (-1);

	/* Close underlying remote socket. */
	if (close(channels[id].remote) == -1)
		return (-1);

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

	/* Unlink underlying local. */
	if (unlink(channels[id].name) == -1)
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
	size_t ret;

	/* Sanity check. */
	if (!nanvix_ipc_channel_is_valid(id))
		return (-1);

	if ((ret = send(channels[id].remote, buf, n, 0)) != n)
		return (-1);

	kdebug("[ipc] sending data", channels[id].name);

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
	size_t ret;

	/* Sanity check. */
	if (!nanvix_ipc_channel_is_valid(id))
		return (-1);

	if ((ret = recv(channels[id].remote, buf, n, 0)) != n)
		return (-1);

	kdebug("[ipc] receiving data", channels[id].name);

	return (0);
}
