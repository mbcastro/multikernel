/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

/**
 * @brief Portal NoC connectors.
 */
static int portals[NR_CCLUSTER];

/**
 * @brief Cluster ID of current process.
 */
static int myrank;

/**
 * @brief Initializes NoC connectors.
 */
void nanvix_connector_init(void)
{
	myrank = mppa_getpid();

	printf("Hello from %d\n", myrank);

	/* Open NoC Connectors. */
	for (int i = 0; i < NR_CCLUSTER; i++)
	{
		char pathname[128];

		sprintf(pathname, "/mppa/portal/%d:%d", i, 16 + i);

		portals[i] = mppa_open(pathname, (i == myrank) ? O_RDONLY : O_WRONLY);
		assert(portals[i] != -1);
	}
	printf("Hello from %d: portals are opened\n", myrank);
}

/*===========================================================================*
 * nanvix_connector_receive()
 *===========================================================================*/

/**
 * @brief Reads data from a cluster.
 *
 * @param ptr  Location where to place the data.
 * @param size Number of bytes to read.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_connector_receive(void *buf, size_t size)
{
	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid size. */
	if (size < 1)
		return (-EINVAL);

	mppa_aiocb_t aiocb = MPPA_AIOCB_INITIALIZER(portals[myrank], buf, size);
	int ret2 = mppa_aio_read(&aiocb);
	printf("mpaa_aio_read() %d/%d\n", ret2, size);
	int ret = mppa_aio_wait(&aiocb);
	printf("mppa_aio_wait() %d/%d\n", ret,size);

	return (0);
}

/*===========================================================================*
 * nanvix_connector_send()
 *===========================================================================*/

/**
 * @brief Writes data to a cluster.
 *
 * @param id   ID of the target NoC connector.
 * @param ptr  Location where to read data from.
 * @param size Number of bytes to write.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_connector_send(int id, const void *buf, size_t size)
{
	/* Invalid cluster ID. */
	if (id == myrank)
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid size. */
	if (size < 1)
		return (-EINVAL);

	int ret = mppa_pwrite(portals[id], buf, size, 0);

	printf("mppa_pwrite() %d\n", ret);

	return (0);
}

