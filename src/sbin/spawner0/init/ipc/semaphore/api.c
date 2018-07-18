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
#include <nanvix/semaphore.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

#include "test.h"

/*===================================================================*
 * API Test: Semaphore Create Unlink                                 *
 *===================================================================*/

/**
 * @brief API Test: Semaphore Create Unlink
 */
static void *test_semaphore_thread_create_unlink(void *args)
{
	char name[NANVIX_PROC_NAME_MAX];
	char semaphore_name[NANVIX_SEM_NAME_MAX];
	int nodenum;
	int semid;
	int inbox;

	((void) args);

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(2) == 0);

	pthread_barrier_wait(&ipc_semaphore_barrier);

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	sprintf(semaphore_name, "/semaphore");

	/* Initialize named inbox. */
	TEST_ASSERT((inbox = mailbox_create(name)) >= 0);

	/* Create and unlink semaphore. */
	TEST_ASSERT((semid = nanvix_sem_open(semaphore_name, O_CREAT, 0, 0)) >= 0);

	pthread_barrier_wait(&ipc_semaphore_barrier);

	TEST_ASSERT(nanvix_sem_unlink(semaphore_name) == 0);

	pthread_barrier_wait(&ipc_semaphore_barrier);

	/* Unlink named inbox. */
	TEST_ASSERT(mailbox_unlink(inbox) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return(NULL);
}

/**
 * @brief API Test: Semaphore Create Unlink
 */
static void test_semaphore_create_unlink(void)
{
	int dmas[ipc_semaphore_ncores];
	pthread_t tids[ipc_semaphore_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_semaphore_ncores; i++)
	{
		dmas[i] = i;
		TEST_ASSERT((pthread_create(&tids[i],
			NULL,
			test_semaphore_thread_create_unlink,
			&dmas[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_semaphore_ncores; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * API Test: Semaphore Open Close                                    *
 *===================================================================*/

/**
 * @brief API Test: Semaphore Open Close
 */
static void *test_semaphore_thread_open_close(void *args)
{
	char name[NANVIX_PROC_NAME_MAX];
	char semaphore_name[NANVIX_SEM_NAME_MAX];
	int nodenum;
	int semid;
	int inbox;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(2) == 0);

	pthread_barrier_wait(&ipc_semaphore_barrier);

	((void) args);

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	sprintf(semaphore_name, "/semaphore");

	/* Initialize named inbox. */
	TEST_ASSERT((inbox = mailbox_create(name)) >= 0);

	/* Create and unlink semaphore. */
	TEST_ASSERT((semid = nanvix_sem_open(semaphore_name, O_CREAT, 0, 0)) >= 0);

	pthread_barrier_wait(&ipc_semaphore_barrier);

	TEST_ASSERT(nanvix_sem_close(semid) == 0);

	pthread_barrier_wait(&ipc_semaphore_barrier);

	TEST_ASSERT(nanvix_sem_open(semaphore_name, 0, 0, 0) == semid);

	pthread_barrier_wait(&ipc_semaphore_barrier);

	TEST_ASSERT(nanvix_sem_unlink(semaphore_name) == 0);

	/* Unlink named inbox. */
	TEST_ASSERT(mailbox_unlink(inbox) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return(NULL);
}

/**
 * @brief API Test: Semaphore Open Close
 */
static void test_semaphore_open_close(void)
{
	int dmas[ipc_semaphore_ncores];
	pthread_t tids[ipc_semaphore_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_semaphore_ncores; i++)
	{
		dmas[i] = i;
		TEST_ASSERT((pthread_create(&tids[i],
			NULL,
			test_semaphore_thread_open_close,
			&dmas[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_semaphore_ncores; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * API Test: Semaphore Wait Post                                     *
 *===================================================================*/

/**
 * @brief API Test: Semaphore Wait Post
 */
static void *test_semaphore_thread_wait_post(void *args)
{
	char name[NANVIX_PROC_NAME_MAX];
	char semaphore_name[NANVIX_SEM_NAME_MAX];
	int tid;
	int nodenum;
	int semid;
	int inbox;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(2) == 0);

	pthread_barrier_wait(&ipc_semaphore_barrier);

	tid = ((int *)args)[0];

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	sprintf(semaphore_name, "/semaphore");

	/* Initialize named inbox. */
	TEST_ASSERT((inbox = mailbox_create(name)) >= 0);

	/* Create and unlink semaphore. */
	TEST_ASSERT((semid = nanvix_sem_open(semaphore_name, O_CREAT, 0,
							 ((ipc_semaphore_ncores - 1)/2))) >= 0);

	pthread_barrier_wait(&ipc_semaphore_barrier);

	if ((tid%2) == 0)
	{
		TEST_ASSERT(nanvix_sem_wait(semid) == 0);
	}
	else
	{
		TEST_ASSERT(nanvix_sem_post(semid) == 0);
	}

	TEST_ASSERT(nanvix_sem_unlink(semaphore_name) == 0);

	/* Unlink named inbox. */
	TEST_ASSERT(mailbox_unlink(inbox) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return(NULL);
}

/**
 * @brief API Test: Semaphore Wait Post
 */
static void test_semaphore_wait_post(void)
{
	int dmas[ipc_semaphore_ncores];
	pthread_t tids[ipc_semaphore_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_semaphore_ncores; i++)
	{
		dmas[i] = i;
		TEST_ASSERT((pthread_create(&tids[i],
			NULL,
			test_semaphore_thread_wait_post,
			&dmas[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_semaphore_ncores; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * API Test: slave tests                                             *
 *===================================================================*/

/**
* @brief API Test: Slave Tests
*/
static void test_semaphore_slave(void)
{
	int status;
	int barrier_slave;
	int pids[NANVIX_PROC_MAX];
	int nodes[ipc_semaphore_nclusters + 1];

	/* Build nodes list. */
	nodes[0] = sys_get_node_num();

	for (int i = 0; i < ipc_semaphore_nclusters; i++)
		nodes[i + 1] = i;

	TEST_ASSERT((barrier_slave = barrier_create(nodes, ipc_semaphore_nclusters + 1)) >= 0);

	char ipc_semaphore_nclusters_str[4];
	const char *args[] = {
		"/test/semaphore-slave",
		ipc_semaphore_nclusters_str,
		NULL
	};

	sprintf(ipc_semaphore_nclusters_str, "%d", ipc_semaphore_nclusters);

	for (int i = 0; i < ipc_semaphore_nclusters; i++)
		TEST_ASSERT((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);

	TEST_ASSERT(barrier_wait(barrier_slave) == 0);

	for (int i = 0; i < ipc_semaphore_nclusters; i++)
	{
		TEST_ASSERT(mppa_waitpid(pids[i], &status, 0) != -1);
		TEST_ASSERT(status == EXIT_SUCCESS);
	}
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test ipc_semaphore_tests_api[] = {
	{ test_semaphore_create_unlink, "Create Unlink" },
	{ test_semaphore_open_close,    "Open Close"    },
	{ test_semaphore_wait_post ,    "Wait Post"     },
	{ test_semaphore_slave,         "Slaves Test"   },
	{ NULL,                         NULL            },
};
