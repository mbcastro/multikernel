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
#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

#include "test.h"

/*============================================================================*
 * Fault Injection Test: Double Link                                          *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Link
 */
static void test_nanvix_ipc_name_double_link(void)
{
	int nodenum;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = sys_get_node_num();

	/* Link name. */
	sprintf(pathname, "cool-name");
	TEST_ASSERT(name_link(nodenum, pathname) == 0);
	TEST_ASSERT(name_link(nodenum, pathname) < 0);
	TEST_ASSERT(name_unlink(pathname) == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Link                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Link 
 */
static void test_nanvix_ipc_name_invalid_link(void)
{
	/* Link invalid names. */
	TEST_ASSERT(name_link(-1, "missing_name") < 0);
	TEST_ASSERT(name_link(1000000, "missing_name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Link                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Link
 */
static void test_nanvix_ipc_name_bad_link(void)
{
	int nodenum;
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	nodenum = sys_get_node_num();

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	/* Link invalid names. */
	TEST_ASSERT(name_link(nodenum, pathname) < 0);
	TEST_ASSERT(name_link(nodenum, NULL) < 0);
	TEST_ASSERT(name_link(nodenum, "") < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Unlink                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Unlink
 */
static void test_nanvix_ipc_name_invalid_unlink(void)
{
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	/* Unlink invalid names. */
	TEST_ASSERT(name_unlink(pathname) < 0);
	TEST_ASSERT(name_unlink(NULL) < 0);
	TEST_ASSERT(name_unlink("") < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Unlink                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Unlink
 */
static void test_nanvix_ipc_name_bad_unlink(void)
{
	int nodenum;

	nodenum = sys_get_node_num();

	/* Unlink missing name. */
	TEST_ASSERT(name_link(nodenum, "cool-name") == 0);
	TEST_ASSERT(name_unlink("missing_name") < 0);
	TEST_ASSERT(name_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Double Unlink                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Unlink
 */
static void test_nanvix_ipc_name_double_unlink(void)
{
	int nodenum;

	nodenum = sys_get_node_num();

	/* Unlink missing name. */
	TEST_ASSERT(name_link(nodenum, "cool-name") == 0);
	TEST_ASSERT(name_unlink("cool-name") == 0);
	TEST_ASSERT(name_unlink("cool-name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Lookup                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Lookup
 */
static void test_nanvix_ipc_name_bad_lookup(void)
{
	/* Lookup missing name. */
	TEST_ASSERT(name_lookup("missing_name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Lookup                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Lookup Invalid Name
 */
static void test_nanvix_ipc_name_invalid_lookup(void)
{
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	/* Lookup invalid names. */
	TEST_ASSERT(name_lookup(pathname) < 0);
	TEST_ASSERT(name_lookup(NULL) < 0);
	TEST_ASSERT(name_lookup("") < 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test nanvix_ipc_name_tests_fault[] = {
	{ test_nanvix_ipc_name_invalid_link,   "Invalid Link"   },
	{ test_nanvix_ipc_name_bad_link,       "Bad Link"       },
	{ test_nanvix_ipc_name_double_link,    "Double Link"    },
	{ test_nanvix_ipc_name_invalid_unlink, "Invalid Unlink" },
	{ test_nanvix_ipc_name_bad_unlink,     "Bad Unlink"     },
	{ test_nanvix_ipc_name_double_unlink,  "Double Unlink"  },
	{ test_nanvix_ipc_name_invalid_lookup, "Invalid Lookup" },
	{ test_nanvix_ipc_name_bad_lookup,     "Bad Lookup"     },
	{ NULL,                     NULL             },
};
