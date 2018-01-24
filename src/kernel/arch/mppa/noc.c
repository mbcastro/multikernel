/*
 * Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#include <nanvix/ipc.h>
#include <nanvix/hal.h>
#include <nanvix/klib.h>
#include <nanvix/name.h>

#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <nanvix/noc.h>

/**
 * @brief Number of Connectors.
 */
#define NR_CONNECTOR 16

/**
 * @brief Rqueue message size (in bytes).
 */
#define RQUEUE_MSG_SIZE 96

/**
 * @brief Portal message size (in bytes).
 */
#define PORTAL_MSG_SIZE 512

/**
 * @brief NoC connectors.
 */
static struct 
{
	int fd;    /**< Underlying file descriptor. */
	int flags; /**< Flags.                      */
} connectors[NR_CONNECTOR];

/*===========================================================================*
 * nanvix_conector_flag_*                                                    *
 *===========================================================================*/

/**
 * @brief Asserts if the target NoC connector is valid.
 *
 * @param i ID of the target NoC connector.
 *
 * @returns One if the target NoC connector is valid and false otherwise.
 */
#define nanvix_connector_is_valid(i) \
	((id >= 0) && (id < NR_CONNECTORS) && !nanvix_connector_is_free(id))

/**
 * @brief Clears the flags of the target NoC connector.
 *
 * @param i ID of the target NoC connector.
 *
 * @returns One if the target NoC connector is free and zero otherwise.
 */
#define nanvix_connector_clear(i) (connectors[(i)] = 0)

/**
 * @brief Asserts if the target NoC connector is free.
 *
 * @param i ID of the target NoC connector.
 *
 * @returns One if the target NoC connector is free and zero otherwise.
 */
#define nanvix_connector_is_free(i) (connectors[(i)] & CONNECTOR_FREE)

/**
 * @brief Sets the target NoC connector as used.
 *
 * @param i ID of the target NoC connector.
 */
#define nanvix_connector_set_used(i) (connectors[(i)] &= ~CONNECTOR_FREE)

/**
 * @brief Sets the target NoC connector as free
 *
 * @param i ID of the target NoC connector.
 */
#define nanvix_connector_set_free(i) (connectors[(i)] |= CONNECTOR_FREE)

/**
 * @brief Sets the target NoC connector as read-only.
 *
 * @param i ID of the target NoC connector.
 */
#define nanvix_connector_set_input(i) (connectors[(i)] &= ~CONNECTOR_OUTPUT)

/**
 * @brief Sets the target NoC connector as write-only.
 *
 * @param i ID of the target NoC connector.
 */
#define nanvix_connector_set_output(i) (connectors[(i)] |= CONNECTOR_OUTPUT)

/**
 * @brief Sets the target NoC connector as a control NoC connector.
 *
 * @param i ID of the target NoC connector.
 */
#define nanvix_connector_set_control(i) (connectors[(i)] &= ~CONNECTOR_DATA)

/**
 * @brief Sets the target NoC connector as a data NoC connector.
 *
 * @param i ID of the target NoC connector.
 */
#define nanvix_connector_set_data(i) (connectors[(i)] |= CONNECTOR_DATA)

/*===========================================================================*
 * nanvix_connector_open()
 *===========================================================================*/

/**
 * @brief Finds a free NoC connector.
 *
 * @returns Upon successful completion, the ID of a free NoC connector is
 * returned. Upon failure, a negative error code is returned instead.
 */
static int nanvix_conector_get(void)
{
	/* Find a free conector. */
	for (int i = 0; i < NR_CONNECTOR; i++)
	{
		/* Found. */
		if (nanvix_connector_is_free(i))
		{
			nanvix_connector_set_used(i);
			return (i);
		}
	}

	return (-EAGAIN);
}

/**
 * @brief Builds a NoC connector pathname.
 *
 * @param pathname Location where to store NoC connector pathname.
 * @param rx_node  RX node.
 * @param dnoc_tag Data NoC tag.
 * @param cnoc_tag Control NoC tag.
 * @param type     NoC connector type.
 */
static void nanvix_connector_pathname(
	char *pathname,
	int rx_node,
	int dnoc_tag,
	int cnoc_tag,
	int type)
{
	/* Data connector. */
	if (flags & CONNECTOR_DATA)
		sprintf(pathname, "/mppa/portal/%d:%d", rx_node, dnoc_tag);

	/* Control connector. */
	else
	{
		char tx_nodes[32];

		/* Build TX nodes list. */
		if (rx_node == IO_CLSTER0)
			sprintf(tx_nodes, "%d..%d,%d", COMPUTE_CLUSTER0, COMPUTE_CLUSTER15, IO_CLUSTER1)
		if (rx_node == IO_CLSTER1)
			sprintf(tx_nodes, "%d..%d,%d", COMPUTE_CLUSTER0, COMPUTE_CLUSTER15, IO_CLUSTER0)
		else if (rx_node == COMPUTE_CLUSTER0)
			sprintf(tx_nodes, "%d..%d,%d,%d", COMPUTE_CLUSTER1, COMPUTE_CLUSTER15, IO_CLUSTER0, IO_CLUSTER1)
		else if (rx_node == COMPUTE_CLUSTER15)
			sprintf(tx_nodes, "%d..%d,%d,%d", COMPUTE_CLUSTER0, COMPUTE_CLUSTER14, IO_CLUSTER0, IO_CLUSTER1)
		else
			sprintf(tx_nodes, "%d..%d,%d..%d,%d,%d", COMPUTE_CLUSTER0, rx_node - 1, rx_node + 1, COMPUTE_CLUSTER15, IO_CLUSTER0, IO_CLUSTER1)

		sprintf(pathname, "/mppa/rqueue/%d:%d/[%s]:%d/%d",
				rx_node,
				dnoc_tag,
				rx_nodes,
				cnoc_tag,
				RQUEUE_SIZE);
	}
}

/**
 * @brief Opens a NoC connector.
 *
 * @param addr  NoC address.
 * @param flags Flags.
 *
 * @returns Upon successful completion, the ID of a free NoC connector is
 * returned. Upon failure, a negative error code is returned instead.
 */
int nanvix_connector_open(struct address addr, int flags)
{
	int i;                              /* Connector ID.                        */
	int mode;                           /* Connector opening mode.              */
	const char CONNECTOR_NAME_MAX = 96; /* Maximum length for a connector name. */ 
	char pathname[CONNECTOR_NAME_MAX];  /* Connector name.                      */

	/* Get a gree NoC connector. */
	if ((i = nanvix_connector_get()) < 0)
		return (-EAGAIN);

	nanvix_connector_pathname(pathname, addr.clusterid, addr.dnoc_tag, addr.cnoc_tag, flags)

	/* Data connector. */
	if (flags & CONNECTOR_DATA)
		nanvix_connector_set_data(i);

	/* Control connector. */
	else
		nanvix_connector_set_control(i);

	/* Output connector. */
	if (flags & CONNECTOR_OUTPUT)
	{
		nanvix_connector_set_output(i);
		mode = O_WRONLY;
	}

	/* Input connector. */
	else
	{
		nanvix_connector_set_input(i);
		mode = O_RDONLY;
	}

	nanvix_connector_set_used(i);
	connectors[i].fd = mppa_open(pathname, mode); 

	return (i);
}

/*===========================================================================*
 * nanvix_connector_close()
 *===========================================================================*/

/**
 * @brief Closes a NoC connector.
 *
 * @param id ID of the target NoC connector to close.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_connector_close(int id)
{
	/* Invalid NoC connector. */
	assert(nanvix_connector_is_valid(id));
	
	/* Release underlying resources. */
	nanvix_connector_clear();
	nanvix_connector_set_free(id);
	mppa_close(connectors[id].fd);

	return (0);
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
	/* Invalid NoC connector. */
	if (!nanvix_connector_is_valid(id))
		return (-EINVAL);

	assert(nanvix_connector_is_input())

	/* Rqueue */
	if (nanvix_conector_is_control(id))
		mppa_read(connectors[id].fd, buf, 1);

	/* Portal */
	else
	{
		mppa_aiocb_t aiocb;

		aiocb = MPPA_AIOCB_INITIALIZER(connectors[id].fd, buf, size);
		mppa_aio_read(&aiocb);
		mppa_aio_wait(&aiocb);
	}

	return (0);
}

/*===========================================================================*
 * nanvix_connector_send()
 *===========================================================================*/

/**
 * @brief Writes data to a NoC connector.
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
	/* Invalid NoC connector. */
	if (!nanvix_connector_is_valid(id))
		return (-EINVAL);

	assert(nanvix_connector_is_output())

	/* Rqueue */
	if (nanvix_conector_is_control(id))
		mppa_write(connectors[id].fd, bug, RQUEUE_MSG_SIZE);

	/* Portal */
	else
		mppa_pwrite(connectors[id].fd, bug, size, 0);

	return (0);
}

