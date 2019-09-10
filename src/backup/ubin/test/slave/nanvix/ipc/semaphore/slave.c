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
#include <pthread.h>
#include <semaphore.h>

#include <mppaipc.h>

#include <nanvix/syscalls.h>
#include <nanvix/semaphores.h>
#include <nanvix/limits.h>
#include <nanvix/const.h>
#include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/*===================================================================*
 * API Test: Semaphore Create Unlink                                 *
 *===================================================================*/

/**
 * @brief API Test: Semaphore Create Unlink
 */
static void test_semaphore_slave(int nclusters)
{
	char semaphore_name[NANVIX_SEM_NAME_MAX];
	int semid;
	int nodenum;
	int nodes[nclusters];
	int barrier;

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	assert((barrier = barrier_create(nodes, nclusters)) >= 0);

	nodenum = sys_get_node_num();

	sprintf(semaphore_name, "/semaphore");

	/* Create and unlink semaphore. */
	assert((semid = sem_open(semaphore_name, O_CREAT, 0, 0)) >= 0);

	assert(barrier_wait(barrier) == 0);

	if ((nodenum%2) == 0)
		assert(nanvix_sem_wait(semid) == 0);
	else
		assert(nanvix_sem_post(semid) == 0);

	/* House keeping. */
	assert(barrier_wait(barrier) == 0);
	assert(barrier_unlink(barrier) == 0);
	assert(nanvix_sem_unlink(semaphore_name) == 0);
}

/*====================================================================*
 * main                                                               *
 *====================================================================*/

/**
 * @brief Remote name unit test.
 */
int main2(int argc, char **argv)
{
	int nclusters;
	int barrier;
	char name[NANVIX_PROC_NAME_MAX];
	int nodenum;
	int inbox;

	/* Retrieve parameters. */
	assert(argc == 2);
	assert((nclusters = atoi(argv[1])) > 0);

	int nodes[nclusters + 1];

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Build nodes list. */
	nodes[0] = SPAWNER_SERVER_NODE;

	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	assert((barrier = barrier_create(nodes, nclusters + 1)) >= 0);

	test_semaphore_slave(nclusters);

	assert(barrier_wait(barrier) == 0);

	assert(mailbox_unlink(inbox) == 0);

	return (EXIT_SUCCESS);
}
