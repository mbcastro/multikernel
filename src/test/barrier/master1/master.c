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

 #include <mppa/osconfig.h>
 #include <mppaipc.h>

 #define __NEED_HAL_CORE_
 #define __NEED_HAL_NOC_
 #define __NEED_HAL_SYNC_

 #include <nanvix/hal.h>
 #include <nanvix/init.h>
 #include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Number of cores in the underlying cluster.
 */
static int ncores = 0;

/*===================================================================*
 * API Test: Create Unlink                                           *
 *===================================================================*/

/**
 * @brief API Test: Barrier Create Unlink
 */
static void *test_barrier_thread_io(void *args)
{
	int nodeid;
	int barrier;
	int nodes[8] = {192, 129, 130, 131, 128, 193, 194, 195};

	((void) args);

	TEST_ASSERT(kernel_setup() == 0);

	nodeid = hal_get_node_id();

	TEST_ASSERT((barrier = barrier_create(nodes, 8)) >= 0);

	printf("Node %d wait...\n", nodeid);

	TEST_ASSERT(barrier_wait(barrier) == 0);

	printf("Node %d passed the barrier.\n", nodeid);

	TEST_ASSERT(barrier_unlink(barrier) == 0);

	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Barrier Create Unlink
 */
static void test_barrier_io(void)
{
	pthread_t threads[ncores];
	int nodes[8] = {192, 129, 130, 131, 128, 193, 194, 195};
	int barrier;
	int nodeid;

	printf("[test][api] Barrier Create Unlink IO1\n");

	nodeid = hal_get_node_id();

	TEST_ASSERT((barrier = barrier_create(nodes, 8)) >= 0);

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_barrier_thread_io,
			NULL)) == 0
		);
	}

	printf("Node %d wait...\n", nodeid);

	TEST_ASSERT(barrier_wait(barrier) == 0);

	printf("Node %d passed the barrier.\n", nodeid);

	TEST_ASSERT(barrier_unlink(barrier) == 0);

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(threads[i], NULL);
}

/*===================================================================*
 * API Test: Barrier Driver                                          *
 *===================================================================*/

/**
 * @brief Barrier test driver.
 */
int main(int argc, const char **argv)
{
	// int nclusters;

	TEST_ASSERT(argc == 2);
	((void) argv);
	/* Retrieve kernel parameters. */
	// nclusters = atoi(argv[1]);

	TEST_ASSERT(kernel_setup() == 0);

	ncores = hal_get_num_cores();

	/* API tests. */
	test_barrier_io();

	TEST_ASSERT(kernel_cleanup() == 0);
	return (EXIT_SUCCESS);
}
