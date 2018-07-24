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

#include <nanvix/syscalls.h>
#include <nanvix/pm.h>
#include <nanvix/mm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Data Size.
 */
#define DATA_SIZE 128

/*============================================================================*
 * API Test: Read Write CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Read Write CC
 */
static void test_mm_rmem_read_write_cc(int masternode, int nclusters)
{
	int nodenum;
	int barrier;
	int nodes[nclusters + 1];
	char buffer[DATA_SIZE];

	nodenum = sys_get_node_num();

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create barrier. */
	TEST_ASSERT((barrier = barrier_create(nodes, nclusters + 1)) >= 0);

	memset(buffer, 1, DATA_SIZE);
	memwrite(nodenum*DATA_SIZE, buffer, DATA_SIZE);

	memset(buffer, 0, DATA_SIZE);
	memread(nodenum*DATA_SIZE, buffer, DATA_SIZE);

	/* Checksum. */
	for (int i = 0; i < DATA_SIZE; i++)
		TEST_ASSERT(buffer[i] == 1);

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
		/* Read Write CC */
		case 0:
			test_mm_rmem_read_write_cc(masternode, nclusters);
			break;

		/* Should not happen. */
		default:
			return (-EXIT_FAILURE);
			break;
	}

	return (EXIT_SUCCESS);
}
