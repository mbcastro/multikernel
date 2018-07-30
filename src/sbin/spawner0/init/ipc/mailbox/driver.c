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
 * @brief Number of cores in the underlying cluster.
 */
int ipc_mailbox_ncores = 0;

/**
 * @brief Global barrier for synchronization.
 */
pthread_barrier_t barrier;

/**
 * @brief Unnamed Mailbox Test Driver
 *
 * @param nbusycores Number of busy cores.
 */
void test_kernel_ipc_mailbox(int nbusycores)
{
	TEST_ASSERT(runtime_setup(1) == 0);

	ipc_mailbox_ncores = sys_get_num_cores() - nbusycores;

	pthread_barrier_init(&barrier, NULL, ipc_mailbox_ncores);

#ifdef _TEST_API_NAMED_MAILBOX_IOCLUSTER

	/* Run API tests. */
	for (int i = 0; ipc_mailbox_tests_api[i].test_fn != NULL; i++)
	{
		printf("[nanvix][test][api][ipc][mailbox] %s\n", ipc_mailbox_tests_api[i].name);
		ipc_mailbox_tests_api[i].test_fn();
	}

#endif /* _TEST_API_NAMED_MAILBOX_IOCLUSTER */

	/* Run fault injection tests. */
	for (int i = 0; ipc_mailbox_tests_fault[i].test_fn != NULL; i++)
	{
		printf("[nanvix][test][fault][ipc][mailbox] %s\n", ipc_mailbox_tests_fault[i].name);
		ipc_mailbox_tests_fault[i].test_fn();
	}

	TEST_ASSERT(runtime_cleanup() == 0);
}

