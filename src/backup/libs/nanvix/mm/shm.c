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

#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
 * Client cache                                                               *
 *============================================================================*/

/* Implemented but not used. */
static int shm_is_shared(int) __attribute__((unused));

/**
 * @brief Flags for shared region.
 */
/*@{@*/
#define SHM_WRITE  (1 << 0) /**< Writable? Else read-only */
#define SHM_SHARED (1 << 1) /**< Shared? Else private.    */
#define SHM_MAPPED (1 << 2) /**< Mapped? Else unmapped.   */
/**@}*/

/**
 * @brief Number of opened shared memory regions.
 */
static int nopen;

/**
 * Table of opened shared memory regions (cache).
 */
static struct
{
	int shmid;
	int flags;
} oregions[SHM_OPEN_MAX];

/*============================================================================*
 * shm_may_write()                                                            *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node may write on a opened shared
 * memory region.
 *
 * @param id   ID of the target opened shared memory region.
 *
 * @returns One if the target node may write into the target opened
 * shared memory region, and zero otherwise.
 */
static inline int shm_may_write(int id)
{
	return (oregions[id].flags & SHM_WRITE);
}

/*============================================================================*
 * shm_is_shared()                                                            *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node is sharing a target opened
 * shared memory region.
 *
 * @param id   ID of the target opened shared memory region.
 *
 * @returns One if the target node is sharing the target opened
 * shared memory region, and zero otherwise.
 */
static int shm_is_shared(int id)
{
	return (oregions[id].flags & SHM_SHARED);
}

/*============================================================================*
 * shm_has_mapped()                                                           *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node has mapped a target opened
 * shared memory region.
 *
 * @param id   ID of the target opened mapped memory region.
 *
 * @returns One if the target node has mapped the target opened mapped
 * memory region, and zero otherwise.
 */
static inline int shm_has_mapped(int id)
{
	return (oregions[id].flags & SHM_MAPPED);
}

/*============================================================================*
 * shm_clear_flags()                                                          *
 *============================================================================*/

/**
 * @brief Clears the flags of a opened shared memory region.
 *
 * @param id   ID of the target opened shared memory region.
 */
static inline void shm_clear_flags(int id)
{
	oregions[id].flags = 0;
}


/*============================================================================*
 * shm_set_writable()                                                         *
 *============================================================================*/

/**
 * @brief Sets a target opened shared memory region as writable.
 *
 * @param id   ID of the target opened shared memory region.
 */
static inline void shm_set_writable(int id)
{
	oregions[id].flags |= SHM_WRITE;
}

/*============================================================================*
 * shm_set_mapped()                                                           *
 *============================================================================*/

/**
 * @brief Sets a target opened shared memory region as mapped.
 *
 * @param id   ID of the target opened shared memory region.
 */
static inline void shm_set_mapped(int id)
{
	oregions[id].flags |= SHM_MAPPED;
}

/*============================================================================*
 * shm_set_shared()                                                           *
 *============================================================================*/

/**
 * @brief Sets a target opened shared memory region as shared.
 *
 * @param id   ID of the target opened shared memory region.
 */
static inline void shm_set_shared(int id)
{
	oregions[id].flags |= SHM_SHARED;
}

/*============================================================================*
 * shm_has_opened()                                                           *
 *============================================================================*/

/**
 * @brief Asserts whether or not a node has opened a given shared
 * memory region.
 *
 * @param shmid ID of target shared memory region.
 *
 * @return If the target node has opened the target shared memory
 * region, its index in the table of opened shared memory regions is
 * returned. Otherwise, -1 is returned instead.
 */
static int shm_has_opened(int shmid)
{
	for (int i = 0; i < nopen; i++)
	{
		if (oregions[i].shmid == shmid)
			return (i);
	}

	return (-1);
}

/*============================================================================*
 * shm_has_mapped()                                                           *
 *============================================================================*/

/**
 * @brief Asserts whether or not a shared memory region is mapped by a
 * node.
 *
 * @param shmid ID of target shared memory region.
 *
 * @returns One if the target node has mapped the target opened mapped
 * memory region, and zero otherwise.
 */
static inline int shm_is_mapped(int shmid)
{
	for (int i = 0; i < nopen; i++)
	{
		if (oregions[i].shmid == shmid)
		{
			if (shm_has_opened(i))
				return (1);
		}
	}
	return (0);
}

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
	{
		errno = EINVAL;
		return (-1);
	}

	/* Name too long. */
	if (strlen(name) >= (SHM_NAME_MAX - 1))
	{
		errno = ENAMETOOLONG;
		return (-1);
	}

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
	/* Sanity check at compile time: Mailbox compliant */
	CHECK_MAILBOX_MSG_SIZE(struct shm_message);

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
	int i;
	int ret;
	int inbox;
	int nodenum;
	struct shm_message msg;

	/* Uninitalized server. */
	if (!server.initialized)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Cannot get inbox. */
	if ((inbox = get_inbox()) < 0)
		return (-1);

	/* Invalid name. */
	if (nanvix_shm_is_invalid_name(name))
		return (-1);

	nodenum = sys_get_node_num();

	/* Build message header. */
	msg.header.source = nodenum;
	msg.header.opcode = SHM_CREATE_EXCL;

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

	/* Failed to create shared memory region. */
	if (msg.header.opcode == SHM_FAILURE)
	{
		errno = msg.op.ret.status;
		return (-1);
	}

	i = nopen++;
	oregions[i].shmid = msg.op.ret.shmid;
	shm_clear_flags(i);
	if (rw)
		shm_set_writable(i);

	return (msg.op.ret.shmid);

error:
	pthread_mutex_unlock(&lock);
	errno = -ret;
	return (-1);
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
	int i;
	int ret;
	int inbox;
	int nodenum;
	struct shm_message msg;

	/* Uninitalized server. */
	if (!server.initialized)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Cannot get inbox. */
	if ((inbox = get_inbox()) < 0)
		return (-1);

	/* Invalid name. */
	if (nanvix_shm_is_invalid_name(name))
		return (-1);

	nodenum = sys_get_node_num();

	/* Build message header. */
	msg.header.source = nodenum;
	msg.header.opcode = SHM_CREATE;

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

	/* Failed to create shared memory region. */
	if (msg.header.opcode == SHM_FAILURE)
	{
		errno = msg.op.ret.status;
		return (-1);
	}

	i = nopen++;
	oregions[i].shmid = msg.op.ret.shmid;
	shm_clear_flags(i);
	if (rw)
		shm_set_writable(i);

	return (msg.op.ret.shmid);

error:
	pthread_mutex_unlock(&lock);
	errno = -ret;
	return (-1);
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
	int i;
	int ret;
	int inbox;
	int nodenum;
	struct shm_message msg;

	/* Uninitalized server. */
	if (!server.initialized)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Cannot get inbox. */
	if ((inbox = get_inbox()) < 0)
		return (-1);

	/* Invalid name. */
	if (nanvix_shm_is_invalid_name(name))
		return (-1);

	/* Too many files are opened. */
	if (nopen >= SHM_OPEN_MAX)
	{
		errno = ENFILE;
		return (-1);
	}

	/* Truncate but cannot write. */
	if (truncate && !rw)
	{
		errno = EINVAL;
		return (-1);
	}

	nodenum = sys_get_node_num();

	/* Build message header. */
	msg.header.source = nodenum;
	msg.header.opcode = SHM_OPEN;

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

	/* Failed to open shared memory region. */
	if (msg.header.opcode == SHM_FAILURE)
	{
		errno = msg.op.ret.status;
		return (-1);
	}

	i = nopen++;
	oregions[i].shmid = msg.op.ret.shmid;
	shm_clear_flags(i);
	if (rw)
		shm_set_writable(i);

	return (msg.op.ret.shmid);

error:
	pthread_mutex_unlock(&lock);
	errno = -ret;
	return (-1);
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
	int i;
	int ret;
	int inbox;
	int nodenum;
	struct shm_message msg;

	/* Uninitalized server. */
	if (!server.initialized)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Cannot get inbox. */
	if ((inbox = get_inbox()) < 0)
		return (-1);

	/* Invalid name. */
	if (nanvix_shm_is_invalid_name(name))
		return (-1);

	nodenum = sys_get_node_num();

	/* Build message. */
	msg.header.source = nodenum;
	msg.header.opcode = SHM_UNLINK;
	msg.seq = ((nodenum << 4) | 0);
	strcpy(msg.op.unlink.name, name);

	pthread_mutex_lock(&lock);

		if ((ret = sys_mailbox_write(server.outbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	/* Failed to unlink shared memory region. */
	if (msg.header.opcode == SHM_FAILURE)
	{
		errno = msg.op.ret.status;
		return (-1);
	}

	/*
	 * The process should have opened
	 * the shared memory region before.
	 */
	if ((i = shm_has_opened(msg.op.ret.shmid)) < 0)
	{
		errno = EACCES;
		return (-1);
	}

	/* Remove the shared region from the list. */
	--nopen;
	for (int j = i; j < nopen; j++)
	{
		oregions[j].shmid = oregions[j + 1].shmid;
		oregions[j].flags = oregions[j + 1].flags;
	}

	return (0);

error:
	pthread_mutex_unlock(&lock);
	errno = -ret;
	return (-1);
}

/*============================================================================*
 * nanvix_map()                                                               *
 *============================================================================*/

/**
 * @brief Maps pages of memory.
 *
 * @param mapblk   Location at mapped block should be stored.
 * @param len      Length of mapping (in bytes).
 * @param writable Writable? Else read-only.
 * @param shared   Shared? Else private.
 * @param fd       Target file descriptor.
 * @param off      Offset within file.
 *
 * @retuns Upon successful completion, zero is returned. Upon failure,
 * -1 is returned instead and errno is set to indicate the error.
 */
int nanvix_map(uint64_t *mapblk, size_t len, int writable, int shared, int fd, off_t off)
{
	int i;
	int ret;
	int inbox;
	int nodenum;
	struct shm_message msg;

	/* Uninitalized server. */
	if (!server.initialized)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Invalid length. */
	if (len == 0)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Cannot get inbox. */
	if ((inbox = get_inbox()) < 0)
		return (-1);

	/*
	 * The process should have opened
	 * the shared memory region before.
	 */
	if ((i = shm_has_opened(fd)) < 0)
	{
		errno = EACCES;
		return (-1);
	}

	/* Cannot write. */
	if (writable && (!shm_may_write(i)))
	{
		errno = EACCES;
		return (-1);
	}

	nodenum = sys_get_node_num();

	/* Build message. */
	msg.header.source = nodenum;
	msg.header.opcode = SHM_MAP;
	msg.seq = ((nodenum << 4) | 0);
	msg.op.map.shmid = fd;
	msg.op.map.size = len;
	msg.op.map.writable = writable;
	msg.op.map.shared = shared;
	msg.op.map.off = off;

	pthread_mutex_lock(&lock);

		if ((ret = sys_mailbox_write(server.outbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	/* Failed to map. */
	if (msg.header.opcode == SHM_FAILURE)
	{
		errno = msg.op.ret.status;
		return (-1);
	}

	/* Map. */
	if (!shm_has_mapped(i))
	{
		shm_set_mapped(i);
		if (shared)
			shm_set_shared(i);
	}

	*mapblk = msg.op.ret.mapblk;

	return (0);

error:
	pthread_mutex_unlock(&lock);
	errno = -ret;
	return (-1);
}

/*============================================================================*
 * nanvix_munmap()                                                            *
 *============================================================================*/

/**
 * @brief Unmaps pages of memory.
 *
 * @param shmid ID of the target shared memory region.
 * @param len   Length of mapping (in bytes).
 *
 * @retuns Upon successful completion, zero is returned. Otherwise, -1
 * is returned and errno is set to indicate the error.
 */
int nanvix_unmap(int shmid, size_t len)
{
	int i;
	int ret;
	int inbox;
	int nodenum;
	struct shm_message msg;

	/* Uninitalized server. */
	if (!server.initialized)
	{
		errno = EAGAIN;
		return (-1);
	}

	/* Invalid length. */
	if (len == 0)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Cannot get inbox. */
	if ((inbox = get_inbox()) < 0)
		return (-1);

	/*
	 * The process should have opened
	 * the shared memory region before.
	 */
	if ((i = shm_has_opened(shmid)) < 0)
	{
		errno = EACCES;
		return (-1);
	}

	/* Not mapped. */
	if (!shm_has_mapped(i))
	{
		errno = EINVAL;
		return (-1);
	}

	nodenum = sys_get_node_num();

	/* Build message. */
	msg.header.source = nodenum;
	msg.header.opcode = SHM_UNMAP;
	msg.seq = ((nodenum << 4) | 0);
	msg.op.unmap.shmid = shmid;
	msg.op.unmap.size = len;

	pthread_mutex_lock(&lock);

		if ((ret = sys_mailbox_write(server.outbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	/* Failed to unmap. */
	if (msg.header.opcode == SHM_FAILURE)
	{
		errno = msg.op.ret.status;
		return (-1);
	}

	return (0);

error:
	pthread_mutex_unlock(&lock);
	errno = -ret;
	return (-1);
}

/*============================================================================*
 * nanvix_mtruncate()                                                         *
 *============================================================================*/

/**
 * @brief Truncates a shared memory region to a specified size.
 *
 * @param shmid  ID of the target shared memory region.
 * @param length Shared memory region size (in bytes).
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, -1 is returned instead, and errno is set to indicate the
 * error.
 */
int nanvix_mtruncate(int shmid, size_t size)
{
	int i;
	int ret;
	int inbox;
	int nodenum;
	struct shm_message msg;

	/* Cannot get inbox. */
	if ((inbox = get_inbox()) < 0)
		return (-1);

	/*
	 * The process should have opened
	 * the shared memory region before.
	 */
	if ((i = shm_has_opened(shmid)) < 0)
	{
		errno = EACCES;
		return (-1);
	}

	/* Cannot write. */
	if (!shm_may_write(i))
	{
		errno = EINVAL;
		return (-1);
	}

	/* Already mapped. */
	if (shm_is_mapped(shmid))
	{
		errno = EBUSY;
		return (-1);
	}

	nodenum = sys_get_node_num();

	/* Build message. */
	msg.header.source = nodenum;
	msg.header.opcode = SHM_TRUNCATE;
	msg.seq = ((nodenum << 4) | 0);
	msg.op.truncate.shmid = shmid;
	msg.op.truncate.size = size;

	pthread_mutex_lock(&lock);

		if ((ret = sys_mailbox_write(server.outbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

		if ((ret = sys_mailbox_read(inbox, &msg, sizeof(struct shm_message))) != MAILBOX_MSG_SIZE)
			goto error;

	pthread_mutex_unlock(&lock);

	/* Failed to truncate. */
	if (msg.header.opcode == SHM_FAILURE)
	{
		errno = msg.op.ret.status;
		return (-1);
	}

	return (0);

error:
	pthread_mutex_unlock(&lock);
	errno = -ret;
	return (-1);
}
