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
	UNUSED(a); *(int*) b = EXIT_SUCCESS; UNUSED(c);

	return (0);
}

#endif

#include <nanvix/syscalls.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/limits.h>

#include "test.h"

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

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
	int status = EXIT_SUCCESS;

	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		TEST_ASSERT(mppa_waitpid(pids[i], &status, 0) != -1);
		TEST_ASSERT(status == EXIT_SUCCESS);
	}
}

/*============================================================================*
 * API Test: Alloc                                                            *
 *============================================================================*/

/**
 * @brief API Test: Alloc
 */
static void test_mm_rmem_alloc(void)
{
	TEST_ASSERT(memalloc() == 0);
    TEST_ASSERT(memalloc() == 1);
    TEST_ASSERT(memalloc() == 2);
    TEST_ASSERT(memalloc() == 3);
}

/*============================================================================*
 * API Test: Alloc CC                                                         *
 *============================================================================*/

/**
 * @brief API Test: Alloc CC
 */
static void test_mm_rmem_alloc_cc(void)
{
	int nodenum;
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/mm-rmem-slave",
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
	TEST_ASSERT(barrier_unlink(barrier) == 0);

	join_slaves();
}

/*============================================================================*
 * API Test: Free                                                             *
 *============================================================================*/

/**
 * @brief API Test: Free
 */
static void test_mm_rmem_free(void)
{
    TEST_ASSERT(memfree(1) == 0);
    TEST_ASSERT(memalloc() == 1);
    TEST_ASSERT(memfree(0) == 0);
    TEST_ASSERT(memfree(2) == 0);
    TEST_ASSERT(memalloc() == 0);
    TEST_ASSERT(memalloc() == 2);
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT(memfree(i) == 0);
    }
}

/*============================================================================*
 * API Test: Free CC                                                          *
 *============================================================================*/

/**
 * @brief API Test: Free CC
 */
static void test_mm_rmem_free_cc(void)
{
	int nodenum;
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/mm-rmem-slave",
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

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);

	spawn_slaves(args);

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);

	join_slaves();
}

/*============================================================================*
 * API Test: Read Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Read Write
 */
static void test_mm_rmem_read_write(void)
{
	static char buffer[RMEM_BLOCK_SIZE];

	memset(buffer, 1, RMEM_BLOCK_SIZE);
    memalloc();
	TEST_ASSERT(memwrite(0, buffer, RMEM_BLOCK_SIZE) == 0);

	memset(buffer, 0, RMEM_BLOCK_SIZE);
	TEST_ASSERT(memread(0, buffer, RMEM_BLOCK_SIZE) == 0);
    memfree(0);

	/* Checksum. */
	for (int i = 0; i < RMEM_BLOCK_SIZE; i++)
		TEST_ASSERT(buffer[i] == 1);
}

/*============================================================================*
 * API Test: Read Write CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Read Write CC
 */
static void test_mm_rmem_read_write_cc(void)
{
	int nodenum;
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];
	char masternode_str[4];
	char mailbox_nclusters_str[4];
	char test_str[4];
	const char *args[] = {
		"/test/mm-rmem-slave",
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

    /* Alloc blocks that will be utilized. */
    for (int i = 0; i < NANVIX_PROC_MAX; i++)
        memalloc();

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, NANVIX_PROC_MAX + 1)) >= 0);

	spawn_slaves(args);

	/* Wait for slaves. */
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);

	join_slaves();
}

/*============================================================================*/

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test mm_rmem_tests_api[] = {
	{ test_mm_rmem_alloc,         "Alloc"         },
	{ test_mm_rmem_free,          "Free"          },
	{ test_mm_rmem_alloc_cc,      "Alloc CC"      },
	{ test_mm_rmem_free_cc,       "Free CC"       },
	{ test_mm_rmem_read_write,    "Read Write"    },
	{ test_mm_rmem_read_write_cc, "Read Write CC" },
	{ NULL,                       NULL            },
};
