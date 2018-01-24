
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
static struct channel channels[NR_CHANNELS] = {{0, 0, 0}, };

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
 * 
 * @returns Upon successful completion, the ID of the
 * target channel is returned. Otherwise -1 is returned
 * instead.
 */
int nanvix_ipc_create(const char *name, int max)
{
	int id;
	int ret;
	int local;
	struct nanvix_process_addr addr;

	assert(name != NULL);
	assert(max > 0);

	/* Gets a free channel. */
	if ((id = nanvix_ipc_channel_get()) == -1)
		return (-1);

	/* Get my rank. */
	if (nanvix_lookup(name, &addr))
		goto erro0;
	local = addr.rank;

	/* Open sync connector. */
	channel[id].syncfd = mppa_open_sync(local, O_RDONLY);
	assert(channel[id].syncfd != -1);

	/* Open portal connectors. */
	for (int i = 0; i < nremotes; i++)
	{
 		int mode = (local == remotes[i]) ? O_RDONLY : O_WRONLY;
		channel[id].portals[i] = mppa_open_portal(remotes[i], mode);
		assert(channel[id].portals[i] != -1);
	}

	/* Initialize sync connector. */
	match = -(1 << 0);
	ret = mppa_ioctl(channel[id].syncfd, MPPA_RX_SET_MATCH, match);
	assert(ret != -1);

	kdebug("[ipc] creating channel %d", id);

	return (id);

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
	ssize_t count;
	long long match;

	/* Sanity check. */
	if (!nanvix_ipc_channel_is_valid(id))
		return (-1);

	/* Sync with remote. */
	count = mppa_read(channel[id].syncfd, &match, sizeof(long long));
	assert(count == sizeof(long long));

	kdebug("[ipc] openning channel %d", id2);

	return (id);

error0:
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
int nanvix_ipc_connect(const char *name)
{
	int id;
	int local;
	int remote;
	ssize_t count;
	long long match;
	struct nanvix_process_addr addr;

	/* Gets a free channel. */
	if ((id = nanvix_ipc_channel_get()) == -1)
		return (-1);

	kdebug("[ipc] connecting to channel %s using %d", name, id);

	/* Get local rank. */
	if (nanvix_lookup("local", &addr))
		goto erro0;
	local = addr.rank;

	/* Get remote rank. */
	if (nanvix_lookup(name, &addr))
		goto erro0;
	remote = addr.rank;

	/* Open sync connector with remote. */
	channel[id].syncfd = mppa_open_sync(remote, tag, O_WRONLY);
	assert(channel[id].syncfd != -1);

	/* Open input portal connector. */
	channel[id].portals[local] = mppa_open_portal(local, tag, O_RDONLY);
	assert(channel[id].portals[local] != -1);

	/* Open output portal conenctor. */
	channel[id].portals[remote] = mppa_open_portal(remote, tag, O_WRONLY);
	assert(channel[id].portals[remote] != -1);
	
	/* Sync with remote. */
	match = 1 << 0;
	count = mppa_write(channel[id].syncfd, &match, sizeof(long long));
	assert(count == sizeof(long long));

	return (id);

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

	kdebug("[ipc] closing channel %d", id);

	nanvix_ipc_channel_put(id);

	return (0);
}
