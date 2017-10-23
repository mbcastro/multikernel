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

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <limits.h>

/**
 * @brief Number of channels.
 */
#define NR_CHANNELS 4

/**
 * @brief IPC channel table.
 */
static struct
{
	int local_sfd;             /* Local local descriptor.  */
	int remote_sfd;            /* Remote local descriptor. */
	struct sockaddr_un local;  /* Local socket.            */
	struct sockaddr_un remote; /* Local socket.            */
	char name[PATH_MAX];       /* Socket name,             */
} channels[NR_CHANNELS];

/**
 * @brief Asserts if an IPC channel is valid.
 *
 * @param id ID of the target IPC channel.
 *
 * @return Upon successful completion zero is returned. Upon failure, non-zero
 * is returned instead.
 */
static int nanvix_ipc_is_valid(int id)
{
	/* Invalid communication channel. */
	if ((id < 0) || (id >= NR_CHANNELS))
		return (0);
	
	/* Communication channel not opened. */
	if ((channels[id].local_sfd < 0) && (channels[id].remote_sfd < 0))
		return (0);

	return (1);
}

/**
 * @brief Initializes the IPC library.
 */
static void nanvix_ipc_init(void)
{
	static int initialized = 0;

	/* Nothing to do. */
	if (initialized)
		return;

	for (int i = 0; i < NR_CHANNELS; i++)
	{
		channels[i].local_sfd = -1;
		channels[i].remote_sfd = -1;
	}

	initialized = 1;
}

/**
 * @brief Gets a free channel.
 *
 * @returns The ID of a free channel.
 */
static int nanvix_get_channel(void)
{
	/* Find an empty slot in the channel table. */
	for (int i = 0; i < NANVIX_IPC_MAX; i++)
	{
		/* Found. */
		if (channels[i].local_sfd < 0)
			return (i);
	}

	return (-1);
}

/**
 * @brief Creates an IPC channel.
 *
 * @param name IPC channel name.
 * 
 * @returns Upon successful completion, the ID of the IPC channel is returned.
 * Upon failure -1 is returned instead.
 */
int nanvix_ipc_create(const char *name)
{
	int id;

	nanvix_ipc_init();

	/* Gets a free channel. */
	if ((id = nanvix_get_channel()) == -1)
		return (-1);

	/* Create local. */
	kdebug("creating socket... ");
	channels[id].local_sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (channels[id].local_sfd == -1)
		goto error0;

	/* Bind local. */
	kdebug("bind socket... ");
	memset(&channels[id].local, 0, sizeof(struct sockaddr_un));
	channels[id].local.sun_family = AF_UNIX;
	strncpy(channels[id].local.sun_path, name, sizeof(channels[id].local.sun_path) - 1);
	strncpy(channels[id].name, name, sizeof(channels[id].name));
	unlink(name);
	if (bind(channels[id].local_sfd, (struct sockaddr *)&channels[id].local, sizeof(struct sockaddr_un)) == -1)
		goto error1;

	/* Listen connections on local, */
	kdebug("listening socket... ");
	if (listen(channels[id].local_sfd, NANVIX_IPC_MAX) == -1)
		goto error2;

	channels[id].remote_sfd = -1;

	return (id);

error2:
	unlink(name);
error1:
	close(channels[id].local_sfd);
	channels[id].local_sfd = -1;
error0:
	perror("cannot nanvix_ipc_create()");
	return (-1);
}

/**
 * Conects to an IPC channel.
 *
 * @param name IPC channel name.
 * 
 * @returns Upon successful completion, the ID of the IPC channel is returned.
 * Upon failure -1 is returned instead.
 */
int nanvix_ipc_connect(const char *name)
{
	int id;

	nanvix_ipc_init();

	/* Gets a free channel. */
	if ((id = nanvix_get_channel()) == -1)
		return (-1);

	/* Create local. */
	kdebug("creating socket... ");
	channels[id].remote_sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (channels[id].remote_sfd == -1)
		goto error0;

	/* Connecto to socket. */
	kdebug("connecting to socket... ");
	memset(&channels[id].remote, 0, sizeof(struct sockaddr_un));
	channels[id].remote.sun_family = AF_UNIX;
	strncpy(channels[id].remote.sun_path, name, sizeof(channels[id].remote.sun_path) - 1);
	strncpy(channels[id].name, name, sizeof(channels[id].name));
	if (connect(channels[id].remote_sfd, (struct sockaddr *)&channels[id].remote, sizeof(struct sockaddr_un)) == -1)
		goto error1;

	return (id);

error1:
	close(channels[id].remote_sfd);
	channels[id].remote_sfd = -1;
error0:
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
	nanvix_ipc_init();

	/* Sanity check. */
	if (!nanvix_ipc_is_valid(id))
		return (-1);

	/* Close underlying local. */
	if (close(channels[id].remote_sfd) == -1)
		return (-1);

	channels[id].remote_sfd = -1;

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
	nanvix_ipc_init();

	/* Sanity check. */
	if (!nanvix_ipc_is_valid(id))
		return (-1);

	/* Close IPC channel. */
	if (nanvix_ipc_close(id) == -1)
		return (-1);

	/* Unlink underlying local. */
	if (unlink(channels[id].name) == -1)
		return (-1);

	channels[id].local_sfd = -1;

	return (0);
}

/**
 * @brief Opens an IPC channel.
 *
 * @param id ID of the target IPC channel.
 *
 * @return Upon successful completion zero is returned. Upon failure, non-zero
 * is returned instead.
 */
int nanvix_ipc_open(int id)
{
	int remote_sfd;
	socklen_t remote_size;

	nanvix_ipc_init();

	/* Sanity check. */
	if (!nanvix_ipc_is_valid(id))
		return (-1);

	kdebug("accepting connection to socket... ");
	remote_size = sizeof(struct sockaddr_un);
	if ((remote_sfd = accept(channels[id].local_sfd, (struct sockaddr *)&channels[id].remote, &remote_size)) == -1)
		return (-1);

	channels[id].remote_sfd = remote_sfd;

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
int nanvix_ipc_send(int id, const char *buf, size_t n)
{
	size_t ret;

	nanvix_ipc_init();

	/* Sanity check. */
	if (!nanvix_ipc_is_valid(id))
		return (-1);

	if ((ret = send(channels[id].remote_sfd, buf, n, 0)) != n)
		return (-1);

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
int nanvix_ipc_receive(int id, char *buf, size_t n)
{
	size_t ret;

	nanvix_ipc_init();

	/* Sanity check. */
	if (!nanvix_ipc_is_valid(id))
		return (-1);

	if ((ret = recv(channels[id].remote_sfd, buf, n, 0)) != n)
		return (-1);

	return (0);
}
