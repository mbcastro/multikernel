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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <mppaipc.h>

#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

#include "test.h"

/**
 * @brief Local lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*============================================================================*
 * API Test: Create Unlink                                                    *
 *============================================================================*/

/**
 * @brief API Test: Mailbox Create Unlink
 */
static void *test_mailbox_thread_create_unlink(void *args)
{
	char pathname[NANVIX_PROC_NAME_MAX];
	int tid;
	int inbox;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

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

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Mailbox Create Unlink
 */
static void test_mailbox_create_unlink(void)
{
	int tids[ipc_mailbox_ncores];
	pthread_t threads[ipc_mailbox_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_mailbox_thread_create_unlink,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Open Close                                                       *
 *============================================================================*/

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
	TEST_ASSERT(runtime_setup(1) == 0);

	pthread_barrier_wait(&barrier);

	tid = ((int *)args)[0];

	sprintf(pathname_local, "cool-name%d", tid);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = mailbox_create(pathname_local)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	sprintf(pathname_remote, "cool-name%d",
		((tid + 1) == ipc_mailbox_ncores) ?
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

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Mailbox Open Close
 */
static void test_mailbox_open_close(void)
{
	int tids[ipc_mailbox_ncores];
	pthread_t threads[ipc_mailbox_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_mailbox_thread_open_close,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Read Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Mailbox Read Write
 */
static void *test_mailbox_thread_read_write(void *args)
{
	char pathname_local[NANVIX_PROC_NAME_MAX];
	char pathname_remote[NANVIX_PROC_NAME_MAX];
	char buf[MAILBOX_MSG_SIZE];
	int tid;
	int inbox;
	int outbox;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	pthread_barrier_wait(&barrier);

	tid = ((int *)args)[0];

	sprintf(pathname_local, "cool-name%d", tid);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((inbox = mailbox_create(pathname_local)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	sprintf(pathname_remote, "cool-name%d",
		((tid + 1) == ipc_mailbox_ncores) ?
		1:tid + 1
	);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((outbox = mailbox_open(pathname_remote)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	memset(buf, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(mailbox_write(outbox, buf, sizeof(buf)) == 0);
	memset(buf, 0, MAILBOX_MSG_SIZE);
	TEST_ASSERT(mailbox_read(inbox, buf, sizeof(buf)) == 0);

	for (int i = 0; i < MAILBOX_MSG_SIZE; i++)
		TEST_ASSERT(buf[i] == 1);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(mailbox_close(outbox) == 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
	pthread_mutex_unlock(&lock);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Mailbox Read Write
 */
static void test_mailbox_read_write(void)
{
	int tids[ipc_mailbox_ncores];
	pthread_t threads[ipc_mailbox_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_mailbox_thread_read_write,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test ipc_mailbox_tests_api[] = {
	{ test_mailbox_create_unlink, "Create Unlink" },
	{ test_mailbox_open_close,    "Open Close"    },
	{ test_mailbox_read_write,    "Read Write"    },
	{ NULL,                       NULL            },
};
