/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nanvix/arch/mppa.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>
#include <nanvix/hal.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

#define MSG_TEST 0

#if MSG_TEST
static int msg;
#endif

/*===================================================================*
 * API Test: Name Unlink                                             *
 *===================================================================*/

/**
 * @brief API Test: Name Unlink
 */
static void test_name_unlink(void)
{
	int nodeid;
	char pathname[PROC_NAME_MAX];

	nodeid = hal_get_cluster_id();
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
	char pathname[PROC_NAME_MAX];

	nodeid = hal_get_cluster_id();
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
	char pathname[PROC_NAME_MAX];

	nodeid = hal_get_cluster_id();

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
	char pathname[PROC_NAME_MAX];

	nodeid = hal_get_cluster_id();

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
	char pathname[PROC_NAME_MAX + 1];

	nodeid = hal_get_cluster_id();

	memset(pathname, 1, PROC_NAME_MAX + 1);

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
	char pathname[PROC_NAME_MAX + 1];

	memset(pathname, 1, PROC_NAME_MAX + 1);

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

/*====================================================================*
 * main                                                               *
 *====================================================================*/

/**
 * @brief Remote name unit test.
 */
int main(int argc, char **argv)
{
	int nclusters;

#if MSG_TEST
	char pathname[PROC_NAME_MAX];
	char out_pathname[PROC_NAME_MAX];
	int nodeid;
	int barrier;
	int inbox;
	int outbox;
#endif

	/* Retrieve parameters. */
	assert(argc == 2);
	assert((nclusters = atoi(argv[1])) > 0);

#if MSG_TEST
	nodeid = hal_get_cluster_id();
	sprintf(pathname, "/cpu%d", nodeid);
	barrier = barrier_open(nclusters);
#endif

	test_name_link();
	test_name_lookup();
	test_name_unlink();
	test_name_duplicate();
	test_name_invalid_link();
	test_name_invalid_unlink();
	test_name_bad_unlink();

#if MSG_TEST
	/* Register this cluster. */
	assert(name_link(nodeid, pathname) == 0);

	/* Wait for others clusters. */
	barrier_wait(barrier);

	/* Message exchange test using name resolution. */

	assert(nclusters > 1);

	inbox = mailbox_create(pathname);
	sprintf(out_pathname, "/cpu%d", (nodeid + 1)%nclusters);

	outbox = mailbox_open(out_pathname);

	msg = nodeid;
	assert(mailbox_write(outbox, &msg) == 0);

	msg = -1;
	while(msg == -1){
		assert(mailbox_read(inbox, &msg) == 0);
	}

	if (nodeid - 1 < 0)
	{
		assert(msg == (nodeid + nclusters - 1));
	}
	else
	{
		assert(msg == (nodeid - 1)%nclusters);

	}

	/* House keeping. */
	assert(mailbox_close(outbox) == 0);
	assert(mailbox_close(inbox) == 0);
	barrier_close(barrier);
#endif

	return (EXIT_SUCCESS);
}
