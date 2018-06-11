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

#include <nanvix/hal.h>
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
static void *test_hal_mailbox_thread_create_unlink(void *args)
{
	int dma;
	int inbox;
	int coreid;

	dma = ((int *)args)[0];

	coreid = hal_get_core_id();

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = hal_mailbox_create(coreid + dma)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	return (NULL);
}

/**
 * @brief API Test: Mailbox Create Unlink
 */
static void test_hal_mailbox_create_unlink(void)
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
			test_hal_mailbox_thread_create_unlink,
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
static void *test_hal_mailbox_thread_open_close(void *args)
{
	int dma;
	int inbox;
	int outbox;
	int coreid;

	dma = ((int *)args)[0];

	coreid = hal_get_core_id();

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = hal_mailbox_create(coreid + dma)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((outbox = hal_mailbox_open(
		coreid + (dma + 1)%NR_IOCLUSTER_DMA)) >= 0
	);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_mailbox_close(outbox) == 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	return (NULL);
}

/**
 * @brief API Test: Mailbox Open Close
 */
static void test_hal_mailbox_open_close(void)
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
			test_hal_mailbox_thread_open_close,
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
static void *test_hal_mailbox_thread_read_write(void *args)
{
	int dma;
	int inbox;
	int outbox;
	char buf[MAILBOX_MSG_SIZE];
	int coreid;

	dma = ((int *)args)[0];

	coreid = hal_get_core_id();

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = hal_mailbox_create(coreid + dma)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((outbox = hal_mailbox_open(
		coreid + (dma + 1)%NR_IOCLUSTER_DMA)) >= 0
	);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	memset(buf, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_write(outbox, buf, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
	memset(buf, 0, MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_read(inbox, buf, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);

	for (int i = 0; i < MAILBOX_MSG_SIZE; i++)
		TEST_ASSERT(buf[i] == 1);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_mailbox_close(outbox) == 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	return (NULL);
}

/**
 * @brief API Test: Mailbox Read Write
 */
static void test_hal_mailbox_read_write(void)
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
			test_hal_mailbox_thread_read_write,
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
static void test_hal_mailbox_invalid_create(void)
{
	int inbox;

	printf("Fault Injection Test: Invalid Create\n");

	TEST_ASSERT((inbox = hal_mailbox_create(-1)) < 0);
}

/*===================================================================*
 * Fault Injection Test: Bad Create                                  *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Bad Create
 */
static void test_hal_mailbox_bad_create(void)
{
	int inbox;

	printf("Fault Injection Test: Bad Create\n");

	TEST_ASSERT((inbox = hal_mailbox_create(CCLUSTER0)) < 0);
}

/*===================================================================*
 * Fault Injection Test: Double Create                               *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Double Create
 */
static void test_hal_mailbox_double_create(void)
{
	int inbox;
	int coreid;

	printf("Fault Injection Test: Double Create\n");

	coreid = hal_get_core_id();

	TEST_ASSERT((inbox = hal_mailbox_create(coreid)) >= 0);
	TEST_ASSERT(hal_mailbox_create(coreid) < 0);

	TEST_ASSERT(hal_mailbox_unlink(inbox) == 0);
}

/*===================================================================*
 * Fault Injection Test: Invalid Open                                *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Invalid Open
 */
static void test_hal_mailbox_invalid_open(void)
{
	int outbox;

	printf("Fault Injection Test: Invalid Open\n");

	TEST_ASSERT((outbox = hal_mailbox_open(-1)) < 0);
}

/*===================================================================*
 * Fault Injection Test: Bad Open                                    *
 *===================================================================*/

#ifdef _TEST_MAILBOX_BAD_TEST_

/**
 * @brief Fault Injection Test: Bad Open
 */
static void test_hal_mailbox_bad_open(void)
{
	int outbox;
	int coreid;

	printf("Fault Injection Test: Bad Open\n");

	coreid = hal_get_core_id();

	TEST_ASSERT((outbox = hal_mailbox_open(coreid)) < 0);
}

#endif

/*===================================================================*
 * Fault Injection Test: Double Open                                 *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Double Open
 */
static void test_hal_mailbox_double_open(void)
{
	int outbox;
	int coreid;

	printf("Fault Injection Test: Double Open\n");

	coreid = hal_get_core_id();

	TEST_ASSERT((outbox = hal_mailbox_open(coreid + 1)) >= 0);
	TEST_ASSERT(hal_mailbox_open(coreid + 1) < 0);

	TEST_ASSERT(hal_mailbox_close(outbox) == 0);
}

/*===================================================================*
 * Fault Injection Test: Double Unlink                               *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Double Unlink
 */
static void test_hal_mailbox_double_unlink(void)
{
	int inbox;
	int coreid;

	printf("Fault Injection Test: Double Unlink\n");

	coreid = hal_get_core_id();

	TEST_ASSERT((inbox = hal_mailbox_create(coreid)) >= 0);
	TEST_ASSERT(hal_mailbox_unlink(inbox) == 0);
	TEST_ASSERT(hal_mailbox_unlink(inbox) < 0);
}

/*===================================================================*
 * Fault Injection Test: Double Close                                *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Double Close
 */
static void test_hal_mailbox_double_close(void)
{
	int outbox;
	int coreid;

	printf("Fault Injection Test: Double Close\n");

	coreid = hal_get_core_id();

	TEST_ASSERT((outbox = hal_mailbox_open(coreid + 1)) >= 0);
	TEST_ASSERT(hal_mailbox_close(outbox) == 0);
	TEST_ASSERT(hal_mailbox_close(outbox) < 0);
}

/*===================================================================*
 * Fault Injection Test: Invalid Write                               *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Invalid Write
 */
static void test_hal_mailbox_invalid_write(void)
{
	char buf[MAILBOX_MSG_SIZE];

	printf("Fault Injection Test: Invalid Write\n");

	memset(buf, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_write(-1, buf, MAILBOX_MSG_SIZE) != MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_write(100000, buf, MAILBOX_MSG_SIZE) != MAILBOX_MSG_SIZE);
}

/*===================================================================*
 * Fault Injection Test: Bad Write                                   *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Bad Write
 */
static void test_hal_mailbox_bad_write(void)
{
	int inbox;
	char buf[MAILBOX_MSG_SIZE];
	int coreid;

	printf("Fault Injection Test: Bad Write\n");

	coreid = hal_get_core_id();

	TEST_ASSERT((inbox = hal_mailbox_create(coreid)) >= 0);

	memset(buf, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_write(inbox, buf, 1) != MAILBOX_MSG_SIZE);

	TEST_ASSERT(hal_mailbox_unlink(inbox) == 0);
}

/*===================================================================*
 * Fault Injection Test: Null Write                                  *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Null Write
 */
static void test_hal_mailbox_null_write(void)
{
	int outbox;
	int coreid;

	printf("Fault Injection Test: Null Write\n");

	coreid = hal_get_core_id();

	TEST_ASSERT((outbox = hal_mailbox_open(coreid + 1)) >= 0);
	TEST_ASSERT(hal_mailbox_write(outbox, NULL, MAILBOX_MSG_SIZE) != MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_close(outbox) == 0);
}

/*===================================================================*
 * Fault Injection Test: Invalid Write                               *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Invalid Read
 */
static void test_hal_mailbox_invalid_read(void)
{
	char buf[MAILBOX_MSG_SIZE];

	printf("Fault Injection Test: Invalid Read\n");

	memset(buf, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_read(-1, buf, MAILBOX_MSG_SIZE) != MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_read(100000, buf, MAILBOX_MSG_SIZE) != MAILBOX_MSG_SIZE);
}

/*===================================================================*
 * Fault Injection Test: Bad Read                                   *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Bad Read
 */
static void test_hal_mailbox_bad_read(void)
{
	int outbox;
	char buf[MAILBOX_MSG_SIZE];
	int coreid;

	printf("Fault Injection Test: Bad Read\n");

	coreid = hal_get_core_id();

	TEST_ASSERT((outbox = hal_mailbox_open(coreid + 1)) >= 0);

	memset(buf, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_read(outbox, buf, 1) != MAILBOX_MSG_SIZE);

	TEST_ASSERT(hal_mailbox_close(outbox) == 0);
}

/*===================================================================*
 * Fault Injection Test: Null Read                                  *
 *===================================================================*/

/**
 * @brief Fault Injection Test: Null Read
 */
static void test_hal_mailbox_null_read(void)
{
	int inbox;
	int coreid;

	printf("Fault Injection Test: Null Read\n");

	coreid = hal_get_core_id();

	TEST_ASSERT((inbox = hal_mailbox_create(coreid)) >= 0);
	TEST_ASSERT(hal_mailbox_read(inbox, NULL, MAILBOX_MSG_SIZE) != MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_unlink(inbox) == 0);
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
	test_hal_mailbox_create_unlink();
	test_hal_mailbox_open_close();
	test_hal_mailbox_read_write();

	/* Fault injection tests. */
	test_hal_mailbox_invalid_create();
	test_hal_mailbox_bad_create();
	test_hal_mailbox_double_create();
	test_hal_mailbox_invalid_open();
#ifdef _TEST_MAILBOX_BAD_TEST	
	test_hal_mailbox_bad_open();
#endif
	test_hal_mailbox_double_open();
	test_hal_mailbox_double_unlink();
	test_hal_mailbox_double_close();
	test_hal_mailbox_invalid_write();
	test_hal_mailbox_bad_write();
	test_hal_mailbox_null_write();
	test_hal_mailbox_invalid_read();
	test_hal_mailbox_bad_read();
	test_hal_mailbox_null_read();

	return (EXIT_SUCCESS);
}
