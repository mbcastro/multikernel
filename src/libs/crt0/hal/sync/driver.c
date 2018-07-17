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

#include <pthread.h>
#include <stdio.h>

#include <nanvix/syscalls.h>

#include "test.h"

/**
 * @brief Number of remote clusters.
 */
int sync_nclusters = 0;

/**
 * @brief Number of cores in the underlying cluster.
 */
int ncores = 0;

/**
 * @brief Nodes list.
 */
int nodes[HAL_NR_NOC_NODES];

/**
 * @brief Global barrier for synchronization.
 */
pthread_barrier_t barrier;

/**
 * @brief Synchronization Point Test Driver
 */
void test_kernel_sys_sync(void)
{
	ncores = sys_get_num_cores();

	pthread_barrier_init(&barrier, NULL, ncores - 1);

	/* Run API tests. */
	for (int i = 0; tests_api[i].test_fn != NULL; i++)
	{
		printf("[nanvix][test][api][hal][sync] %s\n", tests_api[i].name);
		tests_api[i].test_fn();
	}

	/* Run fault injection tests. */
	for (int i = 0; tests_fault[i].test_fn != NULL; i++)
	{
		printf("[nanvix][test][fault][hal][sync] %s\n", tests_fault[i].name);
		tests_fault[i].test_fn();
	}
}

