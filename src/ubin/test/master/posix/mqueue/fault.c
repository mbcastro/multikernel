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

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>

#include <nanvix/limits.h>
#include <nanvix/syscalls.h>
#include <nanvix/mqueue.h>

#include "test.h"

/*============================================================================*
 * Fault Injection Test: Invalid Create                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Create
 */
static void test_posix_mqueue_invalid_create(void)
{
	char buf[NANVIX_MQUEUE_NAME_MAX + 1];

	memset(buf, 'a', NANVIX_MQUEUE_NAME_MAX + 1);
	buf[NANVIX_MQUEUE_NAME_MAX] = '\0';

	/* Create invalid mqueues. */
	TEST_ASSERT(mq_open(NULL, O_CREAT, 0) < 0);
	TEST_ASSERT(mq_open(buf, O_CREAT, 0) < 0);
	TEST_ASSERT(mq_open(NULL, (O_CREAT | O_EXCL), 0) < 0);
	TEST_ASSERT(mq_open(buf, (O_CREAT | O_EXCL), 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Create                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Create 
 */
static void test_posix_mqueue_bad_create(void)
{
	TEST_ASSERT(mq_open("", O_CREAT, 0) < 0);
	TEST_ASSERT(mq_open("", (O_CREAT | O_EXCL), 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Double Create                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Create 
 */
static void test_posix_mqueue_double_create(void)
{
	int mqueue;

	TEST_ASSERT((mqueue = mq_open("cool-name", O_CREAT, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(mq_open("cool-name", (O_CREAT | O_EXCL), S_IRUSR | S_IWUSR) < 0);
	TEST_ASSERT(mq_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Open                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Open
 */
static void test_posix_mqueue_invalid_open(void)
{
	char buf[NANVIX_MQUEUE_NAME_MAX + 1];

	memset(buf, 'a', NANVIX_MQUEUE_NAME_MAX + 1);
	buf[NANVIX_MQUEUE_NAME_MAX] = '\0';

	/* Open invalid mqueues. */
	TEST_ASSERT(mq_open(NULL, 0, 0) < 0);
	TEST_ASSERT(mq_open(buf, 0, 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Open                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Open 
 */
static void test_posix_mqueue_bad_open(void)
{
	TEST_ASSERT(mq_open("", 0, 0) < 0);
	TEST_ASSERT(mq_open("cool-name", 0, 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Unlink                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Unlink
 */
static void test_posix_mqueue_invalid_unlink(void)
{
	char buf[NANVIX_MQUEUE_NAME_MAX + 1];

	memset(buf, 'a', NANVIX_MQUEUE_NAME_MAX + 1);
	buf[NANVIX_MQUEUE_NAME_MAX] = '\0';

	/* Unlink invalid mqueues. */
	TEST_ASSERT(mq_unlink(NULL) < 0);
	TEST_ASSERT(mq_unlink(buf) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Unlink                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Unlink 
 */
static void test_posix_mqueue_bad_unlink(void)
{
	TEST_ASSERT(mq_unlink("") < 0);
	TEST_ASSERT(mq_unlink("missing-name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Double Unlink                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Unlink 
 */
static void test_posix_mqueue_double_unlink(void)
{
	int mqueue;

	TEST_ASSERT((mqueue = mq_open("cool-name", O_CREAT, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(mq_unlink("cool-name") == 0);
	TEST_ASSERT(mq_unlink("cool-name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Send                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Send
 */
static void test_posix_mqueue_invalid_send(void)
{
	char msg[MQUEUE_MESSAGE_SIZE];

	memset(msg, 1, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(-1, msg, MQUEUE_MESSAGE_SIZE, 0) < 0);
	TEST_ASSERT(mq_send(MQUEUE_OPEN_MAX + 1, msg, MQUEUE_MESSAGE_SIZE, 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Send                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Send
 */
static void test_posix_mqueue_bad_send(void)
{
	char msg[MQUEUE_MESSAGE_SIZE];

	memset(msg, 1, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(0, msg, MQUEUE_MESSAGE_SIZE, 0) < 0);
	TEST_ASSERT(mq_send(0, NULL, MQUEUE_MESSAGE_SIZE, 0) < 0);
	TEST_ASSERT(mq_send(0, NULL, 0, 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Null Send                                            *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Null Send
 */
static void test_posix_mqueue_null_send(void)
{
	int queue;

	TEST_ASSERT((queue = mq_open("cool-name", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) >= 0);

	TEST_ASSERT(mq_send(queue, NULL, MQUEUE_MESSAGE_SIZE, 0) < 0);

	TEST_ASSERT(mq_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Size                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Size
 */
static void test_posix_mqueue_invalid_send_size(void)
{
	int queue;
	char msg[MQUEUE_MESSAGE_SIZE];

	TEST_ASSERT((queue = mq_open("cool-name", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) >= 0);

	TEST_ASSERT(mq_send(queue, msg, 0, 0) < 0);
	TEST_ASSERT(mq_send(queue, msg, -1, 0) < 0);
	TEST_ASSERT(mq_send(queue, msg, MQUEUE_MESSAGE_SIZE + 1, 0) < 0);

	TEST_ASSERT(mq_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Priority                                     *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Priority
 */
static void test_posix_mqueue_invalid_priority(void)
{
	int queue;
	char msg[MQUEUE_MESSAGE_SIZE];

	TEST_ASSERT((queue = mq_open("cool-name", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) >= 0);

	memset(msg, 1, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(queue, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX) < 0);
	TEST_ASSERT(mq_send(queue, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX + 1) < 0);
	TEST_ASSERT(mq_send(queue, msg, MQUEUE_MESSAGE_SIZE, -1) < 0);

	TEST_ASSERT(mq_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Receive                                      *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Receive
 */
static void test_posix_mqueue_invalid_receive(void)
{
	unsigned prio;
	char msg[MQUEUE_MESSAGE_SIZE];

	memset(msg, 1, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(-1, msg, MQUEUE_MESSAGE_SIZE, &prio) < 0);
	TEST_ASSERT(mq_receive(MQUEUE_OPEN_MAX + 1, msg, MQUEUE_MESSAGE_SIZE, &prio) < 0);
	TEST_ASSERT(mq_receive(0, msg, MQUEUE_MESSAGE_SIZE, NULL) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Receive                                          *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Receive
 */
static void test_posix_mqueue_bad_receive(void)
{
	unsigned prio;
	char msg[MQUEUE_MESSAGE_SIZE];

	memset(msg, 1, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(0, msg, MQUEUE_MESSAGE_SIZE, &prio) < 0);
	TEST_ASSERT(mq_receive(0, NULL, MQUEUE_MESSAGE_SIZE, &prio) < 0);
	TEST_ASSERT(mq_receive(0, NULL, 0, 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Null Receive                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Null Receive
 */
static void test_posix_mqueue_null_receive(void)
{
	unsigned prio;
	int queue;

	TEST_ASSERT((queue = mq_open("cool-name", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) >= 0);

	TEST_ASSERT(mq_receive(queue, NULL, MQUEUE_MESSAGE_SIZE, &prio) < 0);

	TEST_ASSERT(mq_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Recv Size                                    *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Recv Size
 */
static void test_posix_mqueue_invalid_recv_size(void)
{
	unsigned prio;
	int queue;
	char msg[MQUEUE_MESSAGE_SIZE];

	TEST_ASSERT((queue = mq_open("cool-name", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) >= 0);

	TEST_ASSERT(mq_receive(queue, msg, 0, &prio) < 0);
	TEST_ASSERT(mq_receive(queue, msg, -1, &prio) < 0);
	TEST_ASSERT(mq_receive(queue, msg, MQUEUE_MESSAGE_SIZE + 1, &prio) < 0);

	TEST_ASSERT(mq_unlink("cool-name") == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test posix_mqueue_tests_fault[] = {
	{ test_posix_mqueue_invalid_create,    "Invalid Create"    },
	{ test_posix_mqueue_bad_create,        "Bad Create"        },
	{ test_posix_mqueue_double_create,     "Double Create"     },
	{ test_posix_mqueue_invalid_open,      "Invalid Open"      },
	{ test_posix_mqueue_bad_open,          "Bad Open"          },
	{ test_posix_mqueue_invalid_unlink,    "Invalid Unlink"    },
	{ test_posix_mqueue_bad_unlink,        "Bad Unlink"        },
	{ test_posix_mqueue_double_unlink,     "Double Unlink"     },
	{ test_posix_mqueue_invalid_send,      "Invalid Send"      },
	{ test_posix_mqueue_bad_send,          "Bad Send"          },
	{ test_posix_mqueue_null_send,         "Null Send"         },
	{ test_posix_mqueue_invalid_send_size, "Invalid Send Size" },
	{ test_posix_mqueue_invalid_priority,  "Invalid Priority"  },
	{ test_posix_mqueue_invalid_receive,   "Invalid Receive"   },
	{ test_posix_mqueue_bad_receive,       "Bad Receive"       },
	{ test_posix_mqueue_null_receive,      "Null Receive"      },
	{ test_posix_mqueue_invalid_recv_size, "Invalid Recv Size" },
	{ NULL,                                NULL                },
};
