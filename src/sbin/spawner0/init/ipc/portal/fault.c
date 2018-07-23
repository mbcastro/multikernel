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

#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

#include "test.h"

/*============================================================================*
 * API Test: Invalid Create                                                   *
 *============================================================================*/

/**
 * @brief API Test: Invalid Create
 */
static void test_ipc_portal_invalid_create(void)
{
	TEST_ASSERT(portal_create(NULL) < 0);
}

/*============================================================================*
 * API Test: Bad Create                                                       *
 *============================================================================*/

/**
 * @brief API Test: Bad Create
 */
static void *test_ipc_portal_bad_create_thread(void *args)
{
	((void) args);

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	TEST_ASSERT(portal_create("existing-name") < 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Bad Create
 */
static void test_ipc_portal_bad_create(void)
{
	int inportal;
	int tids[ipc_portal_ncores];
	pthread_t threads[ipc_portal_ncores];
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	TEST_ASSERT(portal_create("") < 0);
	TEST_ASSERT(portal_create(pathname) < 0);

	TEST_ASSERT((inportal = portal_create("existing-name")) >= 0);

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_portal_bad_create_thread,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
		pthread_join(threads[i], NULL);

	/* House keeping. */
	TEST_ASSERT(portal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test: Double Create                                                    *
 *============================================================================*/

/**
 * @brief API Test: Double Create
 */
static void test_ipc_portal_double_create(void)
{
	int inportal;

	TEST_ASSERT((inportal = portal_create("cool-name")) >=  0);
	TEST_ASSERT(portal_create("cool-name") < 0);
	TEST_ASSERT(portal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test: Invalid Unlink                                                   *
 *============================================================================*/

/**
 * @brief API Test: Invalid Unlink
 */
static void test_ipc_portal_invalid_unlink(void)
{
	TEST_ASSERT(portal_unlink(-1) < 0);
	TEST_ASSERT(portal_unlink(1000000) < 0);
}

/*============================================================================*
 * API Test: Bad Unlink                                                       *
 *============================================================================*/

/**
 * @brief API Test: Bad Unlink
 */
static void *test_ipc_portal_bad_unlink_thread(void *args)
{
	int tid;
	int inportal;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	tid = ((int *)args)[0];

	if (tid == 1)
		TEST_ASSERT((inportal = portal_create("existing-name")) >= 0);

	pthread_barrier_wait(&barrier);
	pthread_barrier_wait(&barrier);

	/* House keeping. */
	if (tid == 1)
		TEST_ASSERT(portal_unlink(inportal) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Bad Unlink
 */
static void test_ipc_portal_bad_unlink(void)
{
	int outportal;
	int tids[ipc_portal_ncores];
	pthread_t threads[ipc_portal_ncores];

#ifdef _TEST_IPC_PORTAL_BAD_INBOX_UNLINK_
	TEST_ASSERT(portal_unlink(0) < 0);
#endif /* _TEST_IPC_PORTAL_BAD_INBOX_UNLINK_ */
	TEST_ASSERT(portal_unlink(1) < 0);

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_portal_bad_unlink_thread,
			&tids[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outportal = portal_open("existing-name")) >= 0);
	TEST_ASSERT(portal_unlink(outportal) < 0);
	TEST_ASSERT(portal_close(outportal) == 0);

	pthread_barrier_wait(&barrier);

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Double Unlink                                                    *
 *============================================================================*/

/**
 * @brief API Test: Double Unlink
 */
static void test_ipc_portal_double_unlink(void)
{
	int inportal;

	TEST_ASSERT((inportal = portal_create("cool-name")) >=  0);
	TEST_ASSERT(portal_unlink(inportal) == 0);
	TEST_ASSERT(portal_unlink(inportal) < 0);
}

/*============================================================================*
 * API Test: Invalid Open                                                     *
 *============================================================================*/

/**
 * @brief API Test: Invalid Open
 */
static void test_ipc_portal_invalid_open(void)
{
	TEST_ASSERT(portal_open(NULL) < 0);
}

/*============================================================================*
 * API Test: Bad Open                                                         *
 *============================================================================*/

/**
 * @brief API Test: Bad Open
 */
static void test_ipc_portal_bad_open(void)
{
	int inportal;
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	TEST_ASSERT(portal_open("") < 0);
	TEST_ASSERT(portal_open(pathname) < 0);
	TEST_ASSERT(portal_open("missing-name") < 0);
	TEST_ASSERT((inportal = portal_create("cool-name")) >= 0);
	TEST_ASSERT(portal_open("cool-name") < 0);
	TEST_ASSERT(portal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test: Double Open                                                      *
 *============================================================================*/

/**
 * @brief API Test: Double Open
 */
static void *test_ipc_portal_double_open_thread(void *args)
{
	int tid;
	int inportal;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	tid = ((int *)args)[0];

	if (tid == 1)
		TEST_ASSERT((inportal = portal_create("existing-name")) >= 0);

	pthread_barrier_wait(&barrier);
	pthread_barrier_wait(&barrier);

	/* House keeping. */
	if (tid == 1)
		TEST_ASSERT(portal_unlink(inportal) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Double Open
 */
static void test_ipc_portal_double_open(void)
{
	int outportal;
	int tids[ipc_portal_ncores];
	pthread_t threads[ipc_portal_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_portal_double_open_thread,
			&tids[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outportal = portal_open("existing-name")) >= 0);
	TEST_ASSERT(portal_open("existing-name") < 0);
	TEST_ASSERT(portal_close(outportal) == 0);

	pthread_barrier_wait(&barrier);

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Invalid Close                                                    *
 *============================================================================*/

/**
 * @brief API Test: Invalid Close
 */
static void test_ipc_portal_invalid_close(void)
{
	TEST_ASSERT(portal_close(-1) < 0);
	TEST_ASSERT(portal_close(1000000) < 0);
}

/*============================================================================*
 * API Test: Bad Close                                                        *
 *============================================================================*/

/**
 * @brief API Test: Bad Close
 */
static void test_ipc_portal_bad_close(void)
{
	int inportal;

	TEST_ASSERT(portal_close(0) < 0);
	TEST_ASSERT((inportal = portal_create("cool-name")) >=  0);
	TEST_ASSERT(portal_close(inportal) < 0);
	TEST_ASSERT(portal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test: Double Close                                                     *
 *============================================================================*/

/**
 * @brief API Test: Double Close
 */
static void *test_ipc_portal_double_close_thread(void *args)
{
	int tid;
	int inportal;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	tid = ((int *)args)[0];

	if (tid == 1)
		TEST_ASSERT((inportal = portal_create("existing-name")) >= 0);

	pthread_barrier_wait(&barrier);
	pthread_barrier_wait(&barrier);

	/* House keeping. */
	if (tid == 1)
		TEST_ASSERT(portal_unlink(inportal) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Double Close
 */
static void test_ipc_portal_double_close(void)
{
	int outportal;
	int tids[ipc_portal_ncores];
	pthread_t threads[ipc_portal_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_portal_double_close_thread,
			&tids[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outportal = portal_open("existing-name")) >= 0);
	TEST_ASSERT(portal_close(outportal) == 0);
	TEST_ASSERT(portal_close(outportal) < 0);

	pthread_barrier_wait(&barrier);

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Invalid Read                                                     *
 *============================================================================*/

/**
 * @brief API Test: Invalid Read
 */
static void test_ipc_portal_invalid_read(void)
{
	char buffer[DATA_SIZE];

	TEST_ASSERT(portal_read(-1, buffer, DATA_SIZE) < 0);
	TEST_ASSERT(portal_read(1000000, buffer, DATA_SIZE) < 0);
}

/*============================================================================*
 * API Test: Bad Read                                                         *
 *============================================================================*/

/**
 * @brief API Test: Bad Read
 */
static void *test_ipc_portal_bad_read_thread(void *args)
{
	int tid;
	int inportal;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	tid = ((int *)args)[0];

	if (tid == 1)
		TEST_ASSERT((inportal = portal_create("existing-name")) >= 0);

	pthread_barrier_wait(&barrier);
	pthread_barrier_wait(&barrier);

	/* House keeping. */
	if (tid == 1)
		TEST_ASSERT(portal_unlink(inportal) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Bad Read
 */
static void test_ipc_portal_bad_read(void)
{
	int outportal;
	int tids[ipc_portal_ncores];
	char buffer[DATA_SIZE];
	pthread_t threads[ipc_portal_ncores];

	TEST_ASSERT(portal_read(1, buffer, DATA_SIZE) < 0);

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_portal_bad_read_thread,
			&tids[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outportal = portal_open("existing-name")) >= 0);
	TEST_ASSERT(portal_read(outportal, buffer, DATA_SIZE) < 0);
	TEST_ASSERT(portal_close(outportal) == 0);

	pthread_barrier_wait(&barrier);

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Invalid Read Size                                                *
 *============================================================================*/

/**
 * @brief API Test: Invalid Read Size
 */
static void test_ipc_portal_invalid_read_size(void)
{
	int inportal;
	char buffer[DATA_SIZE];

	TEST_ASSERT((inportal = portal_create("cool-name")) >=  0);
	TEST_ASSERT(portal_read(inportal, buffer, -1) < 0);
	TEST_ASSERT(portal_read(inportal, buffer, 0) < 0);
	TEST_ASSERT(portal_read(inportal, buffer, DATA_SIZE - 1) < 0);
	TEST_ASSERT(portal_read(inportal, buffer, DATA_SIZE + 1) < 0);
	TEST_ASSERT(portal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test: Null Read                                                        *
 *============================================================================*/

/**
 * @brief API Test: Null Read
 */
static void test_ipc_portal_null_read(void)
{
	int inportal;

	TEST_ASSERT((inportal = portal_create("cool-name")) >=  0);
	TEST_ASSERT(portal_read(inportal, NULL, DATA_SIZE) < 0);
	TEST_ASSERT(portal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test: Invalid Write                                                    *
 *============================================================================*/

/**
 * @brief API Test: Invalid Write
 */
static void test_ipc_portal_invalid_write(void)
{
	char buffer[DATA_SIZE];

	TEST_ASSERT(portal_write(-1, buffer, DATA_SIZE) < 0);
	TEST_ASSERT(portal_write(1000000, buffer, DATA_SIZE) < 0);
}

/*============================================================================*
 * API Test: Bad Write                                                        *
 *============================================================================*/

/**
 * @brief API Test: Bad Write
 */
static void test_ipc_portal_bad_write(void)
{
	int inportal;
	char buffer[DATA_SIZE];

	TEST_ASSERT(portal_write(0, buffer, DATA_SIZE) < 0);
	TEST_ASSERT((inportal = portal_create("cool-name")) >=  0);
	TEST_ASSERT(portal_write(inportal, buffer, DATA_SIZE) < 0);
	TEST_ASSERT(portal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test: Invalid Write Size                                               *
 *============================================================================*/

/**
 * @brief API Test: Invalid Write Size
 */
static void *test_ipc_portal_invalid_write_size_thread(void *args)
{
	int tid;
	int inportal;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	tid = ((int *)args)[0];

	if (tid == 1)
		TEST_ASSERT((inportal = portal_create("existing-name")) >= 0);

	pthread_barrier_wait(&barrier);
	pthread_barrier_wait(&barrier);

	/* House keeping. */
	if (tid == 1)
		TEST_ASSERT(portal_unlink(inportal) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Invalid Write Size
 */
static void test_ipc_portal_invalid_write_size(void)
{
	int outportal;
	int tids[ipc_portal_ncores];
	char buffer[DATA_SIZE];
	pthread_t threads[ipc_portal_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_portal_invalid_write_size_thread,
			&tids[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outportal = portal_open("existing-name")) >= 0);
	TEST_ASSERT(portal_write(outportal, buffer, -1) < 0);
	TEST_ASSERT(portal_write(outportal, buffer, 0) < 0);
	TEST_ASSERT(portal_write(outportal, buffer, DATA_SIZE - 1) < 0);
	TEST_ASSERT(portal_write(outportal, buffer, DATA_SIZE + 1) < 0);
	TEST_ASSERT(portal_close(outportal) == 0);

	pthread_barrier_wait(&barrier);

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Null Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Null Write
 */
static void *test_ipc_portal_null_write_thread(void *args)
{
	int tid;
	int inportal;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	tid = ((int *)args)[0];

	if (tid == 1)
		TEST_ASSERT((inportal = portal_create("existing-name")) >= 0);

	pthread_barrier_wait(&barrier);
	pthread_barrier_wait(&barrier);

	/* House keeping. */
	if (tid == 1)
		TEST_ASSERT(portal_unlink(inportal) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Null Write
 */
static void test_ipc_portal_null_write(void)
{
	int outportal;
	int tids[ipc_portal_ncores];
	pthread_t threads[ipc_portal_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_portal_null_write_thread,
			&tids[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outportal = portal_open("existing-name")) >= 0);
	TEST_ASSERT(portal_write(outportal, NULL, DATA_SIZE) < 0);
	TEST_ASSERT(portal_close(outportal) == 0);

	pthread_barrier_wait(&barrier);

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_portal_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test ipc_portal_tests_fault[] = {
	{ test_ipc_portal_invalid_create,     "Invalid Create"     },
	{ test_ipc_portal_bad_create,         "Bad Create"         },
	{ test_ipc_portal_double_create,      "Double Create"      },
	{ test_ipc_portal_invalid_unlink,     "Invalid Unlink"     },
	{ NULL,                                NULL                },
	{ test_ipc_portal_bad_unlink,         "Bad Unlink"         },
	{ test_ipc_portal_double_unlink,      "Double Unlink"      },
	{ test_ipc_portal_invalid_open,       "Invalid Open"       },
	{ test_ipc_portal_double_open,        "Double Open"        },
	{ test_ipc_portal_bad_open,           "Bad Open"           },
	{ test_ipc_portal_invalid_close,      "Invalid Close"      },
	{ test_ipc_portal_bad_close,          "Bad Close"          },
	{ test_ipc_portal_double_close,       "Double Close"       },
	{ test_ipc_portal_invalid_read,       "Invalid Read"       },
	{ test_ipc_portal_bad_read,           "Bad Read"           },
	{ test_ipc_portal_invalid_read_size,  "Invalid Read Size"  },
	{ test_ipc_portal_null_read,          "Null Read"          },
	{ test_ipc_portal_invalid_write,      "Invalid Write"      },
	{ test_ipc_portal_bad_write,          "Bad Write"          },
	{ test_ipc_portal_invalid_write_size, "Invalid Write Size" },
	{ test_ipc_portal_null_write,         "Null Write"         },
	{ NULL,                                NULL                },
};
