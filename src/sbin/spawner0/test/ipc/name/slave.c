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

#include <nanvix/syscalls.h>
#include <nanvix/name.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/*===================================================================*
 * API Test: Link Unlink CC                                          *
 *===================================================================*/

/**
 * @brief API Test: Link Unlink CC
 */
static void test_name_link_unlink(void)
{
	int nodenum;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = sys_get_node_num();
	sprintf(pathname, "/cpu%d", nodenum);

	TEST_ASSERT(name_link(nodenum, pathname) == 0);
	TEST_ASSERT(name_unlink(pathname) == 0);
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

	nodeid = sys_get_node_num();

	sprintf(pathname, "/cpu%d", nodeid);

	TEST_ASSERT(name_lookup(pathname) == nodeid);
}

/*============================================================================*/

/**
 * @brief Naming Service Test Driver
 */
int main2(int argc, char **argv)
{
	int test;
	int nclusters;

	/* Retrieve kernel parameters. */
	TEST_ASSERT(argc == 3);
	nclusters = atoi(argv[1]);
	test = atoi(argv[2]);

	switch (test)
	{
		case 0:
			test_name_link_unlink();
			break;
		case 1:
			test_name_lookup();
			break;
		default:
			exit(EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
