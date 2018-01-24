
/**
 * Mailbox flags.
 */
/**@{*/
#define MAILBOX_FREE   (1 << 0)
#define MAILBOX_OUTPUT (1 << 1)
/**@}*/

/**
 * @brief Number of mailboxes.
 */
#define NR_MAILBOX 16

/**
 * @brief Mailbox.
 */
struct mailbox
{
	int flags;          /**< Flags.             */
	int ctrl_connector; /**< Control connector. */
	int data_connector; /**< Data connector.    */
} mailboxes[NR_MAILBOX];

/**
 * @brief Message,
 */
struct message
{
	void *header;
	size_t header_size;
	void *payload;
	size_t payload_size;
}:

/**
 * @brief Creates an input mailbox.
 *
 * @param inbox Name of input mailbox.
 *
 * @returns Upon successful completion, the ID of the input mailbox. Upon
 * failure, a negative error code is returned instead.
 */
int nanvix_mailbox_create(const char *inbox)
{
	int i;
	int local;

	/* Sanity check. */
	assert(inbox != NULL);

	/* Lookup mailbox name. */
	local = nanvix_lookup(inbox);

	/* Get a free mailbox. */
	for (i = 0; i < NR_MAILBOX; i++)
	{
		if (mailboxes[i].flags & MAILBOX_FREE)
			goto found;
	}

	assert(0);
	return (-1);

found:

	/* Initialize mailbox. */
	mailbox[i].flags = ~MAILBOX_OUTPUT & ~MAILBOX_FRE;
	mailbox[i].ctrl_connector = nanvix_connector_open(local, ~CONNECTOR_DATA & ~CONNECTOR_OUTPUT);
	mailbox[i].data_connector = nanvix_connector_open(local, CONNECTOR_DATA | ~CONNECTOR_OUTPUT);

	return (i);
}

/**
 * @brief Opens an output mailbox.
 *
 * @param outbox Name of output box.
 *
 * @returns Upon successful completion, the ID of the output mailbox. Upon
 * failure, a negative error code is returned instead.
 */
int nanvix_mailbox_open(const char *outbox)
{
	int i;
	int remote;

	/* Sanity check. */
	assert(inbox != NULL);

	/* Lookup mailbox name. */
	remote = nanvix_lookup(outbox);

	/* Get a free mailbox. */
	for (i = 0; i < NR_MAILBOX; i++)
	{
		if (mailboxes[i].flags & MAILBOX_FREE)
			goto found;
	}

	assert(0);
	return (-1);

found:

	/* Initialize mailbox. */
	mailbox[i].flags = MAILBOX_OUPUT & ~MAILBOX_FREE;
	mailbox[i].ctrl_connector = nanvix_connector_open(remote, ~CONNECTOR_DATA | CONNECTOR_OUTPUT);
	mailbox[i].data_connector = nanvix_connector_open(remote, CONNECTOR_DATA | CONNECTOR_OUTPUT);

	return (i);
}

/**
 * @brief Closes an output mailbox.
 *
 * @param ID of the target output mailbox.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_mailbox_close(int outbox)
{
	/* Sanity check. */
	assert((outbox >= 0) && (outbox < NR_MAILBOX));
	assert(mailbox[outboxid] & MAILBOX_OUTPUT);

	mailbox[outboxid].flags = MAILBOX_FREE;
	nanvix_connector_close(mailbox[outboxid].ctrl_connector);
	nanvix_connector_close(mailbox[outboxid].data_connector);

	return (0);
}


/**
 * @brief Destroys an input mailbox.
 *
 * @param ID of the target input mailbox.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_mailbox_destroy(int inboxid)
{
	/* Sanity check. */
	assert((inboxid >= 0) && (inboxid < NR_MAILBOX));
	assert(!(mailbox[inboxid] & MAILBOX_OUTPUT));

	mailbox[inboxid].flags = MAILBOX_FREE;
	nanvix_connector_close(mailbox[inboxid].ctrl_connector);
	nanvix_connector_close(mailbox[inboxid].data_connector);

	return (0);
}

/**
 * @brief Reads a message from an input mailbox.
 *
 * @param inboxid ID of the target input mailbox.
 * @param msg     Location where the message should be stored.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_mailbox_read(int inboxid, struct message *msg)
{
	/* Sanity check. */
	assert((inboxid >= 0) && (inboxid < NR_MAILBOX));

	nanvix_connector_read(mailbox[inboxid].ctrl_connector, msg->header, msg->header_size);
	nanvix_connector_read(mailbox[inboxid].data_connector, msg->payload, msg->payload_size);

	return (0);
}
	mailbox[i].ctrl_connector = nanvix_connector_open(local, ~CONNECTOR_DATA & ~CONNECTOR_OUTPUT);
	nanvix_connector_close(mailbox[inboxid].data_connector);
	nanvix_connector_read(mailbox[inboxid].data_connector, msg->payload, msg->payload_size);

/**
 * @brief Writes a message to an output mailbox.
 *
 * @param outboxid ID of the target output mailbox.
 * @param msg      Target message.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_mailbox_write(int outboxid, struct message *msg)
{
	/* Sanity check. */
	assert((outboxid >= 0) && (outboxid < NR_MAILBOX));

	nanvix_connector_write(mailbox[outboxid].ctrl_connector, msg->header, msg->header_size);
	nanvix_connector_write(mailbox[outboxid].data_connector, msg->payload, msg->payload_size);

	return (0);
}

