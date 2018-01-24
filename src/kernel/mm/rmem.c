struct nanvix_mailbox
{
	int inportal;
	int outportal;
	mppa_aiocb_t 
};

/**
 * @brief Creates a mailbox.
 *
 * @param pathname Name of the mailbox.
 *
 * @returns The ID of the mailbox.
 */
int nanvix_mailbox_create(const char *pathname)
{

}

/**
 * Opens a connection on a mailbox.
 *
 * @returns the Connection number.
 */
int nanvix_mailbox_open(int mailboxid)
{
	struct nanvix_mailbox *mailbox;
	
	/* Invalid mailbox ID. */
	assert((mailboxid >= 0) && (mailboxid < NR_SOCKETS));

	/* Invalid buffer. */
	assert(buffer != NULL);

	/* Invalid buffer size. */
	assert(size > 0);

	mailbox = mailboxs[mailboxid];

	/* Not a valid mailbox. */
	assert(s->flags & SOCKET_VALID);
	
	/* Low-level read operation. */
	s->aiocb = MPPA_AIOCB_INITIALIZER(s->inportal, buffer, size);
	assert(mppa_aio_read(&s->aiocb) == 0);
	assert(mppa_aio_wait(&s->aiocb) == size);
}

/**
 * @brief Reads data from a mailbox synchronously.
 *
 * @param mailboxid ID of the target mailbox.
 * @param buffer   Location where to read data from.
 * @param size     Amount of data (in bytes) to read.
 */
void nanvix_mailbox_read(int mailboxid, void *buffer, size_t size)
{
	struct nanvix_mailbox *mailbox;
	
	/* Invalid mailbox ID. */
	assert((mailboxid >= 0) && (mailboxid < NR_SOCKETS));

	/* Invalid buffer. */
	assert(buffer != NULL);

	/* Invalid buffer size. */
	assert(size > 0);

	mailbox = mailboxs[mailboxid];

	/* Not a valid mailbox. */
	assert(s->flags & SOCKET_VALID);
	
	/* Low-level read operation. */
	s->aiocb = MPPA_AIOCB_INITIALIZER(s->inportal, buffer, size);
	assert(mppa_aio_read(&s->aiocb) == 0);
	assert(mppa_aio_wait(&s->aiocb) == size);
}

/**
 * Writes data to a mailbox asynchronously 
 *
 * @param mailboxid ID of the target mailbox.
 * @param buffer   Location where to write data.
 * @param size     Amount of data (in bytes) to write.
 */
void nanvix_mailbox_write(int mailboxid, void *buffer, size_t size)
{
	struct nanvix_mailbox *mailbox;
	
	/* Invalid mailbox ID. */
	assert((mailboxid >= 0) && (mailboxid < NR_SOCKETS));

	/* Invalid buffer. */
	assert(buffer != NULL);

	/* Invalid buffer size. */
	assert(size > 0);

	mailbox = mailboxs[mailboxid];

	/* Not a valid mailbox. */
	assert(s->flags & SOCKET_VALID);
	
	/* Low-level write operation. */
	mppa_ioctl(s->outportal, MPPA_SET_RX_RANK, XX);
	assert(mppa_pwrite(s->outportal, buffer, size, 0) == size);
}

/**
 *
 */
void server(const char *inbox)
{
	int inboxid;
   
	inboxid = nanvix_mailbox_create(inbox);

	while (1)
	{
		int outboxid;          /* ID of output mailbox. */
		char outbox[PATH_MAX]; /* Name of output mailbox. */

		/* Get request. */
		nanvix_mailbox_read(inbox, &header, sizeof(header), &payload, sizeof(payload));

		/* Serve request. */

		/* Send response. */
		sprintf(outbox, sizeof(outbox) - 1, "/proc/cluster%d", remote);
		outboxid = nanvix_mailbox_open(outbox);
		nanvix_mailbox_write(outboxid, &header, sizeof(header) &payload, sizeof(payload));
		nanvix_mailbox_close(outboxid);
	}

	/* Clean up. */
	nanvix_mailbox_destroy(inbox);
}

void clent(void *local, void *remote, size_t size, int operation)
{
	int inboxid = nanvix_mailbox_create(inbox);

	for (size_t i = 0; i < size; i += BLOCK_SIZE)
	{
		int outboxid;          /* ID of output mailbox. */
		char outbox[PATH_MAX]; /* Name of output mailbox. */

		/* Send request. */
		outboxid = nanvix_mailbox_open(outbox);
		nanvix_mailbox_write(outboxid, &header, sizeof(header) &payload, sizeof(payload));
		nanvix_mailbox_close(outboxid);

		/* Get response. */
		nanvix_mailbox_read(inbox, &header, sizeof(header), &payload, sizeof(payload));

		/* Process response. */
	}


	nanvix_mailbox_destroy(inbox);
}
