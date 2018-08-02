/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>

#include <nanvix/const.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/syscalls.h>

/**
 * @brief Shared Memory Server server connection.
 */
static struct
{
	int initialized; /**< Is the connection initialized? */
	int outbox;      /**< Output mailbox for requests.   */
} server = { 0, -1 };

/**
 * @brief Shared Memory module lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*============================================================================*
 * nanvix_shm_is_invalid_name()                                               *
 *============================================================================*/

/**
 * @brief Asserts whether or not a shared memory region has a valid
 * name.
 *
 * @param name Target name.
 *
 * @returns Zero if the target @p name is valid and a negative error
 * code otherwise.
 */
static inline int nanvix_shm_is_invalid_name(const char *name)
{
	/* Invalid name. */
	if ((name == NULL) || (!strcmp(name, "")))
		return (-EINVAL);

	/* Name too long. */
	if (strlen(name) >= (SHM_NAME_MAX - 1))
		return (-ENAMETOOLONG);

	return (0);
}

/*============================================================================*
 * nanvix_shm_init()                                                          *
 *============================================================================*/

/**
 * @brief Initializes the Shared Memory client.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int nanvix_shm_init(void)
{
	/* Nothing to do.  */
	if (server.initialized)
		return (0);

	/* Open output mailbox */
	if ((server.outbox = sys_mailbox_open(SHM_SERVER_NODE)) < 0)
	{
		printf("[nanvix][shm] cannot open outbox to server\n");
		return (server.outbox);
	}

	server.initialized = 1;

	return (0);
}

/*============================================================================*
 * nanvix_shm_finalize()                                                      *
 *============================================================================*/

/**
 * @brief Closes the Shared Memory client.
 */
void nanvix_shm_finalize(void)
{
	/* Nothing to do. */
	if (!server.initialized)
		return;

	/* Close output mailbox. */
	if (sys_mailbox_close(server.outbox) < 0)
	{
		printf("[nanvix][shm] cannot close outbox to server\n");
		return;
	}

	server.initialized = 0;
}

/*============================================================================*
 * nanvix_shm_create_excl()                                                   *
 *============================================================================*/

/**
 * @brief Creates an exclusive shared memory region.
 *
 * @param name Name of the new shared memory region.
 * @param rw   Read write? Otherwise read-only.
 * @param mode Access permissions.
 *
 * @returns Upon successful completion, a descriptor for the newly
 * created shared memory region is returned. Upon failure, a negative
 * error code is returned instead.
 */
int nanvix_shm_create_excl(const char *name, int rw, mode_t mode)
{
	int ret;
	int inbox;
	int nodenum;
	struct shm_message msg;

	/* Cannot get inbox. */
	if ((inbox = get_inbox()) < 0)
		return (-EAGAIN);

	/* Invalid name. */
	if ((ret = nanvix_shm_is_invalid_name(name)))
		return (ret);

	nodenum = sys_get_node_num();

	/* Build message header. */
	msg.source = nodenum;
	msg.opcode = SHM_CREATE_EXCL;

	pthread_mutex_lock(&lock);

		/* Build message 1.*/
		msg.seq = ((nodenum << 4) | 0);
		strcpy(msg.op.create1.name, name);

		if ((ret = sys_mailbox_write(server.outbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

		/* Build message 2. */
		msg.seq = ((nodenum << 4) | 1);
		msg.op.create2.mode = mode;
		msg.op.create2.excl = 1;
		msg.op.create2.rw = rw;

		if ((ret = sys_mailbox_write(server.outbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg.op.ret);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/*============================================================================*
 * nanvix_shm_create()                                                        *
 *============================================================================*/

/**
 * @brief Creates a shared memory region.
 *
 * @param name     Name of the new shared memory region.
 * @param rw       Read write? Otherwise read-only.
 * @param truncate Truncate region?
 * @param mode     Access permissions.
 *
 * @returns Upon successful completion, a descriptor for the newly
 * created shared memory region is returned. Upon failure, a negative
 * error code is returned instead.
 */
int nanvix_shm_create(const char *name, int rw, int truncate, mode_t mode)
{
	int ret;
	int inbox;
	int nodenum;
	struct shm_message msg;

	/* Cannot get inbox. */
	if ((inbox = get_inbox()) < 0)
		return (-EAGAIN);

	/* Invalid name. */
	if ((ret = nanvix_shm_is_invalid_name(name)))
		return (ret);

	nodenum = sys_get_node_num();

	/* Build message header. */
	msg.source = nodenum;
	msg.opcode = SHM_CREATE;

	pthread_mutex_lock(&lock);

		/* Build message 1.*/
		msg.seq = ((nodenum << 4) | 0);
		strcpy(msg.op.create1.name, name);

		if ((ret = sys_mailbox_write(server.outbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

		/* Build message 2. */
		msg.seq = ((nodenum << 4) | 1);
		msg.op.create2.mode = mode;
		msg.op.create2.excl = 0;
		msg.op.create2.rw = rw;
		msg.op.create2.truncate = truncate;

		if ((ret = sys_mailbox_write(server.outbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg.op.ret);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/*============================================================================*
 * nanvix_shm_open()                                                          *
 *============================================================================*/

/**
 * @brief Opens a shared memory region.
 *
 * @param name     Name of the new shared memory region.
 * @param rw       Read write? Otherwise read-only.
 * @param truncate Truncate region?
 *
 * @returns Upon successful completion, a descriptor for the target
 * shared memory region is returned. Upon failure, a negative error
 * code is returned instead.
 */

int nanvix_shm_open(const char *name, int rw, int truncate)
{
	int ret;
	int inbox;
	int nodenum;
	struct shm_message msg;

	/* Cannot get inbox. */
	if ((inbox = get_inbox()) < 0)
		return (-EAGAIN);

	/* Invalid name. */
	if ((ret = nanvix_shm_is_invalid_name(name)))
		return (ret);

	nodenum = sys_get_node_num();

	/* Build message header. */
	msg.source = nodenum;
	msg.opcode = SHM_OPEN;

	pthread_mutex_lock(&lock);

		/* Build message 1.*/
		msg.seq = ((nodenum << 4) | 0);
		strcpy(msg.op.open1.name, name);

		if ((ret = sys_mailbox_write(server.outbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

		/* Build message 2. */
		msg.seq = ((nodenum << 4) | 1);
		msg.op.open2.rw = rw;
		msg.op.open2.truncate = truncate;

		if ((ret = sys_mailbox_write(server.outbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg.op.ret);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/*============================================================================*
 * nanvix_shm_unlink()                                                        *
 *============================================================================*/

/**
 * @brief Removes a shared memory region.
 *
 * @param name Name of the target existing shared memory region.
 *
 * @returns Upon successful completion, a descriptor for the target
 * shared memory region is returned. Upon failure, a negative error
 * code is returned instead.
 */
int nanvix_shm_unlink(const char *name)
{
	int ret;
	int inbox;
	int nodenum;
	struct shm_message msg;

	/* Cannot get inbox. */
	if ((inbox = get_inbox()) < 0)
		return (-EAGAIN);

	/* Invalid name. */
	if ((ret = nanvix_shm_is_invalid_name(name)))
		return (ret);

	nodenum = sys_get_node_num();

	/* Build message. */
	msg.source = nodenum;
	msg.opcode = SHM_UNLINK;
	msg.seq = ((nodenum << 4) | 0);
	strcpy(msg.op.unlink.name, name);

	pthread_mutex_lock(&lock);

		if ((ret = sys_mailbox_write(server.outbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg.op.ret);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}
