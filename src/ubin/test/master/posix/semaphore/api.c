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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <semaphore.h>

#ifdef _KALRAY_MPPA256

#include <mppaipc.h>

#else

#define UNUSED(x) ((void)(x))

static inline int mppa_spawn(int a, void *b, const void *c, void *d, void *e)
{
	UNUSED(a); UNUSED(b); UNUSED(c); UNUSED(d); UNUSED(e);
	return (0);
}

static inline int mppa_waitpid(int a, void *b, int c)
{
	UNUSED(a); UNUSED(b); UNUSED(c);

	return (0);
}

#endif

#define __NEED_HAL_BARRIER_
#include <nanvix/limits.h>
#include <nanvix/syscalls.h>
#include <nanvix/pm.h>

#include "test.h"

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

/**
 * @brief PID of slaves.
 */
static int pids[NANVIX_PROC_MAX];

/**
 * @brief Spawn slave processes.
 */
static void spawn_slaves(const char **args)
{
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		TEST_ASSERT((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);
}

/**
 * @brief Wait for slaves to terminate.
 */
static void join_slaves(void)
{
	int status;

	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		TEST_ASSERT(mppa_waitpid(pids[i], &status, 0) != -1);
		TEST_ASSERT(status == EXIT_SUCCESS);
	}
}

/*==========================================================================*
 * API Test: Create Unlink                                                  *
 *==========================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_semaphore_create_unlink(void)
{
	sem_t *sem;
	char semaphore_name[NANVIX_SEM_NAME_MAX];

	/* Create and unlink semaphore. */
	sprintf(semaphore_name, "/semaphore");
	TEST_ASSERT((sem = sem_open(semaphore_name, O_CREAT, (S_IRUSR | S_IWUSR), 0)) != SEM_FAILED);
	TEST_ASSERT(sem_unlink(semaphore_name) == 0);
}

/*==========================================================================*
 * API Test: Open Close                                                     *
 *==========================================================================*/

/**
 * @brief API Test: Open Close
 */
static void test_posix_semaphore_open_close(void)
{
	sem_t *sem;
	char semaphore_name[NANVIX_SEM_NAME_MAX];

	sprintf(semaphore_name, "/semaphore");
	TEST_ASSERT((sem = sem_open(semaphore_name, O_CREAT, (S_IRUSR | S_IWUSR), 0)) != SEM_FAILED);
	TEST_ASSERT((sem = sem_open(semaphore_name, 0)) != SEM_FAILED);
	TEST_ASSERT(sem_close(sem) == 0);
	TEST_ASSERT(sem_unlink(semaphore_name) == 0);
}

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_posix_semaphore_create_unlink_cc(void)
{
	int nodenum;
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/posix-semaphore-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	nodenum = sys_get_node_num();

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodenum);
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 0);

	/* Build nodes list. */
	nodes[0] = nodenum;
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);

	spawn_slaves(args);

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Open Close CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Open Close CC
 */
static void test_posix_semaphore_open_close_cc(void)
{
	int nodenum;
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/posix-semaphore-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	nodenum = sys_get_node_num();

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodenum);
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 1);

	/* Build nodes list. */
	nodes[0] = nodenum;
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);

	spawn_slaves(args);

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Open Close 2 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Open Close 2 CC
 */
static void test_posix_semaphore_open_close2_cc(void)
{
	int nodenum;
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	sem_t *sem;
	char semaphore_name[NANVIX_SEM_NAME_MAX];
	const char *args[] = {
		"/test/posix-semaphore-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	nodenum = sys_get_node_num();

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodenum);
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 2);

	/* Build nodes list. */
	nodes[0] = nodenum;
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	/* Create semaphore. */
	sprintf(semaphore_name, "/semaphore");
	TEST_ASSERT((sem = sem_open(semaphore_name, O_CREAT, (S_IRUSR | S_IWUSR), 0)) != SEM_FAILED);

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);

	spawn_slaves(args);

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(sem_unlink(semaphore_name) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Open Close 3 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Open Close 3 CC
 */
static void test_posix_semaphore_open_close3_cc(void)
{
	int nodenum;
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	sem_t *sem;
	char semaphore_name[NANVIX_SEM_NAME_MAX];
	const char *args[] = {
		"/test/posix-semaphore-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	nodenum = sys_get_node_num();

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodenum);
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 3);

	/* Build nodes list. */
	nodes[0] = nodenum;
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);

	spawn_slaves(args);

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		sprintf(semaphore_name, "/semaphore%d", i);
		TEST_ASSERT((sem = sem_open(semaphore_name, 0)) != SEM_FAILED);
		TEST_ASSERT(sem_close(sem) == 0);
	}

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Open Close 4 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Open Close 4 CC
 */
static void test_posix_semaphore_open_close4_cc(void)
{
	int nodenum;
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/posix-semaphore-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	nodenum = sys_get_node_num();

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodenum);
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 4);

	/* Build nodes list. */
	nodes[0] = nodenum;
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);

	spawn_slaves(args);

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_wait(barrier) == 0);

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Wait Post CC                                                     *
 *============================================================================*/

/**
 * @brief API Test: Wait Post CC
 */
static void test_posix_semaphore_wait_post_cc(void)
{
	int nodenum;
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/posix-semaphore-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	nodenum = sys_get_node_num();

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodenum);
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 5);

	/* Build nodes list. */
	nodes[0] = nodenum;
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);

	spawn_slaves(args);

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_wait(barrier) == 0);

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Wait Post 2 CC                                                   *
 *============================================================================*/

/**
 * @brief API Test: Wait Post 2 CC
 */
static void test_posix_semaphore_wait_post2_cc(void)
{
	int nodenum;
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	sem_t *sem;
	char semaphore_name[NANVIX_SEM_NAME_MAX];
	const char *args[] = {
		"/test/posix-semaphore-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	nodenum = sys_get_node_num();

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodenum);
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 6);

	/* Build nodes list. */
	nodes[0] = nodenum;
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	/* Create semaphore. */
	sprintf(semaphore_name, "/semaphore");
	TEST_ASSERT((sem = sem_open(semaphore_name, O_CREAT, (S_IRUSR | S_IWUSR), 1)) != SEM_FAILED);

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);

	spawn_slaves(args);

	/* Post on semaphore. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		TEST_ASSERT(sem_post(sem) == 0);

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(sem_unlink(semaphore_name) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Wait Post 3 CC                                                   *
 *============================================================================*/

/**
 * @brief API Test: Wait Post 3 CC
 */
static void test_posix_semaphore_wait_post3_cc(void)
{
	int nodenum;
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	sem_t *sem;
	char semaphore_name[NANVIX_SEM_NAME_MAX];
	const char *args[] = {
		"/test/posix-semaphore-slave",
		masternode_str,
		mailbox_nclusters_str,
		test_str,
		NULL
	};

	nodenum = sys_get_node_num();

	/* Build arguments. */
	sprintf(masternode_str, "%d", nodenum);
	sprintf(mailbox_nclusters_str, "%d", NANVIX_PROC_MAX);
	sprintf(test_str, "%d", 7);

	/* Build nodes list. */
	nodes[0] = nodenum;
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);

	spawn_slaves(args);

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		sprintf(semaphore_name, "/semaphore%d", i);
		TEST_ASSERT((sem = sem_open(semaphore_name, 0)) != SEM_FAILED);
		TEST_ASSERT(sem_wait(sem) == 0);
		TEST_ASSERT(sem_close(sem) == 0);
	}

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	join_slaves();

	/* House keeping. */
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test posix_semaphore_tests_api[] = {
	{ test_posix_semaphore_create_unlink,    "Create Unlink"    },
	{ test_posix_semaphore_open_close,       "Open Close"       },
	{ test_posix_semaphore_create_unlink_cc, "Create Unlink CC" },
	{ test_posix_semaphore_open_close_cc,    "Open Close CC"    },
	{ test_posix_semaphore_open_close2_cc,   "Open Close 2 CC"  },
	{ test_posix_semaphore_open_close3_cc,   "Open Close 3 CC"  },
	{ test_posix_semaphore_open_close4_cc,   "Open Close 4 CC"  },
	{ test_posix_semaphore_wait_post_cc,     "Wait Post CC"     },
	{ test_posix_semaphore_wait_post2_cc,    "Wait Post 2 CC"   },
	{ test_posix_semaphore_wait_post3_cc,    "Wait Post 3 CC"   },
	{ NULL,                                  NULL               },
};
