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

#include <mppaipc.h>

#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/limits.h>
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

/*============================================================================*
 * API Test: Link Unlink                                                      *
 *============================================================================*/

/**
 * @brief API Test: Link Unlink
 */
static void test_nanvix_ipc_name_link_unlink(void)
{
	int nodenum;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = sys_get_node_num();

	sprintf(pathname, "cool-name");
	TEST_ASSERT(name_link(nodenum, pathname) == 0);
	TEST_ASSERT(name_unlink(pathname) == 0);
}

/*============================================================================*
 * API Test: Lookup                                                           *
 *============================================================================*/

/**
 * @brief API Test: Lookup
 */
static void test_nanvix_ipc_name_lookup(void)
{
	int nodenum;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = sys_get_node_num();

	sprintf(pathname, "cool-name");
	TEST_ASSERT(name_link(nodenum, pathname) == 0);
	TEST_ASSERT(name_lookup(pathname) == nodenum);
	TEST_ASSERT(name_unlink(pathname) == 0);
}

/*============================================================================*
 * API Test: Link Unlink CC                                                   *
 *============================================================================*/

/**
 * @brief API Test: Link Unlink CC
 */
static void test_nanvix_ipc_name_link_unlink_cc(void)
{
	char test_str[4];
	const char *args[] = {
		"/test/ipc-name-slave",
		test_str,
		NULL
	};

	/* Build args. */
	sprintf(test_str, "%d", 0);

	spawn_slaves(args);
	join_slaves();
}

/*============================================================================*
 * API Test: Lookup CC                                                        *
 *============================================================================*/

/**
 * @brief API Test: Lookup CC
 */
static void test_nanvix_ipc_name_lookup_cc(void)
{
	char test_str[4];
	const char *args[] = {
		"/test/ipc-name-slave",
		test_str,
		NULL
	};

	/* Build args. */
	sprintf(test_str, "%d", 1);

	spawn_slaves(args);
	join_slaves();
}

/*============================================================================*/

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test nanvix_ipc_name_tests_api[] = {
	{ test_nanvix_ipc_name_link_unlink,    "Link Unlink"    },
	{ test_nanvix_ipc_name_lookup,         "Lookup"         },
	{ test_nanvix_ipc_name_link_unlink_cc, "Link Unlink CC" },
	{ test_nanvix_ipc_name_lookup_cc,      "Lookup CC"      },
	{ NULL,                  NULL          }
};
