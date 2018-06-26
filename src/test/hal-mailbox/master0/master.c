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

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include <mppa/osconfig.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_MAILBOX_
#include <nanvix/config.h>
#include <nanvix/hal.h>
#include <nanvix/pm.h>

#define OTHER_IOCLUSTER 192

/**
 * @brief Number of cores in the underlying cluster.
 */
static int ncores = 0;

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Global barrier for synchronization.
 */
static pthread_barrier_t barrier;

/**
 * @brief Synchronization point.
 */
static int syncid;
static int syncid_local;

static int nodes[2];
static int nodes_local[2];

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
	int inbox;
	int nodeid;

	((void)args);

	hal_setup();

	nodeid = hal_get_node_id();

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = hal_mailbox_create(nodeid)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Mailbox Create Unlink
 */
static void test_hal_mailbox_create_unlink(void)
{
	pthread_t tids[ncores];

	printf("[test][api] Mailbox Create Unlink\n");

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		assert((pthread_create(&tids[i],
			NULL,
			test_hal_mailbox_thread_create_unlink,
			NULL)) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
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
	int tid;
	int inbox;
	int outbox;
	int nodeid;

	hal_setup();

	tid = ((int *)args)[0];

	nodeid = hal_get_node_id();

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = hal_mailbox_create(nodeid)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((outbox = hal_mailbox_open(
		((tid + 1) == ncores) ?
			nodeid + 1 - ncores + 1:
			nodeid + 1)) >= 0
	);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_mailbox_close(outbox) == 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Mailbox Open Close
 */
static void test_hal_mailbox_open_close(void)
{
	int tids[ncores];
	pthread_t threads[ncores];

	printf("[test][api] Mailbox Open Close\n");

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		tids[i] = i;
		assert((pthread_create(&threads[i],
			NULL,
			test_hal_mailbox_thread_open_close,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * API Test: Open Close between IO Clusters                          *
 *===================================================================*/

/**
 * @brief API Test: Open Close between IO Clusters
 */
static void *test_hal_mailbox_thread_open_close_io(void *args)
{
	int inbox;
	int outbox;
	int nodeid;
	int tid;

	hal_setup();

	tid = ((int *)args)[0];

	nodeid = hal_get_node_id();

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = hal_mailbox_create(nodeid)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_signal(syncid) == 0);
	TEST_ASSERT(hal_sync_wait(syncid_local) == 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((outbox = hal_mailbox_open(OTHER_IOCLUSTER + tid)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_mailbox_close(outbox) == 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_signal(syncid) == 0);
	TEST_ASSERT(hal_sync_wait(syncid_local) == 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	hal_cleanup();
	return(NULL);
}

/**
 * @brief API Test: Open Close between IO Clusters
 */
static void test_hal_mailbox_open_close_io(void)
{
	int tids[ncores];
	pthread_t threads[ncores];

	printf("[test][api] Mailbox Open Close IO Cluster 0\n");

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		tids[i] = i;
		assert((pthread_create(&threads[i],
			NULL,
			test_hal_mailbox_thread_open_close_io,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * API Test: Read Write                                              *
 *===================================================================*/

/**
 * @brief API Test: Mailbox Read Write
 */
static void *test_hal_mailbox_thread_read_write(void *args)
{
	int tid;
	int inbox;
	int outbox;
	char buf[HAL_MAILBOX_MSG_SIZE];
	int nodeid;

	hal_setup();

	tid = ((int *)args)[0];

	nodeid = hal_get_node_id();

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = hal_mailbox_create(nodeid)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((outbox = hal_mailbox_open(
		((tid + 1) == ncores) ?
			nodeid + 1 - ncores + 1:
			nodeid + 1)) >= 0
	);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	memset(buf, 1, HAL_MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_write(outbox, buf, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);
	memset(buf, 0, HAL_MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_read(inbox, buf, HAL_MAILBOX_MSG_SIZE) == HAL_MAILBOX_MSG_SIZE);

	for (int i = 0; i < HAL_MAILBOX_MSG_SIZE; i++)
		TEST_ASSERT(buf[i] == 1);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_mailbox_close(outbox) == 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Mailbox Read Write
 */
static void test_hal_mailbox_read_write(void)
{
	int tids[ncores];
	pthread_t threads[ncores];

	printf("[test][api] Mailbox Read Write\n");

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		tids[i] = i;
		assert((pthread_create(&threads[i],
			NULL,
			test_hal_mailbox_thread_read_write,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(threads[i], NULL);
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

	printf("[test][fault injection] Invalid Create\n");

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

	printf("[test][fault injection] Bad Create\n");

	TEST_ASSERT((inbox = hal_mailbox_create(NAME_SERVER_NODE)) < 0);
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
	int nodeid;

	printf("[test][fault injection] Double Create\n");

	nodeid = hal_get_cluster_id();

	TEST_ASSERT((inbox = hal_mailbox_create(nodeid)) >= 0);
	TEST_ASSERT(hal_mailbox_create(nodeid) < 0);

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

	printf("[test][fault injection] Invalid Open\n");

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
	int nodeid;

	printf("[test][fault injection] Bad Open\n");

	nodeid = hal_get_cluster_id();

	TEST_ASSERT((outbox = hal_mailbox_open(nodeid)) < 0);
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
	int nodeid;

	printf("[test][fault injection] Double Open\n");

	nodeid = hal_get_cluster_id();

	TEST_ASSERT((outbox = hal_mailbox_open(nodeid + 1)) >= 0);
	TEST_ASSERT(hal_mailbox_open(nodeid + 1) < 0);

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
	int nodeid;

	printf("[test][fault injection] Double Unlink\n");

	nodeid = hal_get_cluster_id();

	TEST_ASSERT((inbox = hal_mailbox_create(nodeid)) >= 0);
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
	int nodeid;

	printf("[test][fault injection] Double Close\n");

	nodeid = hal_get_cluster_id();

	TEST_ASSERT((outbox = hal_mailbox_open(nodeid + 1)) >= 0);
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
	char buf[HAL_MAILBOX_MSG_SIZE];

	printf("[test][fault injection] Invalid Write\n");

	memset(buf, 1, HAL_MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_write(-1, buf, HAL_MAILBOX_MSG_SIZE) != HAL_MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_write(100000, buf, HAL_MAILBOX_MSG_SIZE) != HAL_MAILBOX_MSG_SIZE);
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
	char buf[HAL_MAILBOX_MSG_SIZE];
	int nodeid;

	printf("[test][fault injection] Bad Write\n");

	nodeid = hal_get_cluster_id();

	TEST_ASSERT((inbox = hal_mailbox_create(nodeid)) >= 0);

	memset(buf, 1, HAL_MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_write(inbox, buf, 1) != HAL_MAILBOX_MSG_SIZE);

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
	int nodeid;

	printf("[test][fault injection] Null Write\n");

	nodeid = hal_get_cluster_id();

	TEST_ASSERT((outbox = hal_mailbox_open(nodeid + 1)) >= 0);
	TEST_ASSERT(hal_mailbox_write(outbox, NULL, HAL_MAILBOX_MSG_SIZE) != HAL_MAILBOX_MSG_SIZE);
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
	char buf[HAL_MAILBOX_MSG_SIZE];

	printf("[test][fault injection] Invalid Read\n");

	memset(buf, 1, HAL_MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_read(-1, buf, HAL_MAILBOX_MSG_SIZE) != HAL_MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_read(100000, buf, HAL_MAILBOX_MSG_SIZE) != HAL_MAILBOX_MSG_SIZE);
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
	char buf[HAL_MAILBOX_MSG_SIZE];
	int nodeid;

	printf("[test][fault injection] Bad Read\n");

	nodeid = hal_get_cluster_id();

	TEST_ASSERT((outbox = hal_mailbox_open(nodeid + 1)) >= 0);

	memset(buf, 1, HAL_MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_read(outbox, buf, 1) != HAL_MAILBOX_MSG_SIZE);

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
	int nodeid;

	printf("[test][fault injection] Null Read\n");

	nodeid = hal_get_cluster_id();

	TEST_ASSERT((inbox = hal_mailbox_create(nodeid)) >= 0);
	TEST_ASSERT(hal_mailbox_read(inbox, NULL, HAL_MAILBOX_MSG_SIZE) != HAL_MAILBOX_MSG_SIZE);
	TEST_ASSERT(hal_mailbox_unlink(inbox) == 0);
}

/*===================================================================*
 * Mailbox Test Driver                                               *
 *===================================================================*/

/**
 * @brief Mailbox Test Driver
 */
int main(int argc, const char **argv)
{
	((void) argc);
	((void) argv);

	hal_setup();

	ncores = hal_get_num_cores();

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, ncores - 1);

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

	/* Tests using both IO clusters. */

	/* Wait for other IO cluster. */
	nodes[0] = hal_get_node_id();
	nodes[1] = OTHER_IOCLUSTER;

	nodes_local[0] = OTHER_IOCLUSTER;
	nodes_local[1] = hal_get_node_id();

	TEST_ASSERT((syncid_local = hal_sync_create(nodes_local, 2, HAL_SYNC_ONE_TO_ALL)) >= 0);
	TEST_ASSERT((syncid = hal_sync_open(nodes, 2, HAL_SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(hal_sync_signal(syncid) == 0);
	TEST_ASSERT(hal_sync_wait(syncid_local) == 0);

	test_hal_mailbox_open_close_io();

	/* House keeping. */
	TEST_ASSERT(hal_sync_unlink(syncid_local) == 0);
	TEST_ASSERT(hal_sync_close(syncid) == 0)

	hal_cleanup();
	return (EXIT_SUCCESS);
}
