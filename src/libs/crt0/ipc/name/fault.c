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

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SYNC_
#include <nanvix/syscalls.h>
#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

#include "test.h"

/*============================================================================*
* Fault Injection Test: Duplicate Name                                        *
 *============================================================================*/

/**
* @brief Fault Injection Test: Link the Same Name Twice
*/
static void test_name_duplicate(void)
{
	int nodeid;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodeid = sys_get_node_id();

	/* Link name. */
	sprintf(pathname, "cool-name");
	TEST_ASSERT(name_link(nodeid, pathname) == 0);
	TEST_ASSERT(name_link(nodeid, pathname) < 0);
	TEST_ASSERT(name_unlink(pathname) == 0);
}

/*============================================================================*
* Fault Injection Test: Invalid Link                                          *
 *============================================================================*/

/**
* @brief Fault Injection Test: Link Invalid Names
*/
static void test_name_invalid_link(void)
{
	int nodeid;
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	nodeid = sys_get_node_id();

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	/* Link invalid names. */
	TEST_ASSERT(name_link(nodeid, pathname) < 0);
	TEST_ASSERT(name_link(nodeid, NULL) < 0);
	TEST_ASSERT(name_link(nodeid, "") < 0);
}

/*============================================================================*
* Fault Injection Test: Invalid Unlink                                        *
 *============================================================================*/

/**
* @brief Fault Injection Test: Unlink Invalid Name
*/
static void test_name_invalid_unlink(void)
{
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	/* Unlink invalid names. */
	TEST_ASSERT(name_unlink(pathname) < 0);
	TEST_ASSERT(name_unlink(NULL) < 0);
	TEST_ASSERT(name_unlink("") < 0);
}

/*============================================================================*
* Fault Injection Test: Bad Unlink                                            *
 *============================================================================*/

/**
* @brief Fault Injection Test: Unlink Bad Name
*/
static void test_name_bad_unlink(void)
{
	/* Unlink missing name. */
	TEST_ASSERT(name_unlink("missing_name") < 0);
}

/*============================================================================*
* Fault Injection Test: Bad Lookup                                            *
 *============================================================================*/

/**
* @brief Fault Injection Test: Lookup Missing Name
*/
static void test_name_bad_lookup(void)
{
	/* Lookup missing name. */
	TEST_ASSERT(name_lookup("missing_name") < 0);
}

/*============================================================================*
* Fault Injection Test: Invalid Lookup                                        *
 *============================================================================*/

/**
* @brief Fault Injection Test: Lookup Invalid Name
*/
static void test_name_invalid_lookup(void)
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
struct test ipc_name_tests_fault[] = {
	{ test_name_duplicate,      "Link Duplicate Name" },
	{ test_name_invalid_link,   "Invalid Link"        },
	{ test_name_invalid_unlink, "Invalid Unlink"      },
	{ test_name_bad_unlink,     "Bad Unlink"          },
	{ test_name_bad_lookup,     "Bad Lookup"          },
	{ test_name_invalid_lookup, "Invalid Lookup"      },
	{ NULL,                     NULL                  },
};
