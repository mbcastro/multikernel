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

#include <nanvix/arch/mppa.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>
#include <nanvix/hal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/*===================================================================*
 * API Test: Name Unlink                                             *
 *===================================================================*/

/**
 * @brief API Test: Name Unlink
 */
static void test_name_unlink(void)
{
	int nodeid;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodeid = hal_get_node_id();
	sprintf(pathname, "/cpu%d", nodeid);

	/* Unregister this cluster. */
	TEST_ASSERT(name_unlink(pathname) == 0);
}

/*===================================================================*
 * API Test: Name Link                                               *
 *===================================================================*/

/**
 * @brief API Test: Name Link
 */
static void test_name_link(void)
{
	int nodeid;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodeid = hal_get_node_id();
	sprintf(pathname, "/cpu%d", nodeid);

	/* Register this cluster. */
	TEST_ASSERT(name_link(nodeid, pathname) == 0);
}

/*===================================================================*
 * API Test: Name Lookup                                             *
 *===================================================================*/

/**
 * @brief API Test: master name lookup.
 */
static void test_name_lookup(void)
{
	int nodeid;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodeid = hal_get_node_id();

	sprintf(pathname, "/cpu%d", nodeid);

	TEST_ASSERT(name_lookup(pathname) == nodeid);
}

/*===================================================================*
 * Fault Injection Test: duplicate name                                 *
 *===================================================================*/

/**
 * @brief Fault Injection Test: link the same name twice
 */
static void test_name_duplicate(void)
{
	int nodeid;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodeid = hal_get_node_id();

	sprintf(pathname, "/cpu%d", nodeid);

	/* Link name. */
	TEST_ASSERT(name_link(nodeid, pathname) == 0);
	TEST_ASSERT(name_link(nodeid, pathname) < 0);
	TEST_ASSERT(name_link(nodeid, "test") < 0);
	TEST_ASSERT(name_unlink(pathname) == 0);
}

/*===================================================================*
* Fault Injection Test: invalid link                                 *
*====================================================================*/

/**
* @brief Fault Injection Test: Link invalid names
*/
static void test_name_invalid_link(void)
{
	int nodeid;
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	nodeid = hal_get_node_id();

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	/* Link invalid names. */
	TEST_ASSERT(name_link(nodeid, pathname) < 0);
	TEST_ASSERT(name_link(nodeid, NULL) < 0);
	TEST_ASSERT(name_link(nodeid, "") < 0);
}

/*===================================================================*
* Fault Injection Test: invalid unlink                                 *
*====================================================================*/

/**
* @brief Fault Injection Test: Unlink invalid names
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

/*===================================================================*
* Fault Injection Test: bad unlink                                 *
*====================================================================*/

/**
* @brief Fault Injection Test: Unlink bad name
*/
static void test_name_bad_unlink(void)
{
	/* Unlink bad name. */
	TEST_ASSERT(name_unlink("missing_name") < 0);
}

/*===================================================================*
* Fault Injection Test: Bad lookup                                 *
*====================================================================*/

/**
* @brief Fault Injection Test: Lookup missing name
*/
static void test_name_bad_lookup(void)
{
	/* Lookup missing name. */
	TEST_ASSERT(name_lookup("missing_name") < 0);
}

/*===================================================================*
* Fault Injection Test: Invalid lookup                                 *
*====================================================================*/

/**
* @brief Fault Injection Test: Lookup invalid names
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

/*====================================================================*
 * main                                                               *
 *====================================================================*/

/**
 * @brief Remote name unit test.
 */
int main(int argc, char **argv)
{
	int nclusters;

	/* Retrieve parameters. */
	TEST_ASSERT(argc == 2);
	TEST_ASSERT((nclusters = atoi(argv[1])) > 0);

	TEST_ASSERT(kernel_setup() == 0);

	test_name_link();
	test_name_lookup();
	test_name_unlink();
	test_name_duplicate();
	test_name_invalid_link();
	test_name_invalid_unlink();
	test_name_bad_unlink();
	test_name_bad_lookup();
	test_name_invalid_lookup();

	TEST_ASSERT(kernel_cleanup() == 0);
	return (EXIT_SUCCESS);
}
