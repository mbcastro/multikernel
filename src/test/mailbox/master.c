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
	TEST_ASSERT((inbox = _mailbox_create(clusterid + dma)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((outbox = _mailbox_open(
		clusterid + (dma + 1)%NR_IOCLUSTER_DMA)) >= 0
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
 * Fault Injection Test: Invalid Create                              *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Invalid Create
 */
static void test_mailbox_invalid_create(void)
{
	int inbox;

	printf("Fault Injection Test: Invalid Create\n");

	TEST_ASSERT((inbox = _mailbox_create(-1)) < 0);
}

/*===================================================================*
 * Fault Injection Test: Bad Create                                  *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Bad Create
 */
static void test_mailbox_bad_create(void)
{
	int inbox;

	printf("Fault Injection Test: Bad Create\n");

	TEST_ASSERT((inbox = _mailbox_create(CCLUSTER0)) < 0);
}

/*===================================================================*
 * Fault Injection Test: Double Create                               *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Double Create
 */
static void test_mailbox_double_create(void)
{
	int inbox;
	int clusterid;

	printf("Fault Injection Test: Double Create\n");

	clusterid = k1_get_cluster_id();

	TEST_ASSERT((inbox = _mailbox_create(clusterid)) >= 0);
	TEST_ASSERT(_mailbox_create(clusterid) < 0);

	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
 * Fault Injection Test: Invalid Open                                *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Invalid Open
 */
static void test_mailbox_invalid_open(void)
{
	int outbox;

	printf("Fault Injection Test: Invalid Open\n");

	TEST_ASSERT((outbox = _mailbox_open(-1)) < 0);
}

/*===================================================================*
 * Fault Injection Test: Bad Open                                    *
 *===================================================================*/

#ifdef _TEST_MAILBOX_BAD_TEST_

/**
 * @brief Fault Injection Test: Bad Open
 */
static void test_mailbox_bad_open(void)
{
	int outbox;
	int clusterid;

	printf("Fault Injection Test: Bad Open\n");

	clusterid = k1_get_cluster_id();

	TEST_ASSERT((outbox = _mailbox_open(clusterid)) < 0);
}

#endif

/*===================================================================*
 * Fault Injection Test: Double Open                                 *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Double Open
 */
static void test_mailbox_double_open(void)
{
	int outbox;
	int clusterid;

	printf("Fault Injection Test: Double Open\n");

	clusterid = k1_get_cluster_id();

	TEST_ASSERT((outbox = _mailbox_open(clusterid + 1)) >= 0);
	TEST_ASSERT(_mailbox_open(clusterid + 1) < 0);

	TEST_ASSERT(mailbox_close(outbox) == 0);
}

/*===================================================================*
 * Fault Injection Test: Invalid Unlink                              *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Invalid Unlink
 */
static void test_mailbox_invalid_unlink(void)
{
	printf("Fault Injection Test: Invalid Unlink\n");

	TEST_ASSERT(mailbox_unlink(-1) < 0);
	TEST_ASSERT(mailbox_unlink(100000) < 0);
}

/*===================================================================*
 * Fault Injection Test: Bad Unlink                                  *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Bad Unlink
 */
static void test_mailbox_bad_unlink(void)
{
	printf("Fault Injection Test: Bad Unlink\n");

	TEST_ASSERT(mailbox_unlink(0) < 0);
	TEST_ASSERT(mailbox_unlink(1) < 0);
}

/*===================================================================*
 * Fault Injection Test: Invalid Close                              *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Invalid Close
 */
static void test_mailbox_invalid_close(void)
{
	printf("Fault Injection Test: Invalid Close\n");

	TEST_ASSERT(mailbox_close(-1) < 0);
	TEST_ASSERT(mailbox_close(100000) < 0);
}

/*===================================================================*
 * Fault Injection Test: Bad Close                                  *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Bad Close
 */
static void test_mailbox_bad_close(void)
{
	printf("Fault Injection Test: Bad Close\n");

	TEST_ASSERT(mailbox_close(0) < 0);
	TEST_ASSERT(mailbox_close(1) < 0);
}

/*===================================================================*
 * Fault Injection Test: Double Unlink                               *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Double Unlink
 */
static void test_mailbox_double_unlink(void)
{
	int inbox;
	int clusterid;

	printf("Fault Injection Test: Double Unlink\n");

	clusterid = k1_get_cluster_id();

	TEST_ASSERT((inbox = _mailbox_create(clusterid)) >= 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
	TEST_ASSERT(mailbox_unlink(inbox) < 0);
}

/*===================================================================*
 * Fault Injection Test: Double Close                                *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Double Close
 */
static void test_mailbox_double_close(void)
{
	int outbox;
	int clusterid;

	printf("Fault Injection Test: Double Close\n");

	clusterid = k1_get_cluster_id();

	TEST_ASSERT((outbox = _mailbox_open(clusterid + 1)) >= 0);
	TEST_ASSERT(mailbox_close(outbox) == 0);
	TEST_ASSERT(mailbox_close(outbox) < 0);
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

	/* API tests. */
	test_mailbox_create_unlink();
	test_mailbox_open_close();
	test_mailbox_read_write();

	/* Fault injection tests. */
	test_mailbox_invalid_create();
	test_mailbox_bad_create();
	test_mailbox_double_create();
	test_mailbox_invalid_open();
#ifdef _TEST_MAILBOX_BAD_TEST	
	test_mailbox_bad_open();
#endif
	test_mailbox_double_open();
	test_mailbox_invalid_unlink();
	test_mailbox_bad_unlink();
	test_mailbox_invalid_close();
	test_mailbox_bad_close();
	test_mailbox_double_unlink();
	test_mailbox_double_close();

	return (EXIT_SUCCESS);
}
