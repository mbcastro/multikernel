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

#include <mppa/osconfig.h>
#include <nanvix/hal.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#endif

/**
 * @brief Remote memory.
 */
static char rmem[RMEM_SIZE];

static pthread_barrier_t barrier;

static pthread_mutex_t lock;

/*===================================================================*
 * rmem_write()                                                      *
 *===================================================================*/

/**
 * @brief Handles a write request.
 *
 * @param inportal Input portal for data transfer.
 * @param remote   Remote client.
 * @param blknum   RMEM block.
 * @param size     Number of bytes to write.
 */
static inline void rmem_write(int inportal, int remote, uint64_t blknum, int size)
{
	portal_allow(inportal, remote);
	portal_read(inportal, &rmem[blknum], size);
}

/*===================================================================*
 * rmem_read()                                                       *
 *===================================================================*/

/**
 * @brief Handles a read request.
 *
 * @param remote Remote client.
 * @param blknum RMEM block.
 * @param size   Number of bytes to write.
 */
static inline void rmem_read(int remote, uint64_t blknum, int size)
{
	int outportal;

	outportal = _portal_open(remote);
	portal_write(outportal, &rmem[blknum], size);
	portal_close(outportal);
}

/*===================================================================*
 * rmem_server()                                                     *
 *===================================================================*/

/**
 * @brief Handles remote memory requests.
 *
 * @param args Server arguments.
 *
 * @returns Always returns NULL.
 */
static void *rmem_server(void *args)
{
	int dma;           /* DMA channel to use.         */
	int inbox;         /* Mailbox for small messages. */
	int inportal;      /* Portal for receiving data.  */
	char pathname[16]; /* RMEM bank.                  */

	hal_setup();

	dma = ((int *)args)[0];

	sprintf(pathname, "/rmem%d", dma);
	pthread_mutex_lock(&lock);
		inbox = hal_mailbox_create(IOCLUSTER1 + dma);
		inportal = portal_create(pathname);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	while(1)
	{
		struct rmem_message msg;

		mailbox_read(inbox, &msg);

		/* handle write operation. */
		switch (msg.op)
		{
			/* Write to RMEM. */
			case RMEM_WRITE:
				rmem_write(inportal, msg.source, msg.blknum, msg.size);
				break;

			/* Read from RMEM. */
			case RMEM_READ:
				rmem_read(msg.source, msg.blknum, msg.size);
				break;

			/* Should not happen. */
			default:
				break;
		}
	}

	/* House keeping. */
	pthread_mutex_lock(&lock);
		portal_unlink(inportal);
		mailbox_unlink(inbox);
	pthread_mutex_unlock(&lock);

	hal_cleanup();
	return (NULL);
}

/*===================================================================*
 * main()                                                            *
 *===================================================================*/

/**
 * @brief Remote memory server.
 */
int main(int argc, char **argv)
{
	int global_barrier;               /* System barrier. */
	int dmas[NR_IOCLUSTER_DMA];       /* DMA IDs.        */
	pthread_t tids[NR_IOCLUSTER_DMA]; /* Thread IDs.     */
	char pathname[16];

	((void) argc);
	((void) argv);

	hal_setup();

#ifdef DEBUG
	printf("[RMEM] booting up server\n");
#endif

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, NR_IOCLUSTER_DMA + 1);

	/* Register name processes */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
	{
		sprintf(pathname, "/rmem%d", i);
		name_link(IOCLUSTER1 + i, pathname);
	}

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

	/* Release master IO cluster. */
	global_barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(global_barrier);

#ifdef DEBUG
	printf("[RMEM] server alive\n");
#endif

	/* Wait for RMEM server threads. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
		pthread_join(tids[i], NULL);

	/* House keeping. */
	barrier_close(global_barrier);

	hal_cleanup();
	return (EXIT_SUCCESS);
}
