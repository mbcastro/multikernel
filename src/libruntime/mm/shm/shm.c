/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Must come first. */
#define __NEED_RESOURCE
#define __SHM_SERVICE

#include <nanvix/runtime/stdikc.h>
#include <nanvix/runtime/mailbox.h>
#include <nanvix/runtime/shm.h>
#include <nanvix/sys/noc.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/thread.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>

/**
 * @brief Connection to SHMem Server.
 */
static struct
{
	int initialized; /**< Is the connection initialized? */
	int outbox;      /**< Output mailbox for requests.   */
} server = {
	.initialized = 0,
	.outbox = -1
};

/*============================================================================*
 * SHM Client c                                                               *
 *============================================================================*/

/**
 * @brief Number of opened shared memory regions.
 */
static int nopen;

/**
 * Table of opened shared memory regions (cache).
 */
static struct oregion
{
	/*
	 * XXX: Don't Touch! This Must Come First!
	 */
	struct resource resource;  /**< Generic resource information.  */

	int shmid;
	int flags;
} oregions[SHM_OPEN_MAX];

/**
 * @brief Pool of open shared memory regions.
 */
struct resource_pool pool = {
	oregions, SHM_OPEN_MAX, sizeof(struct oregion)
};

/*============================================================================*
 * shm_is_opened()                                                            *
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
static int shm_is_opened(int shmid)
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
			if (shm_is_opened(i))
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
	if ((name == NULL) || (!kstrcmp(name, "")))
		return (-EINVAL);

	/* Name too long. */
	if (kstrlen(name) >= (SHM_NAME_MAX - 1))
		return (-ENAMETOOLONG);

	return (0);
}

/*============================================================================*
 * __nanvix_shm_create()                                                      *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_shm_create(const char *name, int rw, int truncate, mode_t mode)
{
	int i;
	struct shm_message msg;

	/* Uninitalized server. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid name. */
	if (nanvix_shm_is_invalid_name(name))
		return (-EINVAL);

	/* Allocate region. */
	if ((i = resource_alloc(&pool)) < 0)
		return (-ENFILE);

	/* Build message header. */
	message_header_build(&msg.header, SHM_CREATE);

	/* Build message.*/
	kstrcpy(msg.op.create.name, name);
	msg.op.create.mode = mode;
	msg.op.create.excl = 0;
	msg.op.create.rw = rw;
	msg.op.create.truncate = truncate;

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct shm_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct shm_message)
		) == sizeof(struct shm_message)
	);

	/* Failed to create shared memory region. */
	if (msg.header.opcode == SHM_FAIL)
	{
		resource_free(&pool, i);
		return (msg.op.ret.status);
	}

	nopen++;
	oregions[i].shmid = msg.op.ret.shmid;
	resource_set_rdonly(&oregions[i].resource);
	if (rw)
		resource_set_rdwr(&oregions[i].resource);

	return (msg.op.ret.shmid);
}

/*============================================================================*
 * __nanvix_shm_create_excl()                                                 *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_shm_create_excl(const char *name, int rw, mode_t mode)
{
	int i;
	struct shm_message msg;

	/* Uninitalized server. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid name. */
	if (nanvix_shm_is_invalid_name(name))
		return (-EINVAL);

	/* Allocate region. */
	if ((i = resource_alloc(&pool)) < 0)
		return (-ENFILE);

	/* Build message header. */
	message_header_build(&msg.header, SHM_CREATE_EXCL);

	/* Build message.*/
	kstrcpy(msg.op.create.name, name);
	msg.op.create.mode = mode;
	msg.op.create.excl = 1;
	msg.op.create.rw = rw;

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct shm_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct shm_message)
		) == sizeof(struct shm_message)
	);

	/* Failed to create shared memory region. */
	if (msg.header.opcode == SHM_FAIL)
	{
		resource_free(&pool, i);
		return (msg.op.ret.status);
	}

	nopen++;
	oregions[i].shmid = msg.op.ret.shmid;
	resource_set_rdonly(&oregions[i].resource);
	if (rw)
		resource_set_rdwr(&oregions[i].resource);

	return (msg.op.ret.shmid);
}

/*============================================================================*
 * __nanvix_shm_open()                                                        *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_shm_open(const char *name, int rw, int truncate)
{
	int i;
	struct shm_message msg;

	/* Uninitalized server. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid name. */
	if (nanvix_shm_is_invalid_name(name))
		return (-EINVAL);

	/* Truncate but cannot write. */
	if (truncate && !rw)
		return (-EINVAL);

	/* Allocate region. */
	if ((i = resource_alloc(&pool)) < 0)
		return (-ENFILE);

	/* Build message header. */
	message_header_build(&msg.header, SHM_OPEN);

	/* Build message 1.*/
	kstrcpy(msg.op.open.name, name);
	msg.op.open.rw = rw;
	msg.op.open.truncate = truncate;

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct shm_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct shm_message)
		) == sizeof(struct shm_message)
	);

	/* Failed to open shared memory region. */
	if (msg.header.opcode == SHM_FAIL)
	{
		resource_free(&pool, i);
		return (msg.op.ret.status);
	}

	i = nopen++;
	oregions[i].shmid = msg.op.ret.shmid;
	resource_set_rdonly(&oregions[i].resource);
	if (rw)
		resource_set_rdwr(&oregions[i].resource);

	return (msg.op.ret.shmid);
}

/*============================================================================*
 * __nanvix_shm_unlink()                                                      *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_shm_unlink(const char *name)
{
	int i;
	struct shm_message msg;

	/* Uninitalized server. */
	if (!server.initialized)
		return (-EAGAIN);


	/* Invalid name. */
	if (nanvix_shm_is_invalid_name(name))
		return (-EINVAL);

	/* Build message. */
	message_header_build(&msg.header, SHM_UNLINK);
	kstrcpy(msg.op.unlink.name, name);

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct shm_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct shm_message)
		) == sizeof(struct shm_message)
	);

	/* Failed to unlink shared memory region. */
	if (msg.header.opcode == SHM_FAIL)
		return (msg.op.ret.status);

	/*
	 * The process should have opened
	 * the shared memory region before.
	 */
	if ((i = shm_is_opened(msg.op.ret.shmid)) < 0)
		return (-EACCES);

	/* Release region. */
	nopen--;
	resource_free(&pool, i);

	return (0);
}

/*============================================================================*
 * __nanvix_map()                                                             *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_map(
	uint64_t *mapblk,
	size_t len,
	int writable,
	int shared,
	int fd,
	off_t off
)
{
	int i;
	struct shm_message msg;

	/* Uninitalized server. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid length. */
	if (len == 0)
		return (-EINVAL);

	/*
	 * The process should have opened
	 * the shared memory region before.
	 */
	if ((i = shm_is_opened(fd)) < 0)
		return (-EACCES);

	/* Cannot write. */
	if (writable && (!resource_is_writable(&oregions[i].resource)))
		return (-EACCES);

	/* Build message. */
	message_header_build(&msg.header, SHM_MAP);
	msg.op.map.shmid = fd;
	msg.op.map.size = len;
	msg.op.map.writable = writable;
	msg.op.map.shared = shared;
	msg.op.map.off = off;

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct shm_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct shm_message)
		) == sizeof(struct shm_message)
	);

	/* Failed to map. */
	if (msg.header.opcode == SHM_FAIL)
		return (msg.op.ret.status);

	/* Map. */
	if (!resource_is_mapped(&oregions[i].resource))
	{
		resource_set_mapped(&oregions[i].resource);
		if (shared)
			resource_set_shared(&oregions[i].resource);
	}

	*mapblk = msg.op.ret.mapblk;

	return (0);
}

/*============================================================================*
 * __nanvix_munmap()                                                          *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_unmap(int shmid, size_t len)
{
	int i;
	struct shm_message msg;

	/* Uninitalized server. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid length. */
	if (len == 0)
		return (-EINVAL);

	/*
	 * The process should have opened
	 * the shared memory region before.
	 */
	if ((i = shm_is_opened(shmid)) < 0)
		return (-EACCES);

	/* Not mapped. */
	if (!resource_is_mapped(&oregions[i].resource))
		return (-EINVAL);

	/* Build message. */
	message_header_build(&msg.header, SHM_UNMAP);
	msg.op.unmap.shmid = shmid;
	msg.op.unmap.size = len;

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct shm_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct shm_message)
		) == sizeof(struct shm_message)
	);

	/* Failed to unmap. */
	if (msg.header.opcode == SHM_FAIL)
		return (msg.op.ret.status);

	return (0);
}

/*============================================================================*
 * __nanvix_mtruncate()                                                       *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_mtruncate(int shmid, size_t size)
{
	int i;
	struct shm_message msg;

	/* Uninitalized server. */
	if (!server.initialized)
		return (-EAGAIN);

	/*
	 * The process should have opened
	 * the shared memory region before.
	 */
	if ((i = shm_is_opened(shmid)) < 0)
		return (-EACCES);

	/* Cannot write. */
	if (!resource_is_writable(&oregions[i].resource))
		return (-EINVAL);

	/* Already mapped. */
	if (shm_is_mapped(shmid))
		return (-EBUSY);

	/* Build message. */
	message_header_build(&msg.header, SHM_TRUNCATE);
	msg.op.truncate.shmid = shmid;
	msg.op.truncate.size = size;

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct shm_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct shm_message)
		) == sizeof(struct shm_message)
	);

	/* Failed to truncate. */
	if (msg.header.opcode == SHM_FAIL)
		return (msg.op.ret.status);

	return (0);
}

/*============================================================================*
 * nanvix_shm_shutdown()                                                      *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int nanvix_shm_shutdown(void)
{
	struct shm_message msg;

	/* Invalid server ID. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Build operation header. */
	message_header_build(&msg.header, SHM_EXIT);

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg, sizeof(struct shm_message)
		) == 0
	);

	return (0);
}

/*============================================================================*
 * __nanvix_shm_setup()                                                       *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_shm_setup(void)
{
	/* Nothing to do.  */
	if (server.initialized)
		return (0);

	/* Open output mailbox */
	if ((server.outbox = nanvix_mailbox_open(SHM_SERVER_NAME, SHM_SERVER_PORT_NUM)) < 0)
	{
		uprintf("[nanvix][shm] cannot open outbox to server");
		return (server.outbox);
	}

	server.initialized = true;

	return (0);
}

/*============================================================================*
 * __nanvix_shm_cleanup()                                                     *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_shm_cleanup(void)
{
	int ret;

	/* Nothing to do. */
	if (!server.initialized)
		return (0);

	/* Close output mailbox. */
	if ((ret = nanvix_mailbox_close(server.outbox)) < 0)
	{
		uprintf("[nanvix][shm] cannot close outbox to server");
		return (ret);
	}

	server.initialized = 0;

	return (0);
}
