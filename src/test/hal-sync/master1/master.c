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

#include <assert.h>
#include <stdlib.h>

#include <mppa/osconfig.h>

#include <nanvix/hal.h>
#include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/*============================================================================*
 * API Test: Barrier Mode                                                     *
 *============================================================================*/

/**
 * @brief API Test: Barrier
 */
static void test_hal_sync_barrier(void)
{
	int syncid;
	int syncid_local;
	int nodes[2];
	int nodes_local[2];

	nodes[0] = 192;
	nodes[1] = 128;

	nodes_local[0] = 128;
	nodes_local[1] = 192;

	/* Open syncrhonization points. */
	TEST_ASSERT((syncid_local = hal_sync_create(nodes_local, 2, HAL_SYNC_ONE_TO_ALL)) >= 0);
	TEST_ASSERT((syncid = hal_sync_open(nodes, 2, HAL_SYNC_ONE_TO_ALL)) >= 0);

	TEST_ASSERT(hal_sync_wait(syncid_local) == 0);
	TEST_ASSERT(hal_sync_signal(syncid) == 0);

	/* House keeping. */
	TEST_ASSERT(hal_sync_unlink(syncid_local) == 0);
	TEST_ASSERT(hal_sync_close(syncid) == 0);
}

/*============================================================================*
 * HAL Sync Test Driver                                                       *
 *============================================================================*/

/**
 * @brief HAL Sync Test Driver
 */
int main(int argc, const char **argv)
{
	((void) argc);
	((void) argv);

	hal_setup();

	/* API tests. */
	test_hal_sync_barrier();

	hal_cleanup();
	return (EXIT_SUCCESS);
}
