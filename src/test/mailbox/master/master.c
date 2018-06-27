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
 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>

 #include <mppa/osconfig.h>
 #include <mppaipc.h>

 #define __NEED_HAL_CORE_
 #define __NEED_HAL_NOC_
 #define __NEED_HAL_SYNC_
 #define __NEED_HAL_MAILBOX_
 #include <nanvix/hal.h>
 #include <nanvix/init.h>
 #include <nanvix/name.h>
 #include <nanvix/limits.h>
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
 * @brief Number of cores in the underlying cluster.
 */
static int ncores = 0;

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
	char pathname[NANVIX_PROC_NAME_MAX];
	int tid;
	int inbox;

	TEST_ASSERT(kernel_setup() == 0);

	pthread_barrier_wait(&barrier);

	tid = ((int *)args)[0];

	sprintf(pathname, "cool-name%d", tid);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = mailbox_create(pathname)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Mailbox Create Unlink
 */
static void test_mailbox_create_unlink(void)
{
	int tids[ncores];
	pthread_t threads[ncores];

	printf("[test][api] Mailbox Create Unlink\n");

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		tids[i] = i;
		assert((pthread_create(&threads[i],
			NULL,
			test_mailbox_thread_create_unlink,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * API Test: Open Close                                              *
 *===================================================================*/

/**
 * @brief API Test: Mailbox Open Close
 */
static void *test_mailbox_thread_open_close(void *args)
{
	char pathname_local[NANVIX_PROC_NAME_MAX];
	char pathname_remote[NANVIX_PROC_NAME_MAX];
	int tid;
	int inbox;
	int outbox;

	TEST_ASSERT(kernel_setup() == 0);

	pthread_barrier_wait(&barrier);

	tid = ((int *)args)[0];

	sprintf(pathname_local, "cool-name%d", tid);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = mailbox_create(pathname_local)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	sprintf(pathname_remote, "cool-name%d",
		((tid + 1) == ncores) ?
		1:tid + 1
	);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((outbox = mailbox_open(pathname_remote)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(mailbox_close(outbox) == 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Mailbox Open Close
 */
static void test_mailbox_open_close(void)
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
			test_mailbox_thread_open_close,
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
static void *test_mailbox_thread_read_write(void *args)
{
	char pathname_local[NANVIX_PROC_NAME_MAX];
	char pathname_remote[NANVIX_PROC_NAME_MAX];
	char buf[HAL_MAILBOX_MSG_SIZE];
	int tid;
	int inbox;
	int outbox;

	TEST_ASSERT(kernel_setup() == 0);

	pthread_barrier_wait(&barrier);

	tid = ((int *)args)[0];

	sprintf(pathname_local, "cool-name%d", tid);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = mailbox_create(pathname_local)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	sprintf(pathname_remote, "cool-name%d",
		((tid + 1) == ncores) ?
		1:tid + 1
	);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((outbox = mailbox_open(pathname_remote)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	memset(buf, 1, HAL_MAILBOX_MSG_SIZE);
	TEST_ASSERT(mailbox_write(outbox, buf, sizeof(buf)) == 0);
	memset(buf, 0, HAL_MAILBOX_MSG_SIZE);
	TEST_ASSERT(mailbox_read(inbox, buf, sizeof(buf)) == 0);

	for (int i = 0; i < HAL_MAILBOX_MSG_SIZE; i++)
		TEST_ASSERT(buf[i] == 1);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(mailbox_close(outbox) == 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Mailbox Read Write
 */
static void test_mailbox_read_write(void)
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
			test_mailbox_thread_read_write,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * API Test: Compute Clusters tests                                  *
 *===================================================================*/

/**
 * @brief API Test: Compute Clusters tests
 */
static void test_mailbox_cc(int nclusters)
{
	int status;
	int pids[nclusters];

	char nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/mailbox-slave",
		nclusters_str,
		test_str,
		NULL
	};

	printf("[test][api] Compute Clusters\n");

	sprintf(nclusters_str, "%d", nclusters);
	sprintf(test_str, "%d", 0);

	for (int i = 0; i < nclusters; i++)
		TEST_ASSERT((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);

	for (int i = 0; i < nclusters; i++)
	{
		TEST_ASSERT(mppa_waitpid(pids[i], &status, 0) != -1);
		TEST_ASSERT(status == EXIT_SUCCESS);
	}
}

/*===================================================================*
 * API Test: Mailbox Driver                                          *
 *===================================================================*/

/**
 * @brief Mailbox test driver.
 */
int main(int argc, const char **argv)
{
	int nodes[2];
	int syncid;
	int nclusters;

	TEST_ASSERT(argc == 2);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);

	TEST_ASSERT(kernel_setup() == 0);

	/* Wait spawner server. */
	nodes[0] = 128;
	nodes[1] = hal_get_node_id();

	TEST_ASSERT((syncid = hal_sync_create(nodes, 2, HAL_SYNC_ONE_TO_ALL)) >= 0);
	TEST_ASSERT(hal_sync_wait(syncid) == 0);

	ncores = hal_get_num_cores();

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, ncores - 1);

	/* API tests. */
	test_mailbox_create_unlink();
	test_mailbox_open_close();
	test_mailbox_read_write();
	test_mailbox_cc(nclusters);

	TEST_ASSERT(kernel_cleanup() == 0);
	return (EXIT_SUCCESS);
}
