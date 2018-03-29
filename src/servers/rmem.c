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
#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#endif

/**
 * @brief Remote memory.
 */
static char rmem[RMEM_SIZE];

/**
 * @brief Locks.
 */
static int rmem_locks[RMEM_SIZE/RMEM_BLOCK/SIZE];

/**
 * @brief Remotes waiting for a block.
 */
static int remotes[NR_CCLUSTER];

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

	outportal = portal_open(name_cluster_name(remote));
	portal_write(outportal, &rmem[blknum], size);
	portal_close(outportal);
}

/*===================================================================*
 * rmem_lock()                                                       *
 *===================================================================*/

/**
 * @brief Handles a lock request.
 *
 * @param remote Remote client.
 * @param blknum RMEM block.
 */
static inline void rmem_lock(int remote, uint64_t blknum)
{
	int outbox;
	struct rmem_message msg;

	pthread_mutex_lock(&lock);	

	/* Sleep. */
	if (rmem_locks[blknum] != 0)
	{
		remotes[remote] = blknum;
		pthread_mux_unlock(&lock);
		return;
	}

	/* Lock. */
	rmem_locks[blknum] = remote + 1;
	pthread_mux_unlock(&lock);

	/* ACK message. */
	msg.source = k1_get_cluster_id();
	msg.op = RMEM_SUCCESS;
	outbox = mailbox_open(name_cluster_name(remote));
	mailbox_write(outbox, &msg);
	mailbox_close(outbox);
}

/*===================================================================*
 * rmem_unlock()                                                     *
 *===================================================================*/

/**
 * @brief Handles an unlock request.
 *
 * @param remote Remote client.
 * @param blknum RMEM block.
 */
static inline void rmem_unlock(int remote, uint64_t blknum)
{
	int outbox;
	struct rmem_message msg;

	pthread_mutex_lock(&lock);	

	/* Nothing to do. */
	if (rmem_locks[blknum] != (remote + 1))
	{
		pthread_mux_unlock(&lock);
		return;
	}

	/* Unlock. */
	rmem_locks[blknum] = 0;
	remote = -1;

	for (int i = 0; i < NR_CCLUSTER, i++)
	{
		if (remotes[i] == blknum)
		{
			remote = i;
			rmem_locks[blknum] = remote + 1;
			break;
		}
	}

	pthread_mux_unlock(&lock);

	/* ACK message. */
	if (remote >= 0)
	{
		msg.source = k1_get_cluster_id();
		msg.op = RMEM_SUCCESS;
		outbox = mailbox_open(name_cluster_name(remote));
		mailbox_write(outbox, &msg);
		mailbox_close(outbox);
	}
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
		switch (msg.op)
		{
			/* Write to RMEM. */
			case RMEM_WRITE:
				rmem_write(input, msg.source, msg.blknum, msg.size);
				break;

			/* Read from RMEM. */
			case RMEM_READ:
				rmem_read(output, msg.source, msg.blknum, msg.size);
				break;

			/* Lock a RMEM block. */
			case RMEM_LOCK:
				rmem_lock(msg.source, msg.blknum);
				break;

			/* Unlock a RMEM block. */
			case RMEM_UNLOCK:
				rmem_unlock(msg.source, msg.blknum)
				break;
		}
	}

	/* House keeping. */
	pthread_mutex_lock(&lock);
		portal_unlink(inportal);
		mailbox_unlink(inbox);
	pthread_mutex_unlock(&lock);

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
	int dmas[NR_IOCLUSTER_DMA];       /* DMA IDs.    */
	pthread_t tids[NR_IOCLUSTER_DMA]; /* Thread IDs. */

	((void) argc);
	((void) argv);

#ifdef DEBUG
	printf("[RMEM] booting up server\n");
#endif

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, NR_IOCLUSTER_DMA + 1);

	memset(remotes, 0, sizeof(remotes));
	memset(rmem_locks, 0, sizeof(rmem_locks));

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
