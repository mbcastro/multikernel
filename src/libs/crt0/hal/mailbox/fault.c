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

#include <nanvix/syscalls.h>
#include <nanvix/const.h>

#include "test.h"

/*============================================================================*
 * Fault Injection Test: Invalid Create                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Create
 */
static void test_sys_mailbox_invalid_create(void)
{
	int inbox;

	TEST_ASSERT((inbox = sys_mailbox_create(-1)) < 0);
	TEST_ASSERT((inbox = sys_mailbox_create(1000000)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Create                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Create
 */
static void test_sys_mailbox_bad_create(void)
{
	int inbox;

	TEST_ASSERT((inbox = sys_mailbox_create(NAME_SERVER_NODE)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Double Create                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Create
 */
static void test_sys_mailbox_double_create(void)
{
	int inbox;
	int nodenum;

	nodenum = sys_get_node_num();

	TEST_ASSERT((inbox = sys_mailbox_create(nodenum)) >= 0);
	TEST_ASSERT(sys_mailbox_create(nodenum) < 0);

	TEST_ASSERT(sys_mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Open                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Open
 */
static void test_sys_mailbox_invalid_open(void)
{
	int outbox;

	TEST_ASSERT((outbox = sys_mailbox_open(-1)) < 0);
	TEST_ASSERT((outbox = sys_mailbox_open(1000000)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Open                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Open
 */
static void test_sys_mailbox_bad_open(void)
{
	int outbox;
	int nodenum;

	nodenum = sys_get_node_num();

	TEST_ASSERT((outbox = sys_mailbox_open(nodenum)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Double Open                                          *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Open
 */
static void test_sys_mailbox_double_open(void)
{
	int outbox;
	int nodenum;

	nodenum = sys_get_node_num();

	TEST_ASSERT((outbox = sys_mailbox_open(nodenum + 1)) >= 0);
	TEST_ASSERT(sys_mailbox_open(nodenum + 1) < 0);

	TEST_ASSERT(sys_mailbox_close(outbox) == 0);
}

/*============================================================================*
 * Fault Injection Test: Double Unlink                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Unlink
 */
static void test_sys_mailbox_double_unlink(void)
{
	int inbox;
	int nodenum;

	nodenum = sys_get_node_num();

	TEST_ASSERT((inbox = sys_mailbox_create(nodenum)) >= 0);
	TEST_ASSERT(sys_mailbox_unlink(inbox) == 0);
	TEST_ASSERT(sys_mailbox_unlink(inbox) < 0);
}

/*============================================================================*
 * Fault Injection Test: Double Close                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Close
 */
static void test_sys_mailbox_double_close(void)
{
	int outbox;
	int nodenum;

	nodenum = sys_get_node_num();

	TEST_ASSERT((outbox = sys_mailbox_open(nodenum + 1)) >= 0);
	TEST_ASSERT(sys_mailbox_close(outbox) == 0);
	TEST_ASSERT(sys_mailbox_close(outbox) < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Write                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Write
 */
static void test_sys_mailbox_invalid_write(void)
{
	char buf[MAILBOX_MSG_SIZE];

	memset(buf, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(sys_mailbox_write(-1, buf, MAILBOX_MSG_SIZE) != MAILBOX_MSG_SIZE);
	TEST_ASSERT(sys_mailbox_write(100000, buf, MAILBOX_MSG_SIZE) != MAILBOX_MSG_SIZE);
}

/*============================================================================*
 * Fault Injection Test: Bad Write                                            *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Write
 */
static void test_sys_mailbox_bad_write(void)
{
	int inbox;
	char buf[MAILBOX_MSG_SIZE];
	int nodenum;

	nodenum = sys_get_node_num();

	TEST_ASSERT((inbox = sys_mailbox_create(nodenum)) >= 0);

	memset(buf, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(sys_mailbox_write(inbox, buf, 1) != MAILBOX_MSG_SIZE);

	TEST_ASSERT(sys_mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * Fault Injection Test: Null Write                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Null Write
 */
static void test_sys_mailbox_null_write(void)
{
	int outbox;
	int nodenum;

	nodenum = sys_get_node_num();

	TEST_ASSERT((outbox = sys_mailbox_open(nodenum + 1)) >= 0);
	TEST_ASSERT(sys_mailbox_write(outbox, NULL, MAILBOX_MSG_SIZE) != MAILBOX_MSG_SIZE);
	TEST_ASSERT(sys_mailbox_close(outbox) == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Write                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Read
 */
static void test_sys_mailbox_invalid_read(void)
{
	char buf[MAILBOX_MSG_SIZE];

	memset(buf, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(sys_mailbox_read(-1, buf, MAILBOX_MSG_SIZE) != MAILBOX_MSG_SIZE);
	TEST_ASSERT(sys_mailbox_read(100000, buf, MAILBOX_MSG_SIZE) != MAILBOX_MSG_SIZE);
}

/*============================================================================*
 * Fault Injection Test: Bad Read                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Read
 */
static void test_sys_mailbox_bad_read(void)
{
	int outbox;
	char buf[MAILBOX_MSG_SIZE];
	int nodenum;

	nodenum = sys_get_node_num();

	TEST_ASSERT((outbox = sys_mailbox_open(nodenum + 1)) >= 0);

	memset(buf, 1, MAILBOX_MSG_SIZE);
	TEST_ASSERT(sys_mailbox_read(outbox, buf, 1) != MAILBOX_MSG_SIZE);

	TEST_ASSERT(sys_mailbox_close(outbox) == 0);
}

/*============================================================================*
 * Fault Injection Test: Null Read                                            *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Null Read
 */
static void test_sys_mailbox_null_read(void)
{
	int inbox;
	int nodenum;

	nodenum = sys_get_node_num();

	TEST_ASSERT((inbox = sys_mailbox_create(nodenum)) >= 0);
	TEST_ASSERT(sys_mailbox_read(inbox, NULL, MAILBOX_MSG_SIZE) != MAILBOX_MSG_SIZE);
	TEST_ASSERT(sys_mailbox_unlink(inbox) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test mailbox_tests_fault[] = {
	{ test_sys_mailbox_invalid_create, "Invalid Create" },
	{ test_sys_mailbox_bad_create,     "Bad Create"     },
	{ test_sys_mailbox_double_create,  "Double Create"  },
	{ test_sys_mailbox_invalid_open,   "Invalid Open"   },
	{ test_sys_mailbox_bad_open,       "Bad Open"       },
	{ test_sys_mailbox_double_open,    "Double Open"    },
	{ test_sys_mailbox_double_unlink,  "Double Unlink"  },
	{ test_sys_mailbox_double_close,   "Double Close"   },
	{ test_sys_mailbox_invalid_write,  "Invalid Write"  },
	{ test_sys_mailbox_bad_write,      "Bad Write"      },
	{ test_sys_mailbox_null_write,     "Null Write"     },
	{ test_sys_mailbox_invalid_read,   "Invalid Read"   },
	{ test_sys_mailbox_bad_read,       "Bad Read"       },
	{ test_sys_mailbox_null_read,      "Null Read"      },
	{ NULL,                            NULL             },
};
