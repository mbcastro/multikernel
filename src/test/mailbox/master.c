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

#include <assert.h>
#include <pthread.h>

#include <mppa/osconfig.h>

#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Global barrier for synchronization.
 */
static pthread_barrier_t barrier;

/**
 * @brief Lock for critical sections.
 */
static pthread_mutex_t lock;

/*===================================================================*
 * API Test: Create Unlink                                           *
 *===================================================================*/

/**
 * @brief API Test: Mailbox Create Unlink
 */
static void *test_mailbox_thread_create_unlink(void *args)
{
	int dma;
	int inbox;
	int clusterid;

	dma = ((int *)args)[0];

	clusterid = k1_get_cluster_id();

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = _mailbox_create(clusterid + dma)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	return (NULL);
}

/**
 * @brief API Test: Mailbox Create Unlink
 */
static void test_mailbox_create_unlink(void)
{
	int dmas[NR_IOCLUSTER_DMA];
	pthread_t tids[NR_IOCLUSTER_DMA];

	printf("API Test: Mailbox Create Unlink\n");

	/* Spawn driver threads. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
	{
		dmas[i] = i;
		assert((pthread_create(&tids[i],
			NULL,
			test_mailbox_thread_create_unlink,
			&dmas[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * API Test: Open Close                                              *
 *===================================================================*/

/**
 * @brief API Test: Mailbox Open Close
 */
static void *test_mailbox_thread_open_close(void *args)
{
	int dma;
	int inbox;
	int outbox;
	int clusterid;

	dma = ((int *)args)[0];

	clusterid = k1_get_cluster_id();

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = _mailbox_create(clusterid + dma)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((outbox = _mailbox_open(
		clusterid + (dma + 1)%NR_IOCLUSTER_DMA)) >= 0
	);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(mailbox_close(outbox) == 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	return (NULL);
}

/**
 * @brief API Test: Mailbox Open Close
 */
static void test_mailbox_open_close(void)
{
	int dmas[NR_IOCLUSTER_DMA];
	pthread_t tids[NR_IOCLUSTER_DMA];

	printf("API Test: Mailbox Open Close\n");

	/* Spawn driver threads. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
	{
		dmas[i] = i;
		assert((pthread_create(&tids[i],
			NULL,
			test_mailbox_thread_open_close,
			&dmas[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * API Test: Read Write                                              *
 *===================================================================*/

/**
 * @brief API Test: Mailbox Read Write
 */
static void *test_mailbox_thread_read_write(void *args)
{
	int dma;
	int inbox;
	int outbox;
	char buf[MAILBOX_MSG_SIZE];
	int clusterid;

	dma = ((int *)args)[0];

	clusterid = k1_get_cluster_id();

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = _mailbox_create(clusterid + dma, NAME)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((outbox = _mailbox_open(
		clusterid + (dma + 1)%NR_IOCLUSTER_DMA,
		NAME)) >= 0
	);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	memset(buf, 1, MAILBOX_MSG_SIZE);
	mailbox_write(outbox, buf);
	memset(buf, 0, MAILBOX_MSG_SIZE);
	mailbox_read(inbox, buf);

	for (int i = 0; i < MAILBOX_MSG_SIZE; i++)
		TEST_ASSERT(buf[i] == 1);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(mailbox_close(outbox) == 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	return (NULL);
}

/**
 * @brief API Test: Mailbox Read Write
 */
static void test_mailbox_read_write(void)
{
	int dmas[NR_IOCLUSTER_DMA];
	pthread_t tids[NR_IOCLUSTER_DMA];

	printf("API Test: Mailbox Read Write\n");

	/* Spawn driver threads. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
	{
		dmas[i] = i;
		assert((pthread_create(&tids[i],
			NULL,
			test_mailbox_thread_read_write,
			&dmas[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
		pthread_join(tids[i], NULL);
}
/*===================================================================*
 * API Test: Mailbox Driver                                          *
 *===================================================================*/

/**
 * @brief Mailbox test driver.
 */
int main(int argc, const char **argv)
{
	((void) argc);
	((void) argv);

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, NR_IOCLUSTER_DMA);

	test_mailbox_create_unlink();
	test_mailbox_open_close();
	test_mailbox_read_write();

	return (EXIT_SUCCESS);
}
