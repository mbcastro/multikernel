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

/**
 * @brief Number of Connectors.
 */
#define NR_CONNECTOR 16

/**
 * @brief Opens a NoC connector.
 *
 * @param addr  NoC address.
 * @param flags Flags.
 *
 * @returns Upon successful completion, the ID of a free NoC connector is
 * returned. Upon failure, a negative error code is returned instead.
 */
void nanvix_connector_init(void)
{
	int myrank;

	myrank = mppa_get_pid();

	for (int i = 0; i < NR_CLUSTER; i++)
	{
		char pathname[128];

		sprintf(pathname, "/mppa/mppa/%d:%d", i, 16 + i);

		portals[i] = mppa_open(pathname, (i == myrank) ? O_RDONLY : O_WRONLY);
		assert(portals[i] != -1);
	}
}

/*===========================================================================*
 * nanvix_connector_receive()
 *===========================================================================*/

/**
 * @brief Reads data from a NoC connector.
 *
 * @param id   ID of the target NoC connector.
 * @param ptr  Location where to place the data.
 * @param size Number of bytes to read.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_connector_receive(int id, void *buf, size_t size)
{
	mppa_aiocb_t aiocb;

	aiocb = MPPA_AIOCB_INITIALIZER(portals[id], buf, size);
	mppa_aio_read(&aiocb);
	mppa_aio_wait(&aiocb);

	return (0);
}

/*===========================================================================*
 * nanvix_connector_send()
 *===========================================================================*/

/**
 * @brief Writes data to a process.
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
	mppa_pwrite(portals[id], buf, size, 0);

	return (0);
}

