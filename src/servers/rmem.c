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

#include <mppa/osconfig.h>
#include <nanvix/hal.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Remote memory.
 */
static char rmem[RMEM_SIZE];

static pthread_barrier_t barrier;

static pthread_mutex_t lock;

/**
 * @brief Handles remote memory requests 
 */
static void *rmem_server(void *args)
{
	int dma;           /* DMA channel to use. */
	char pathname[16]; /*  */
	int inbox;         /* Mailbox for small messages. */
	int inportal;      /* Portal for data transfers.  */

	dma = ((int *)args)[0];

	sprintf(pathname, "/rmem%d", dma);
	pthread_mutex_lock(&lock);
		inbox = mailbox_create(pathname);
		inportal = portal_create(pathname);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

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

	pthread_mutex_lock(&lock);
		portal_unlink(inportal);
		mailbox_unlink(inbox);
	pthread_mutex_unlock(&lock);

	return (NULL);
}

/**
 * @brief Remote memory server.
 */
int main(int argc, char **argv)
{
	int dmas[NR_IOCLUSTER_DMA];       /* DMA IDs.    */
	pthread_t tids[NR_IOCLUSTER_DMA]; /* Thread IDs. */

	((void) argc);
	((void) argv);

#ifdef DEBUG
	printf("[RMEM] booting up server\n");
#endif

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, NR_IOCLUSTER_DMA + 1);

	/* Spawn RMEM server threads. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
	{
		dmas[i] = i;
		assert((pthread_create(&tids[i],
			NULL,
			rmem_server,
			&dmas[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

#ifdef DEBUG
	printf("[RMEM] server alive\n");
#endif

	/* Release master IO cluster. */
	barrier_open(NR_IOCLUSTER);
	barrier_release();

	/* Wait for RMEM server threads. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
		pthread_join(tids[i], NULL);

	/* House keeping. */
	barrier_close();

	return (EXIT_SUCCESS);
}
