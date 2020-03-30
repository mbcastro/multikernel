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
#define __SHM_SERVER

#include <nanvix/servers/shm.h>
#include <nanvix/runtime/name.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/sys/semaphore.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/noc.h>
#include <nanvix/ulib.h>

/**
 * @brief SHM Server information.
 */
static struct
{
	int nodenum;  /**< Node number.  */
	int inbox;    /**< Input mailbox */
	int inportal; /**< Input Portal  */
	const char *name;
} server = {
	-1, -1, -1, SHM_SERVER_NAME
};

/*============================================================================*
 * do_open()                                                                  *
 *============================================================================*/

/**
 * @brief Handles an open request.
 */
static int do_open(struct shm_message *request, struct shm_message *response)
{
	int ret;

	ret = __do_shm_open(
		request->header.source,
		request->op.open.name,
		request->op.open.rw,
		request->op.open.truncate
	);

	if (ret < 0)
		return (ret);

	response->op.ret.shmid = ret;

	return (0);
}

/*============================================================================*
 * do_create()                                                                *
 *============================================================================*/

/**
 * @brief Handles a create request.
 */
static int do_create(struct shm_message *request, struct shm_message *response)
{
	int ret;

	ret = __do_shm_create(
		request->header.source,
		request->op.create.name,
		request->op.create.rw,
		request->op.create.mode
	);

	if (ret < 0)
		return (ret);

	response->op.ret.shmid = ret;

	return (0);
}

/*============================================================================*
 * do_create_excl()                                                           *
 *============================================================================*/

/**
 * @brief Handles an exclusive create request.
 */
static int do_create_excl(struct shm_message *request, struct shm_message *response)
{
	int ret;

	ret = __do_shm_create_exclusive(
		request->header.source,
		request->op.create.name,
		request->op.create.rw,
		request->op.create.mode
	);

	if (ret < 0)
		return (ret);

	response->op.ret.shmid = ret;

	return (0);
}

/*============================================================================*
 * do_unlink()                                                                *
 *============================================================================*/

/**
 * @brief Handles an unlink request.
 */
static int do_unlink(struct shm_message *request, struct shm_message *response)
{
	int ret;

	ret = __do_shm_unlink(request->header.source, request->op.unlink.name);

	if (ret < 0)
		return (ret);

	response->op.ret.shmid = ret;

	return (0);
}

/*============================================================================*
 * do_map()                                                                   *
 *============================================================================*/

/**
 * @brief Handles a map request.
 */
static int do_map(struct shm_message *request, struct shm_message *response)
{
	int ret;
	uint64_t mapblk;

	ret = __do_shm_map(
		request->header.source,
		request->op.map.shmid,
		request->op.map.size,
		request->op.map.writable,
		request->op.map.shared,
		request->op.map.off,
		&mapblk
	);

	if (ret < 0)
		return (ret);

	response->op.ret.mapblk = mapblk;

	return (0);
}

/*============================================================================*
 * do_unmap()                                                                 *
 *============================================================================*/

/**
 * @brief Handles an unmap request.
 */
static int do_unmap(struct shm_message *request)
{
	int ret;

	ret = __do_shm_unmap(request->header.source, request->op.unmap.shmid);

	return (ret);
}

/*============================================================================*
 * do_truncate()                                                              *
 *============================================================================*/

/**
 * @brief Handles a truncate request.
 */
static int do_truncate(struct shm_message *request)
{
	int ret;

	ret = __do_shm_truncate(
		request->header.source,
		request->op.truncate.shmid,
		request->op.truncate.size
	);

	return (ret);
}

/*============================================================================*
 * shm_loop()                                                                 *
 *============================================================================*/

/**
 * @brief Handles shared memory region requests.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_shm_loop(void)
{
	int shutdown = 0;

	while (!shutdown)
	{
		int outbox;
		int reply = 0;
		int ret = -ENOSYS;
		struct shm_message request;
		struct shm_message response;

		uassert(
			kmailbox_read(
				server.inbox,
				&request,
				sizeof(struct shm_message)
			) == sizeof(struct shm_message)
		);

		shm_debug("shm request source=%d port=%d opcode=%d",
			request.header.source,
			request.header.portal_port,
			request.header.opcode
		);

		/* TODO check for bad node number. */

		/* Handle request. */
		switch (request.header.opcode)
		{
			case SHM_CREATE:
				ret = do_create(&request, &response);
				reply = 1;
				break;

			case SHM_CREATE_EXCL:
				ret = do_create_excl(&request, &response);
				reply = 1;
				break;

			case SHM_OPEN:
				ret = do_open(&request, &response);
				reply = 1;
				break;

			case SHM_UNLINK:
				ret = do_unlink(&request, &response);
				reply = 1;
				break;

			case SHM_MAP:
				break;

			case SHM_UNMAP:
				break;

			case SHM_TRUNCATE:
				ret = do_truncate(&request);
				reply = 1;
				break;

			case SHM_EXIT:
				shutdown = 1;
				break;

			default:
				break;
		}

		/* No reply? */
		if (!reply)
			continue;

		response.op.ret.status = ret;
		message_header_build(
			&response.header,
			(ret < 0) ? SHM_FAIL : SHM_SUCCESS
		);

		uassert((
			outbox = kmailbox_open(
				request.header.source,
				request.header.mailbox_port
			)) >= 0
		);
		uassert(
			kmailbox_write(
				outbox,
				&response,
				sizeof(struct shm_message
			)) == sizeof(struct shm_message)
		);
		uassert(kmailbox_close(outbox) == 0);
	}

	return (0);
}

/*============================================================================*
 * do_shm_startup()                                                           *
 *============================================================================*/

/**
 * @brief Initializes the shared memory server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_shm_startup(struct nanvix_semaphore *lock)
{
	int ret;

	uprintf("[nanvix][shm] booting up server");

	server.nodenum = knode_get_num();
	server.inbox = stdinbox_get();
	server.inportal = stdinportal_get();

	/* Link name. */
	if ((ret = name_link(server.nodenum, server.name)) < 0)
		return (ret);

	shm_init();

	uprintf("[nanvix][shm] server alive");
	uprintf("[nanvix][shm] attached to node %d", server.nodenum);
	uprintf("[nanvix][shm] listening to mailbox %d", server.inbox);
	uprintf("[nanvix][shm] listening to portal %d", server.inportal);

	nanvix_semaphore_up(lock);

	return (0);
}

/*============================================================================*
 * do_shm_shutdown()                                                          *
 *============================================================================*/

/**
 * @brief Shutdowns the shared memory server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_shm_shutdown(void)
{
	uprintf("[nanvix][shm] shutting down server");

	return (0);
}

/*============================================================================*
 * do_shm_server()                                                            *
 *============================================================================*/

/**
 * @brief Shared memory server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_shm_server(struct nanvix_semaphore *lock)
{
	int ret;

	if ((ret = do_shm_startup(lock)) < 0)
	{
		uprintf("[nanvix][shm] failed to startup server!");
		goto error;
	}

	if ((ret = do_shm_loop()) < 0)
	{
		uprintf("[nanvix][shm] failed to launch server!");
		goto error;
	}

	if ((ret = do_shm_shutdown()) < 0)
	{
		uprintf("[nanvix][shm] failed to shutdown server!");
		goto error;
	}

	return (0);

error:
	return (ret);
}

/*============================================================================*
 * shm_server()                                                               *
 *============================================================================*/

/**
 * @brief Handles shared memory requests.
 *
 * @returns Always returns zero.
 */
int shm_server(struct nanvix_semaphore *lock)
{
	uassert(do_shm_server(lock) == 0);

	return (0);
}
