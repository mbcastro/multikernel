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
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#include <nanvix/syscalls.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_posix_semaphore_create_unlink_cc(int masternode, int nclusters)
{
	sem_t *sem;
	int nodenum;
	int barrier;
	int nodes[nclusters + 1];
	char semaphore_name[NANVIX_SEM_NAME_MAX];

	nodenum = sys_get_node_num();

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, nclusters + 1)) >= 0);

	/* Create and unlink semaphore. */
	sprintf(semaphore_name, "/semaphore%d", nodenum);
	TEST_ASSERT((sem = sem_open(semaphore_name, O_CREAT, 0, 0)) != SEM_FAILED);
	TEST_ASSERT(sem_unlink(semaphore_name) == 0);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Open Close CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Open Close CC
 */
static void test_posix_semaphore_open_close_cc(int masternode, int nclusters)
{
	sem_t *sem;
	int nodenum;
	int barrier;
	int nodes[nclusters + 1];
	char semaphore_name[NANVIX_SEM_NAME_MAX];

	nodenum = sys_get_node_num();

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, nclusters + 1)) >= 0);

	/* Create and unlink semaphore. */
	sprintf(semaphore_name, "/semaphore%d", nodenum);
	TEST_ASSERT((sem = sem_open(semaphore_name, O_CREAT, 0, 0)) != SEM_FAILED);
	TEST_ASSERT((sem = sem_open(semaphore_name, 0)) != SEM_FAILED);
	TEST_ASSERT(sem_close(sem) == 0);
	TEST_ASSERT(sem_unlink(semaphore_name) == 0);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Open Close 2 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Open Close 2 CC
 */
static void test_posix_semaphore_open_close2_cc(int masternode, int nclusters)
{
	sem_t *sem;
	int barrier;
	int nodes[nclusters + 1];

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, nclusters + 1)) >= 0);

	/* Open and close semaphore. */
	TEST_ASSERT((sem = sem_open("/semaphore", 0)) != SEM_FAILED);
	TEST_ASSERT(sem_close(sem) == 0);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Open Close 3 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Open Close 3 CC
 */
static void test_posix_semaphore_open_close3_cc(int masternode, int nclusters)
{
	sem_t *sem;
	int barrier;
	int nodenum;
	int nodes[nclusters + 1];
	char semaphore_name[NANVIX_SEM_NAME_MAX];

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, nclusters + 1)) >= 0);

	nodenum = sys_get_node_num();

	/* Create semaphore. */
	sprintf(semaphore_name, "/semaphore%d", nodenum);
	TEST_ASSERT((sem = sem_open(semaphore_name, O_CREAT, 0, 0)) != SEM_FAILED);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	/* House keeping. */
	TEST_ASSERT(sem_unlink(semaphore_name) == 0);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Wait Post 2 CC                                                   *
 *============================================================================*/

/**
 * @brief API Test: Wait Post 2 CC
 */
static void test_posix_semaphore_wait_post2_cc(int masternode, int nclusters)
{
	sem_t *sem;
	int barrier;
	int nodes[nclusters + 1];

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, nclusters + 1)) >= 0);

	TEST_ASSERT((sem = sem_open("/semaphore", 0)) != SEM_FAILED);
	TEST_ASSERT(sem_wait(sem) == 0);
	TEST_ASSERT(sem_close(sem) == 0);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Wait Post 3 CC                                                   *
 *============================================================================*/

/**
 * @brief API Test: Wait Post 3 CC
 */
static void test_posix_semaphore_wait_post3_cc(int masternode, int nclusters)
{
	sem_t *sem;
	int barrier;
	int nodenum;
	int nodes[nclusters + 1];
	char semaphore_name[NANVIX_SEM_NAME_MAX];

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, nclusters + 1)) >= 0);

	nodenum = sys_get_node_num();

	/* Create semaphore. */
	sprintf(semaphore_name, "/semaphore%d", nodenum);
	TEST_ASSERT((sem = sem_open(semaphore_name, O_CREAT, 0, 1)) != SEM_FAILED);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	TEST_ASSERT(sem_post(sem) == 0);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	/* House keeping. */
	TEST_ASSERT(sem_unlink(semaphore_name) == 0);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*/

/**
 * @brief Mailbox unit test.
 */
int main2(int argc, char **argv)
{
	int test;
	int masternode;
	int nclusters;

	/* Retrieve kernel parameters. */
	TEST_ASSERT(argc == 4);
	masternode = atoi(argv[1]);
	nclusters = atoi(argv[2]);
	test = atoi(argv[3]);

	switch(test)
	{
		/* Create Unlink CC */
		case 0:
			test_posix_semaphore_create_unlink_cc(masternode, nclusters);
			break;

		/* Open Close CC */
		case 1:
			test_posix_semaphore_open_close_cc(masternode, nclusters);
			break;

		/* Open Close 2 CC */
		case 2:
			test_posix_semaphore_open_close2_cc(masternode, nclusters);
			break;

		/* Open Close 3 CC */
		case 3:
			test_posix_semaphore_open_close3_cc(masternode, nclusters);
			break;

		/* Wait Post 2 CC */
		case 4:
			test_posix_semaphore_wait_post2_cc(masternode, nclusters);
			break;

		/* Wait Post 3 CC */
		case 5:
			test_posix_semaphore_wait_post3_cc(masternode, nclusters);
			break;

		/* Should not happen. */
		default:
			return (-EXIT_FAILURE);
			break;
	}

	return (EXIT_SUCCESS);
}
