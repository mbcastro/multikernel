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
#include <mqueue.h>
#include <string.h>

#define __NEED_HAL_BARRIER_
#include <nanvix/limits.h>
#include <nanvix/mqueues.h>

#include "test.h"

/*============================================================================*
 * API Test: Create Unlink                                                    *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_mqueue_create_unlink(void)
{
	mqd_t mqueue;
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((mqueue = mq_open(mqueue_name, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}

/*============================================================================*
 * API Test: Create Unlink 2                                                  *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_mqueue_create_unlink2(void)
{
	mqd_t mqueue;
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((mqueue = mq_open(mqueue_name, (O_CREAT | O_EXCL) | O_RDONLY, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}

/*============================================================================*
 * API Test: Create Unlink 3                                                  *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_mqueue_create_unlink3(void)
{
	mqd_t mqueue;
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((mqueue = mq_open(mqueue_name, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(mq_open(mqueue_name, (O_CREAT | O_EXCL) | O_RDONLY, S_IRUSR | S_IWUSR) < 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}

/*============================================================================*
 * API Test: Create Unlink 4                                                  *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_mqueue_create_unlink4(void)
{
	mqd_t mqueue1, mqueue2;
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((mqueue1 = mq_open(mqueue_name, (O_CREAT | O_EXCL) | O_RDONLY, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT((mqueue2 = mq_open(mqueue_name, (O_CREAT | O_RDONLY), S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}

/*============================================================================*
 * API Test: Open Close 1                                                     *
 *============================================================================*/

/**
 * @brief API Test: Open Close 1
 */
static void test_posix_mqueue_open_close1(void)
{
	mqd_t mqueue1, mqueue2;
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((mqueue1 = mq_open(mqueue_name, O_CREAT, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT((mqueue2 = mq_open(mqueue_name, O_RDONLY, 0)) >= 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}

/*============================================================================*
 * API Test: Open Close 2                                                     *
 *============================================================================*/

/**
 * @brief API Test: Open Close 2
 */
static void test_posix_mqueue_open_close2(void)
{
	mqd_t mqueue1, mqueue2;
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((mqueue1 = mq_open(mqueue_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT((mqueue2 = mq_open(mqueue_name, O_WRONLY, 0)) >= 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}

/*============================================================================*
 * API Test: Send Receive 1                                                   *
 *============================================================================*/

/**
 * @brief API Test: Send Receive 1
 */
static void test_posix_mqueue_send_receive1(void)
{
	unsigned prio;
	int inqueue, outqueue;
	char msg[MQUEUE_MESSAGE_SIZE];
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((inqueue = mq_open(mqueue_name, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT((outqueue = mq_open(mqueue_name, O_WRONLY, 0)) >= 0);

	memset(msg, 1, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 1);

	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}

/*============================================================================*
 * API Test: Send Receive 2                                                   *
 *============================================================================*/

/**
 * @brief API Test: Send Receive 2
 */
static void test_posix_mqueue_send_receive2(void)
{
	unsigned prio;
	int inqueue, outqueue;
	char msg[MQUEUE_MESSAGE_SIZE];
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((inqueue = mq_open(mqueue_name, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT((outqueue = mq_open(mqueue_name, O_WRONLY, 0)) >= 0);

	memset(msg, 1, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);
	memset(msg, 2, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);

	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 1);
	
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 2);

	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}
/*============================================================================*
 * API Test: Send Receive 2                                                   *
 *============================================================================*/

/**
 * @brief API Test: Send Receive 3
 */
static void test_posix_mqueue_send_receive3(void)
{
	unsigned prio;
	int inqueue, outqueue;
	char msg[MQUEUE_MESSAGE_SIZE];
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((inqueue = mq_open(mqueue_name, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT((outqueue = mq_open(mqueue_name, O_WRONLY, 0)) >= 0);

	memset(msg, 1, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);
	memset(msg, 2, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue, msg, MQUEUE_MESSAGE_SIZE, 0) == 0);

	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 1);
	
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == 0);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 2);

	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}

/*============================================================================*
 * API Test: Send Receive 4                                                   *
 *============================================================================*/

/**
 * @brief API Test: Send Receive 4
 */
static void test_posix_mqueue_send_receive4(void)
{
	unsigned prio;
	int inqueue, outqueue;
	char msg[MQUEUE_MESSAGE_SIZE];
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((inqueue = mq_open(mqueue_name, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT((outqueue = mq_open(mqueue_name, O_WRONLY, 0)) >= 0);

	memset(msg, 1, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue, msg, MQUEUE_MESSAGE_SIZE, 0) == 0);
	memset(msg, 2, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);

	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 2);
	
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == 0);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 1);

	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}

/*============================================================================*
 * API Test: Send Receive 5                                                   *
 *============================================================================*/

/**
 * @brief API Test: Send Receive 5
 */
static void test_posix_mqueue_send_receive5(void)
{
	unsigned prio;
	int inqueue, outqueue;
	char msg[MQUEUE_MESSAGE_SIZE];
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((inqueue = mq_open(mqueue_name, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT((outqueue = mq_open(mqueue_name, O_WRONLY, 0)) >= 0);

	memset(msg, 2, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 2) == 0);
	memset(msg, 3, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue, msg, MQUEUE_MESSAGE_SIZE, 0) == 0);
	memset(msg, 1, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);

	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 1);
	
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 2);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 2);
	
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == 0);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 3);

	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}

/*============================================================================*
 * API Test: Multiple Senders                                                 *
 *============================================================================*/

/**
 * @brief API Test: Multiple Senders
 */
static void test_posix_mqueue_multiple_senders(void)
{
	unsigned prio;
	int inqueue, outqueue1, outqueue2, outqueue3;
	char msg[MQUEUE_MESSAGE_SIZE];
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((inqueue = mq_open(mqueue_name, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT((outqueue1 = mq_open(mqueue_name, O_WRONLY, 0)) >= 0);
	TEST_ASSERT((outqueue2 = mq_open(mqueue_name, O_WRONLY, 0)) >= 0);
	TEST_ASSERT((outqueue3 = mq_open(mqueue_name, O_WRONLY, 0)) >= 0);

	/* Sending. */
	memset(msg, 1, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue1, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);
	memset(msg, 2, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue2, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);
	memset(msg, 3, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue3, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);

	/* Receiving. */
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 1);
	
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 2);
	
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 3);

	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}

/*============================================================================*
 * API Test: Multiple Receivers                                               *
 *============================================================================*/

/**
 * @brief API Test: Multiple Receivers
 */
static void test_posix_mqueue_multiple_receivers(void)
{
	unsigned prio;
	int inqueue1, inqueue2, inqueue3, outqueue;
	char msg[MQUEUE_MESSAGE_SIZE];
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((inqueue1 = mq_open(mqueue_name, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT((inqueue2 = mq_open(mqueue_name, O_RDONLY, 0)) >= 0);
	TEST_ASSERT((inqueue3 = mq_open(mqueue_name, O_RDONLY, 0)) >= 0);
	TEST_ASSERT((outqueue = mq_open(mqueue_name, O_WRONLY, 0)) >= 0);

	/* Sending. */
	memset(msg, 1, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);
	memset(msg, 2, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);
	memset(msg, 3, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);

	/* Receiving. */
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue1, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 1);
	
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue2, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 2);
	
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue3, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 3);

	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}

/*============================================================================*
 * API Test: Senders/Receivers                                                *
 *============================================================================*/

/**
 * @brief API Test: Senders/Receivers
 */
static void test_posix_mqueue_senders_receivers(void)
{
	unsigned prio;
	int inqueue1, inqueue2, outqueue1, outqueue2;
	char msg[MQUEUE_MESSAGE_SIZE];
	char mqueue_name[NANVIX_MQUEUE_NAME_MAX];

	sprintf(mqueue_name, "/mqueue");
	TEST_ASSERT((inqueue1 = mq_open(mqueue_name, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT((inqueue2 = mq_open(mqueue_name, O_RDONLY, 0)) >= 0);
	TEST_ASSERT((outqueue1 = mq_open(mqueue_name, O_WRONLY, 0)) >= 0);
	TEST_ASSERT((outqueue2 = mq_open(mqueue_name, O_WRONLY, 0)) >= 0);

	/* Sending. */
	memset(msg, 1, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue1, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);
	memset(msg, 2, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_send(outqueue2, msg, MQUEUE_MESSAGE_SIZE, MQ_PRIO_MAX - 1) == 0);

	/* Receiving. */
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue1, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 1);
	
	memset(msg, 0, MQUEUE_MESSAGE_SIZE);
	TEST_ASSERT(mq_receive(inqueue2, msg, MQUEUE_MESSAGE_SIZE, &prio) == MQUEUE_MESSAGE_SIZE);

	/* Sanity check. */
	TEST_ASSERT(prio == MQ_PRIO_MAX - 1);
	for (int i = 0; i < MQUEUE_MESSAGE_SIZE; i++)
		TEST_ASSERT(msg[i] == 2);

	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
	TEST_ASSERT(mq_unlink(mqueue_name) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test posix_mqueue_tests_api[] = {
	{ test_posix_mqueue_create_unlink,      "Create Unlink"      },
	{ test_posix_mqueue_create_unlink2,     "Create Unlink 2"    },
	{ test_posix_mqueue_create_unlink3,     "Create Unlink 3"    },
	{ test_posix_mqueue_create_unlink4,     "Create Unlink 4"    },
	{ test_posix_mqueue_open_close1,        "Open Close 1"       },
	{ test_posix_mqueue_open_close2,        "Open Close 2"       },
	{ test_posix_mqueue_send_receive1,      "Send Receive 1"     },
	{ test_posix_mqueue_send_receive2,      "Send Receive 2"     },
	{ test_posix_mqueue_send_receive3,      "Send Receive 3"     },
	{ test_posix_mqueue_send_receive4,      "Send Receive 4"     },
	{ test_posix_mqueue_send_receive5,      "Send Receive 5"     },
	{ test_posix_mqueue_multiple_senders,   "Multiple Senders"   },
	{ test_posix_mqueue_multiple_receivers, "Multiple Receivers" },
	{ test_posix_mqueue_senders_receivers,  "Senders/Receivers"  },
	{ NULL,                             NULL              },
};
