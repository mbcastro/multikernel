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
#include <pthread.h>

#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Data Size.
 */
#define DATA_SIZE 128

/**
 * @brief ID of master node.
 */
static int masternode;

/**
 * @brief Underlying NoC node ID.
 */
static int nodenum;

/**
 * @brief Data buffer.
 */
static char buffer[DATA_SIZE];

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

/**
 * @brief Sync slaves.
 *
 * @param nclusters Number of slaves.
 */
static void sync_slaves(int nclusters)
{
	int barrier;
	int nodes[nclusters];

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, nclusters)) >= 0);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	/* House keeping. */
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/**
 * @brief Sync with master.
 *
 * @poaram nclusters Number of slaves.
 */
static void sync_master(int nclusters)
{
	int barrier;
	int nodes[NANVIX_PROC_MAX + 1];

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, nclusters + 1)) >= 0);

	/* Sync. */
	TEST_ASSERT(barrier_wait(barrier) == 0);

	/* House keeping. */
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_ipc_portal_create_unlink(int nclusters)
{
	int inportal;
	char pathname[NANVIX_PROC_NAME_MAX];

	sprintf(pathname, "ccluster%d", (nodenum + 1)%nclusters);
	TEST_ASSERT((inportal = portal_create(pathname)) >= 0);
	TEST_ASSERT(portal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test: Open Close CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Open Close CC
 */
static void test_ipc_portal_open_close_cc(int nclusters)
{
	int inportal;
	int outportal;
	char pathname[NANVIX_PROC_NAME_MAX];

	sprintf(pathname, "ccluster%d", nodenum);
	TEST_ASSERT((inportal = portal_create(pathname)) >= 0);

	sync_slaves(nclusters);

	sprintf(pathname, "ccluster%d", (nodenum + 1)%nclusters);
	TEST_ASSERT((outportal = portal_open(pathname)) >= 0);

	sync_slaves(nclusters);

	/* House keeping. */
	TEST_ASSERT(portal_close(outportal) == 0);
	TEST_ASSERT(portal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test: Read Write CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Read Write CC
 */
static void test_ipc_portal_read_write_cc(int nclusters)
{
	int inportal;
	int outportal;
	char pathname[NANVIX_PROC_NAME_MAX];

	sprintf(pathname, "ccluster%d", nodenum);
	TEST_ASSERT((inportal = portal_create(pathname)) >= 0);

	sync_slaves(nclusters);

	sprintf(pathname, "ccluster%d", (nodenum + 1)%nclusters);
	TEST_ASSERT((outportal = portal_open(pathname)) >= 0);

	if (nodenum != 0)
	{
		TEST_ASSERT((portal_allow(
			inportal,
			(nodenum == 0) ?
				nclusters - 1 :
				(nodenum - 1)%nclusters)) == 0
		);
	}

	if (nodenum != (nclusters - 1))
	{
		TEST_ASSERT((portal_write(
			outportal,
			buffer,
			DATA_SIZE) == DATA_SIZE)
		);
	}

	if (nodenum != 0)
	{
		TEST_ASSERT((portal_read(
			inportal,
			buffer,
			DATA_SIZE) == DATA_SIZE)
		);
	}

	/* House keeping. */
	TEST_ASSERT(portal_close(outportal) == 0);
	TEST_ASSERT(portal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test: Read Write 2 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Read Write 2 CC
 */
static void test_ipc_portal_read_write2_cc(int nclusters)
{
	int inportal;
	char pathname[NANVIX_PROC_NAME_MAX];

	sprintf(pathname, "ccluster%d", (nodenum + 1)%nclusters);
	TEST_ASSERT((inportal = portal_create(pathname)) >= 0);

	sync_master(nclusters);

	TEST_ASSERT((portal_allow(
		inportal,
		masternode) == 0)
	);

	TEST_ASSERT((portal_read(
		inportal,
		buffer,
		DATA_SIZE) == DATA_SIZE)
	);

	/* House keeping. */
	TEST_ASSERT(portal_unlink(inportal) == 0);
}

/*============================================================================*/

/**
 * @brief HAL Mailbox Test Driver
 */
int main2(int argc, char **argv)
{
	int test;
	int nclusters;

	/* Retrieve kernel parameters. */
	TEST_ASSERT(argc == 4);
	masternode = atoi(argv[1]);
	nclusters = atoi(argv[2]);
	test = atoi(argv[3]);

	nodenum = sys_get_node_num();

	switch (test)
	{
		/* Create Unlink CC */
		case 0:
			test_ipc_portal_create_unlink(nclusters);
			break;

		/* Open Close CC */
		case 1:
			test_ipc_portal_open_close_cc(nclusters);
			break;

		/* Read Write CC */
		case 2:
			test_ipc_portal_read_write_cc(nclusters);
			break;

		/* Read Write 2 CC */
		case 3:
			test_ipc_portal_read_write2_cc(nclusters);
			break;

		/* Should not happen. */
		default:
			exit(EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
