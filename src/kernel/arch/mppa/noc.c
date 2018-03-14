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
#include <nanvix/hal.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

/**
 * @brief Magic number for NoC packets.
 */
#define NOC_PACKET_MAGIC 0xc001f00l

/**
 * @brief Size (in bytes) of NoC packet's payload.
 */
#define NOC_PACKET_PAYLOAD_SIZE 128

/**
 * @brief NoC packet.
 */
struct noc_packet
{
	unsigned magic;                        /**< Magic header.         */
	unsigned source;                       /**< Cluster ID of source. */
	char payload[NOC_PACKET_PAYLOAD_SIZE]; /**< Payload.              */
};

/**
 * @brief Portal NoC connectors.
 */
static int portals[NR_CCLUSTER];

/**
 * @brief Initializes NoC connectors.
 *
 * @parm ncclusters Number of compute clusters to initialize.
 */
void nanvix_noc_init(int ncclusters)
{
	int myrank;

	myrank = arch_get_cluster_id();

	/* Open NoC Connectors. */
	for (int i = 0; i < ncclusters; i++)
	{
		char pathname[128];

		sprintf(pathname, "/mppa/portal/%d:%d", i, 16 + i);

		portals[i] = mppa_open(pathname, (i == myrank) ? O_RDONLY : O_WRONLY);
		assert(portals[i] != -1);
	}
}

/**
 * @brief Reads data from the local NoC buffer.
 *
 * @param buf  Location where data should be written to.
 *
 * @returns Upon successful completion the cluster ID of the remote source is
 * returned. Upon failure, a negative error code is returned instead.
 */
int nanvix_noc_receive(void *buf)
{
	static const int ntries = 3;                   /**< Number of tries.   */
	static struct noc_packet buffers[NR_CCLUSTER]; /**< Local NoC buffers. */

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid size. */
	if (size < 1)
		return (-EINVAL);

	mppa_aiocb_t aiocb = MPPA_AIOCB_INITIALIZER(
			portals[arch_get_cluster_id()],
			buffers, sizeof(struct noc_packet)
	);

	/* Try some times. */
	for (int j = 0; j < ntries; j++)
	{
		/* Check for any message, */
		for (int i = 0; i < NR_CCLUSTER; i++)
		{
			/* Found. */
			if (buffers[i].magic == NOC_PACKET_MAGIC)
			{
				buffers[i].magic = 0;
				memcpy(buf, &buf, NOC_PACKET_PAYLOAD_SIZE);

				return (buffers[i].source);
			}
		}

		assert(mppa_aio_read(&aiocb) == 0);
		assert(mppa_aio_wait(&aiocb) == sizeof(struct noc_packet));
	}

	return (-EAGAIN);
}

/**
 * @brief Writes data to a remote NoC connector.
 *
 * @param id  ID of the destination cluster.
 * @param buf Location where data should be read from.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_noc_send(int id, const void *buf)
{
	int myrank;               /* Cluster ID of the current process. */
	struct noc_packet packet; /* Packet to send.                    */

	/* Invalid cluster ID. */
	if ((myrank = arch_get_cluster_id()) == id)
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid size. */
	if (size < 1)
		return (-EINVAL);

	/* Build NoC packet. */
	packet.magic = NOC_PACKET_MAGIC;
	packet.source = myrank;
	memcpy(packet.buffer, &buf, NOC_PACKET_PAYLOAD_SIZE);
	mppa_pwrite(portals[id], &packet, sizeof(struct noc_packet), myrank*sizeof(struct noc_packet));

	return (0);
}

