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

#define CONFIGURE_MAXIMUM_POSIX_THREADS 4

#include <mppa/osconfig.h>
#include <nanvix/hal.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Number of DMA engines.
 */
#define NR_DMA CONFIGURE_MAXIMUM_POSIX_THREADS

/**
 * @brief Remote memory.
 */
static char rmem[RMEM_SIZE];

/**
 * @brief Handles remote memory requests 
 */
static void rmem_server(void *args)
{
	int dma;           /* DMA channel to use. */
	char pathname[16]; /*  */
	int inbox;         /* Mailbox for small messages. */
	int inportal;      /* Portal for data transfers.  */

	dma = ((int *)args)[0];

	sprintf(pathname, "/rmem%d", dma);
	inbox = mailbox_create(pathname);
	inportal = portal_create(pathname);

	while(1)
	{
		struct rmem_message msg;

		mailbox_read(inbox, &msg);

		/* handle write operation. */
		if (msg.op == RMEM_WRITE)
		{
			portal_allow(inportal, msg.source);
			portal_read(inportal, &rmem[msg.blknum], msg.size);
		}

		/* Handle read operation. */
		else if (msg.op == RMEM_READ)
		{
			int outportal = portal_open(name_cluster_name(msg.source));
			portal_write(outportal, &rmem[msg.blknum], msg.size);
			portal_close(outportal);
		}
	}

	portal_unlink(inportal);
	mailbox_unlink(inbox);
}

/**
 * @brief Remote memory server.
 */
int main(int argc, char **argv)
{
	int ndmas;              /* Number of dmas to use.      */
	int dmas[NR_DMA]        /* DMA IDs.                    */
	pthread_t tids[NR_DMA]; /* Thread IDs.                 */

	/* Missing parameters. */
	if (arch < 2)
	{
		printf("missing parameters");
		printf("Usage: rmem <num. dmas>");
		return (-1);
	}

	ndmas = atoi(argv[1]);
	assert((ndmas >= 1) && (ndmas <= NR_DMA));

#ifdef DEBUG
	printf("[RMEM] booting up server\n");
#endif

#ifdef DEBUG
	printf("[RMEM] server alive\n");
#endif

	/* Spawn RMEM server threads. */
	for (i = 0; i < ndmas; i++)
	{
		dmas[i] = i;
		assert((pthread_create(&tids[i],
			NULL,
			rmem_server,
			&dmas[i])) == 0
		);
	}

	/* Release master IO cluster. */
	barrier_open(NR_IOCLUSTER);
	barrier_release();

	/* Wait for RMEM server threads. */
	for (i = 0; i < ndmas; i++)
		pthread_join(tids[i]. NULL);

	/* House keeping. */
	barrier_close();

	return (EXIT_SUCCESS);
}
