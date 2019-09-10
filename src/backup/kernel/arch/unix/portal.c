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

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>

#define __NEED_HAL_NOC_
#define __NEED_HAL_CORE_
#define __NEED_HAL_PORTAL_
#define __NEED_HAL_PERFORMANCE_
#include <hal.h>
#include <klib.h>
#include <resource.h>


#define PORTAL_BASENAME "portal"

#define PORTAL_BUFFER_SIZE (2*1024*1024)

/**
 * @brief Portal buffer.
 */
struct portal_buffer
{
	int busy;                      /**< Busy?                          */
	int online;                    /**< Online?                        */
	int ready;                     /**< Ready?                         */
	int nbytes;                    /**< Number of bytes in the buffer. */
	char data[PORTAL_BUFFER_SIZE]; /**< Data.                          */
};

/**
 * @brief Portals
 */
struct portal
{
	struct resource resource;       /**< Generic resource information.  */
	int shm;                        /**< Portal NoC connector.          */
	int remote;                     /**< Remote NoC node ID.            */
	int local;                      /**< Local NoC node ID.             */
	size_t volume;                  /**< Amount of data transferred.    */
	uint64_t latency;               /**< Transfer latency.              */
	char pathname[128];             /**< Name of shared memory region.  */
	sem_t *lock;                    /**< Portal lock.                   */
	sem_t *locks[HAL_NR_NOC_NODES]; /**< Locks for portal buffer locks. */
	struct portal_buffer *buffers;  /**< Portal buffers.                */
} portals[HAL_NR_PORTAL];

/**
 * @brief Sync module lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Resource pool for portals.
 */
static const struct resource_pool pool = {
	portals,
	HAL_NR_PORTAL,
	sizeof(struct portal)
};

/*============================================================================*
 * unix_portal_lock()                                                         *
 *============================================================================*/

/**
 * @brief Locks Unix portal module.
 */
static void unix_portal_lock(void)
{
	pthread_mutex_lock(&lock);
}

/*============================================================================*
 * unix_portal_unlock()                                                       *
 *============================================================================*/

/**
 * @brief Unlocks Unix portal module.
 */
static void unix_portal_unlock(void)
{
	pthread_mutex_unlock(&lock);
}

/*============================================================================*
 * portal_is_valid()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a portal is valid.
 *
 * @param portalid ID of the target portal.
 *
 * @returns One if the target portal is valid, and false
 * otherwise.
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
static int portal_is_valid(int portalid)
{
	return ((portalid >= 0) && (portalid < HAL_NR_PORTAL));
}

/**
 * @brief Builds the pathname of a portal buffer lock.
 *
 * @param pathname Place where the pathname should be stored.
 * @param portalid ID of the target portal.
 * @param bufferid ID of the target buffer.
 */
static inline void portal_buffer_lock_name(char *pathname, int portalid, int bufferid)
{
	sprintf(pathname,
		"%s-%d",
		portals[portalid].pathname,
		bufferid
	);
}

/**
 * @brief Initializes a portal buffer.
 *
 * @param portalid ID of the target portal.
 * @param bufferid ID of the target buffer.
 */
static inline void portal_buffer_init(int portalid, int bufferid)
{
	char pathname[128];

	portal_buffer_lock_name(pathname, portalid, bufferid);

	/* Create and initialize portal buffer lock. */
	KASSERT((portals[portalid].locks[bufferid] = 
		sem_open(pathname,
			O_RDWR | O_CREAT,
			S_IRUSR | S_IWUSR,
			1)
		) != NULL
	);

	portals[portalid].buffers[bufferid].online = 0;
	portals[portalid].buffers[bufferid].ready = 0;
	portals[portalid].buffers[bufferid].busy = 0;
}

/**
 * @brief Locks a portal buffer.
 *
 * @param portalid ID of the target portal.
 * @param bufferid ID of the target buffer.
 */
static inline void portal_buffer_lock(int portalid, int bufferid)
{
	KASSERT(sem_wait(portals[portalid].locks[bufferid]) != -1);
}

/**
 * @brief Unlocks a portal buffer.
 *
 * @param portalid ID of the target portal.
 * @param bufferid ID of the target buffer.
 */
static inline void portal_buffer_unlock(int portalid, int bufferid)
{
	KASSERT(sem_post(portals[portalid].locks[bufferid]) != -1);
}

/**
 * @brief Writes data to a portal buffer.
 *
 * @param portalid ID of the target portal.
 * @param bufferid ID of the target buffer.
 * @param ptr      Target data.
 * @param n        Number of bytes to write.
 */
static inline void portal_buffer_write(int portalid, int bufferid, const void *ptr, size_t n)
{
	((void) portalid);
	((void) bufferid);
	((void) ptr);
	((void) n);

	/* TODO */
}

/**
 * @brief Reads data from a portal buffer.
 *
 * @param portalid ID of the target portal.
 * @param bufferid ID of the target buffer.
 * @param ptr    Target data.
 * @param n      Number of bytes to write.
 */
static inline void portal_buffer_read(int portalid, int bufferid, void *ptr, size_t n)
{
	((void) portalid);
	((void) bufferid);
	((void) ptr);
	((void) n);

	/* TODO */
}

/**
 * @brief Initializes the lock of a portal.
 *
 * @param portalid ID of the target portal.
 */
static void portal_lock_init(int portalid)
{
	/* Create and initialize portal buffer lock. */
	KASSERT((portals[portalid].lock = 
		sem_open(portals[portalid].pathname,
			O_RDWR | O_CREAT,
			S_IRUSR | S_IWUSR,
			1)
		) != NULL
	);
}

/**
 * @brief Destroys the lock of a portal.
 *
 * @param portalid ID of the target portal.
 */
static void portal_lock_destroy(int portalid)
{
	KASSERT(sem_unlink(portals[portalid].pathname) != -1);
}

/**
 * @brief Closes the lock of a portal.
 *
 * @param portalid ID of the target portal.
 */
static void portal_lock_close(int portalid)
{
	KASSERT(sem_close(portals[portalid].lock) != -1);
}

/**
 * @brief Locks a portal.
 *
 * @param portalid ID of the target portal.
 */
static inline void portal_lock(int portalid)
{
	KASSERT(sem_wait(portals[portalid].lock) != -1);
}

/**
 * @brief Unlocks a portal.
 *
 * @param portalid ID of the target portal.
 */
static inline void portal_unlock(int portalid)
{
	KASSERT(sem_post(portals[portalid].lock) != -1);
}

/*============================================================================*
 * hal_portal_create()                                                        *
 *============================================================================*/

/**
 * @brief See hal_portal_create().
 */
static int unix_portal_create(int local)
{
	int shm;        /* Portal NoC Connector.       */
	void *p;        /* Pointer to portal buffers.  */
	int portalid;   /* ID of  portal               */
	char *pathname; /* NoC connector name.         */

	/* Check if portal was already created. */
	for (int i = 0; i < HAL_NR_PORTAL; i++)
	{
		/* Skip invalid portals. */
		if (!resource_is_used(&portals[i].resource))
			continue;

		/* Skip output portals. */
		if (!resource_is_readable(&portals[i].resource))
			continue;

		/* Exists. */
		if (portals[i].local == local)
			return (-EEXIST);
	}

	/* Allocate portal. */
	if ((portalid = resource_alloc(&pool)) < 0)
		goto error0;

	pathname = portals[portalid].pathname;

	/* Build pathname for portal. */
	sprintf(pathname,
		"%s-%d",
		PORTAL_BASENAME,
		local
	);

	portal_lock_init(portalid);

	portal_lock(portalid);

		/* Create portal buffers. */
		KASSERT((shm =
			shm_open(pathname,
				O_RDWR | O_CREAT,
				S_IRUSR | S_IWUSR)
			) != -1
		);

		/* Allocate portal buffers. */
		KASSERT(ftruncate(shm,
					HAL_NR_NOC_NODES*sizeof(struct portal_buffer)
				) != -1
		);

		/* Attach portal buffers. */
		KASSERT((p = 
			mmap(NULL,
				HAL_NR_NOC_NODES*sizeof(struct portal_buffer),
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				shm,
				0)
			) != NULL
		);

		/* Initialize portal. */
		portals[portalid].buffers = p;
		portals[portalid].shm = shm;
		portals[portalid].remote = -1;
		portals[portalid].local = local;
		portals[portalid].latency = 0;
		portals[portalid].volume = 0;
		resource_set_rdonly(&portals[portalid].resource);
		resource_set_notbusy(&portals[portalid].resource);
		for (int i = 0; i < HAL_NR_NOC_NODES; i++)
			portal_buffer_init(portalid, i);

	portal_unlock(portalid);

	return (portalid);

error0:
	return (-EAGAIN);
}

/**
 * @brief Creates a portal.
 *
 * @param local ID of the local NoC node.
 *
 * @returns Upon successful completion, the ID of a newly created
 * portal is returned. Upon failure, a negative error code is returned
 * instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_portal_create(int local)
{
	int portalid;

	/* Invalid local NoC node. */
	if (local < -1)
		return (-EINVAL);

	/* Bad local NoC node. */
	if (local != hal_get_node_id())
		return (-EINVAL);

	unix_portal_lock();
		portalid = unix_portal_create(local);
	unix_portal_unlock();

	return (portalid);
}

/*============================================================================*
 * hal_portal_allow()                                                         *
 *============================================================================*/

/**
 * @brief Enables read operations from a remote.
 *
 * @param portalid ID of the target portal.
 * @param remote   NoC node ID of target remote.
 *
 * @returns Upons successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_portal_allow(int portalid, int remote)
{
	int local;
	int nodenum;

	/* Invalid portal.*/
	if (!portal_is_valid(portalid))
		return (-EINVAL);

	local = hal_get_node_id();

	/* Invalid remote. */
	if (remote == local)
		goto error0;

	/* Invalid remote. */
	if (remote < 0)
		goto error0;

again:

	unix_portal_lock();

		/* Bad portal. */
		if (!resource_is_used(&portals[portalid].resource))
			goto error1;

		/* Bad portal. */
		if (!resource_is_readable(&portals[portalid].resource))
			goto error1;

		/* Busy portal. */
		if (resource_is_busy(&portals[portalid].resource))
		{
			unix_portal_unlock();
			goto again;
		}

		/* Read operation is ongoing. */
		if (portals[portalid].remote != -1)
		{
			unix_portal_unlock();
			goto again;
		}

		portal_lock(portalid);

		nodenum = hal_get_node_num(remote);

		/* Device is not online. */
		if (!portals[portalid].buffers[nodenum].online)
			goto error2;

		/* Device is busy. */
		if (portals[portalid].buffers[nodenum].busy)
			goto error2;

		/* Device is ready. */
		if (portals[portalid].buffers[nodenum].ready)
			goto error2;

		portals[portalid].remote = remote;
		portals[portalid].buffers[nodenum].ready = 1;

	portal_unlock(portalid);
	unix_portal_unlock();

	return (0);

error2:
	portal_unlock(portalid);
error1:
	unix_portal_unlock();
error0:
	return (-EINVAL);
}

/*============================================================================*
 * hal_portal_open()                                                          *
 *============================================================================*/

/**
 * @brief See hal_portal_open().
 */
static int unix_portal_open(int local, int remote)
{
	void *p;        /* Pointer to portal buffers. */
	int shm;        /* Portal NoC Connector.      */
	int error;      /* Error.                     */
	int nodenum;    /* Uuderlying node number.    */
	int portalid;   /* ID of  portal              */
	char *pathname; /* NoC connector name.        */

	/* Check if output portal is already opened. */
	for (int i = 0; i < HAL_NR_PORTAL; i++)
	{
		/* Skip invalid portals. */
		if (!resource_is_used(&portals[i].resource))
			continue;

		/* Skip output portals. */
		if (!resource_is_writable(&portals[i].resource))
			continue;

		/* Skip invalid remotes. */
		if (portals[i].remote != remote)
			continue;

		/* Exists. */
		if (portals[i].local == local)
			return (-EEXIST);
	}

	/* Allocate portal. */
	if ((portalid = resource_alloc(&pool)) < 0)
	{
		error = -EAGAIN;
		goto error0;
	}

	pathname = portals[portalid].pathname;

	/* Build pathname for portal. */
	sprintf(pathname,
		"%s-%d",
		PORTAL_BASENAME,
		remote	
	);

	/* Open portal lock. */
	if ((portals[portalid].lock = sem_open(pathname, O_RDWR)) == NULL)
	{
		error = -ENOENT;
		goto error1;
	}

	/* Lock portal. */
	KASSERT(sem_wait(portals[portalid].lock) != -1);

		/* TODO: check if the portal has not been closed. */

		/* Open portal buffers. */
		KASSERT((shm =
			shm_open(pathname,
				O_RDWR,
				0
			)
		) != -1);

		/* Attach portal buffers. */
		KASSERT((p =
			mmap(NULL,
				HAL_NR_NOC_NODES*sizeof(struct portal_buffer),
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				shm,
				0)
			) != NULL
		);

		nodenum = hal_get_node_num(local);

		/* Initialize portal. */
		portals[portalid].buffers = p;
		portals[portalid].shm = shm;
		portals[portalid].remote = remote;
		portals[portalid].local = local;
		portals[portalid].latency = 0;
		portals[portalid].volume = 0;
		resource_set_wronly(&portals[portalid].resource);
		resource_set_notbusy(&portals[portalid].resource);
		portals[portalid].buffers[nodenum].online = 1;

	/* Unlock portal. */
	portal_unlock(portalid);

	return (portalid);

error1:
	resource_free(&pool, portalid);
error0:
	return (error);
}

/**
 * @brief Opens a portal.
 *
 * @param remote ID of the target NoC node.
 *
 * @returns Upon successful completion, the ID of the target portal is
 * returned. Upon failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_portal_open(int remote)
{
	int local;
	int portalid;

	/* Invalid node ID. */
	if (remote < 0)
		return (-EINVAL);

	local = hal_get_node_id();

	/* Bad remote. */
	if (remote == local)
		return (-EINVAL);

	unix_portal_lock();
		portalid = unix_portal_open(local, remote);
	unix_portal_unlock();

	return (portalid);
}

/*============================================================================*
 * hal_portal_wait()                                                          *
 *============================================================================*/

/**
 * @brief Waits for an asynchronous operation on a portal to complete.
 *
 * @param portalid ID of target portal.
 *
 * @returns Upon successful completion, the number of bytes
 * read/written is returned. Upon failure, a negative error code is
 * returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
ssize_t hal_portal_wait(int portalid)
{
	((void) portalid);

	return (0);
}

/*============================================================================*
 * hal_portal_aread()                                                         *
 *============================================================================*/

/**
 * @brief Reads data asynchronously from a portal.
 *
 * @param portalid ID of target portal.
 * @param buf      Location from where data should be written.
 * @param n        Number of bytes to read.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_portal_aread(int portalid, void *buf, size_t n)
{
	((void) portalid);
	((void) buf);
	((void) n);

	return (0);
}

/*============================================================================*
 * hal_portal_read()                                                          *
 *============================================================================*/

/**
 * @brief Reads data from a portal.
 *
 * @param portalid ID of target portal.
 * @param buf      Location from where data should be written.
 * @param n        Number of bytes to read.
 *
 * @returns Upon successful completion, the number of bytes read is
 * returned. Upon failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
ssize_t hal_portal_read(int portalid, void *buf, size_t n)
{
	int nread;
	int nodenum;

	/* Invalid portal ID.*/
	if (!portal_is_valid(portalid))
		goto error0;

	/* Invalid buffer. */
	if (buf == NULL)
		goto error0;

	/* Invalid read size. */
	if (n < 1)
		goto error0;

again:

	unix_portal_lock();

		/* Bad portal. */
		if (!resource_is_used(&portals[portalid].resource))
			goto error1;

		/* Bad portal. */
		if (!resource_is_readable(&portals[portalid].resource))
			goto error1;

		/* Busy portal. */
		if (resource_is_busy(&portals[portalid].resource))
		{
			unix_portal_unlock();
			goto again;
		}

		/* No read operation is ongoing. */
		if (portals[portalid].remote == -1)
			goto error1;

		portal_lock(portalid);

		nodenum = hal_get_node_num(portals[portalid].remote);

		/* Device should be online. */
		KASSERT(portals[portalid].buffers[nodenum].online);

		/* No data is available. */
		if (!portals[portalid].buffers[nodenum].busy)
		{
			portal_unlock(portalid);
			unix_portal_unlock();
			goto again;
		}

		/* Set portal as busy. */
		resource_set_busy(&portals[portalid].resource);

	/*
	 * Release lock, since we may sleep below.
	 */
	unix_portal_unlock();

	memcpy(buf, portals[portalid].buffers[nodenum].data, nread = n);

	portals[portalid].remote = -1;
	portals[portalid].buffers[nodenum].busy = 0;

	portal_unlock(portalid);
	
	unix_portal_lock();
		resource_set_notbusy(&portals[portalid].resource);
	unix_portal_unlock();

	return (nread);

error1:
	unix_portal_unlock();
error0:
	return (-EINVAL);
}

/*============================================================================*
 * hal_portal_awrite()                                                        *
 *============================================================================*/

/**
 * @brief Writes data asynchronously to a portal.
 *
 * @param portalid ID of target portal.
 * @param buf      Location from where data should be read.
 * @param n        Number of bytes to write.
 *
 * @returns Upon successful, zero is returned. Upon failure, a
 * negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_portal_awrite(int portalid, const void *buf, size_t n)
{
	((void) portalid);
	((void) buf);
	((void) n);

	return (0);
}

/*============================================================================*
 * hal_portal_write()                                                         *
 *============================================================================*/

/**
 * @brief Writes data to a portal.
 *
 * @param portalid ID of target portal.
 * @param buf      Location from where data should be read.
 * @param n        Number of bytes to write.
 *
 * @returns Upon successful the number of bytes written is returned.
 * Upon failure, a negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
ssize_t hal_portal_write(int portalid, const void *buf, size_t n)
{
	int nwrite;
	int nodenum;

	/* Invalid portal ID.*/
	if (!portal_is_valid(portalid))
		goto error0;

	/* Invalid buffer. */
	if (buf == NULL)
		goto error0;

	/* Invalid write size. */
	if (n < 1)
		goto error0;

again:
	
	unix_portal_lock();

		/* Bad portal. */
		if (!resource_is_used(&portals[portalid].resource))
			goto error1;

		/* Bad portal. */
		if (!resource_is_writable(&portals[portalid].resource))
			goto error1;

		/* Busy portal. */
		if (resource_is_busy(&portals[portalid].resource))
		{
			unix_portal_unlock();
			goto again;
		}

		portal_lock(portalid);

		nodenum = hal_get_node_num(portals[portalid].local);

		/* Device is not online. */
		if (!portals[portalid].buffers[nodenum].online)
			goto error2;

		/* Device is not ready. */
		if (!portals[portalid].buffers[nodenum].ready)
		{
			portal_unlock(portalid);
			unix_portal_unlock();
			goto again;
		}

		/* Set portal as busy. */
		resource_set_busy(&portals[portalid].resource);

	/*
	 * Release lock, since we may sleep below.
	 */
	unix_portal_unlock();

	memcpy(portals[portalid].buffers[nodenum].data, buf, nwrite = n);
	portals[portalid].buffers[nodenum].ready = 0;
	portals[portalid].buffers[nodenum].busy = 1;

	portal_unlock(portalid);
	
	unix_portal_lock();
		resource_set_notbusy(&portals[portalid].resource);
	unix_portal_unlock();

	return (nwrite);

error2:
	portal_unlock(portalid);
error1:
	unix_portal_unlock();
error0:
	return (-EINVAL);
}

/*============================================================================*
 * hal_portal_close()                                                         *
 *============================================================================*/

/**
 * @brief Closes a portal.
 *
 * @param portalid ID of target portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_portal_close(int portalid)
{
	int nodenum;

	/* Invalid portal. */
	if (!portal_is_valid(portalid))
		goto error0;

again:

	unix_portal_lock();

		nodenum = hal_get_node_num(portals[portalid].local);

		/* Bad portal. */
		if (!resource_is_used(&portals[portalid].resource))
			goto error1;

		/* Bad portal. */
		if (resource_is_readable(&portals[portalid].resource))
			goto error1;

		/* Busy portal. */
		if (resource_is_busy(&portals[portalid].resource))
		{
			unix_portal_unlock();
			goto again;
		}

		/*
		 * We should lock the portal to prevent a
		 * concurrent unlink operation.
		 */
		portal_lock(portalid);	

			portals[portalid].buffers[nodenum].online = 0;

			/* Detach portal buffer. */
			KASSERT(munmap(portals[portalid].buffers,
						HAL_NR_NOC_NODES*sizeof(struct portal_buffer)
					) != -1
			);

		portal_unlock(portalid);

		portal_lock_close(portalid);
		resource_free(&pool, portalid);

	unix_portal_unlock();

	return (0);

error1:
	unix_portal_unlock();
error0:
	return (-EAGAIN);
}

/*============================================================================*
 * hal_portal_unlink()                                                        *
 *============================================================================*/

/**
 * @brief Destroys a portal.
 *
 * @param portalid ID of the target portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 * @note This function is reentrant.
 */
int hal_portal_unlink(int portalid)
{
	/* Invalid portal. */
	if (!portal_is_valid(portalid))
		goto error0;

again:

	unix_portal_lock();

		/* Bad portal. */
		if (!resource_is_used(&portals[portalid].resource))
			goto error1;

		/* Bad portal. */
		if (resource_is_writable(&portals[portalid].resource))
			goto error1;

		/* Busy portal. */
		if (resource_is_busy(&portals[portalid].resource))
		{
			unix_portal_unlock();
			goto again;
		}

		portal_lock(portalid);

			/* Check if the portal is busy. */
			for (int i = 0; i < HAL_NR_NOC_NODES; i++)
			{
				/*
				 * Some other process is using this portal.
				 * So release locks and try again.
				 */
				if (portals[portalid].buffers[i].online)
				{
					portal_unlock(portalid);
					unix_portal_unlock();
					goto again;
				}
			}

			/* Detach portal buffers. */
			KASSERT(munmap(portals[portalid].buffers,
						HAL_NR_NOC_NODES*sizeof(struct portal_buffer)
					) != -1
			);

			/* Destroy portal buffers. */
			KASSERT(shm_unlink(portals[portalid].pathname) != -1);

			/* Close portal lock. */
			portal_lock_close(portalid);

		/* Destroy portal lock. */
		portal_lock_destroy(portalid);

		resource_free(&pool, portalid);

	unix_portal_unlock();

	return (0);

error1:
	unix_portal_unlock();
error0:
	return (-EAGAIN);
}

/*============================================================================*
 * hal_portal_ioctl()                                                         *
 *============================================================================*/

/**
 * @brief Performs control operations in a portal.
 *
 * @param portalid Target portal.
 * @param request  Request.
 * @param args     Additional arguments.
 *
 * @param Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int hal_portal_ioctl(int portalid, unsigned request, va_list args)
{
	((void) portalid);
	((void) request);
	((void) args);

	return (0);
}
