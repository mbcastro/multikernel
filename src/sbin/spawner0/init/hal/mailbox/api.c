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

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <nanvix/syscalls.h>
#include <nanvix/const.h>

#include "test.h"

/*============================================================================*
 * API Test: Create Unlink                                                    *
 *============================================================================*/

/**
 * @brief API Test: Mailbox Create Unlink
 */
static void *test_sys_mailbox_thread_create_unlink(void *args)
{
	int inbox;
	int nodenum;

	((void)args);

	kernel_setup();

	nodenum = sys_get_node_num();

	TEST_ASSERT((inbox = sys_mailbox_create(nodenum)) >= 0);

	pthread_barrier_wait(&barrier);

	TEST_ASSERT(sys_mailbox_unlink(inbox) == 0);

	kernel_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Mailbox Create Unlink
 */
static void test_sys_mailbox_create_unlink(void)
{
	pthread_t tids[mailbox_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < mailbox_ncores; i++)
	{
		TEST_ASSERT((pthread_create(&tids[i],
			NULL,
			test_sys_mailbox_thread_create_unlink,
			NULL)) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < mailbox_ncores; i++)
		pthread_join(tids[i], NULL);
}

/*============================================================================*
 * API Test: Open Close                                                       *
 *============================================================================*/

/**
 * @brief API Test: Mailbox Open Close
 */
static void *test_sys_mailbox_thread_open_close(void *args)
{
	int tid;
	int inbox;
	int outbox;
	int nodenum;

	kernel_setup();

	tid = ((int *)args)[0];

	nodenum = sys_get_node_num();

	TEST_ASSERT((inbox = sys_mailbox_create(nodenum)) >= 0);

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outbox = sys_mailbox_open(
		((tid + 1) == mailbox_ncores) ?
			nodenum + 1 - mailbox_ncores + 1:
			nodenum + 1)) >= 0
	);

	pthread_barrier_wait(&barrier);

	TEST_ASSERT(sys_mailbox_close(outbox) == 0);

	TEST_ASSERT(sys_mailbox_unlink(inbox) == 0);

	kernel_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Mailbox Open Close
 */
static void test_sys_mailbox_open_close(void)
{
	int tids[mailbox_ncores];
	pthread_t threads[mailbox_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < mailbox_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_sys_mailbox_thread_open_close,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < mailbox_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Double Open                                                       *
 *============================================================================*/

/**
 * @brief API Test: Mailbox Double Open
 */
static void *test_sys_mailbox_thread_double_open(void *args)
{
	int tid;
	int inbox;
	int nodenum;
	int outboxes[2];

	kernel_setup();

	tid = ((int *)args)[0];

	nodenum = sys_get_node_num();

	TEST_ASSERT((inbox = sys_mailbox_create(nodenum)) >= 0);

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outboxes[0] = sys_mailbox_open(
		((tid + 1) == mailbox_ncores) ?
			nodenum + 1 - mailbox_ncores + 1 :
			nodenum + 1)) >= 0
	);

	TEST_ASSERT((outboxes[1] = sys_mailbox_open(
		(tid == 0) ?
			nodenum + 1 - mailbox_ncores + 1 :
			nodenum - 1)) >= 0
	);

	pthread_barrier_wait(&barrier);

	TEST_ASSERT(sys_mailbox_close(outboxes[0]) == 0);
	TEST_ASSERT(sys_mailbox_close(outboxes[1]) == 0);

	TEST_ASSERT(sys_mailbox_unlink(inbox) == 0);

	kernel_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Mailbox Double Open
 */
static void test_sys_mailbox_double_open(void)
{
	int tids[mailbox_ncores];
	pthread_t threads[mailbox_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < mailbox_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_sys_mailbox_thread_double_open,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < mailbox_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Read Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Mailbox Read Write
 */
static void *test_sys_mailbox_thread_read_write(void *args)
{
	int tnum;
	int inbox;
	int outbox;
	char buf[MAILBOX_MSG_SIZE];
	int nodenum;

	kernel_setup();

	tnum = ((int *)args)[0];

	/* Build nodes list. */
	nodenum = sys_get_node_num();
	mailbox_nodes[tnum] = nodenum;

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((inbox = sys_mailbox_create(nodenum)) >= 0);

	TEST_ASSERT((outbox = sys_mailbox_open(
		((tnum + 1) == mailbox_ncores) ?
			mailbox_nodes[1]:
			mailbox_nodes[tnum + 1])) >= 0
	);

	pthread_barrier_wait(&barrier);

	memset(buf, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(sys_mailbox_write(outbox, buf, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);

	memset(buf, 0, MAILBOX_MSG_SIZE);
	TEST_ASSERT(sys_mailbox_read(inbox, buf, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);

	for (int i = 0; i < MAILBOX_MSG_SIZE; i++)
		TEST_ASSERT(buf[i] == 1);

	pthread_barrier_wait(&barrier);

	TEST_ASSERT(sys_mailbox_close(outbox) == 0);

	TEST_ASSERT(sys_mailbox_unlink(inbox) == 0);

	kernel_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Mailbox Read Write
 */
static void test_sys_mailbox_read_write(void)
{
	int tids[mailbox_ncores];
	pthread_t threads[mailbox_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < mailbox_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_sys_mailbox_thread_read_write,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < mailbox_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test mailbox_tests_api[] = {
	/* Intra-Cluster API Tests */
	{ test_sys_mailbox_create_unlink, "Create Unlink"          },
	{ test_sys_mailbox_open_close,    "Open Close"             },
	{ test_sys_mailbox_double_open,   "Double Open"            },
	{ test_sys_mailbox_read_write,    "Read Write"             },
	{ NULL,                            NULL                    },
};
