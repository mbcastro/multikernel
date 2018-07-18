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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include <mppaipc.h>

#include <nanvix/syscalls.h>
#include <nanvix/semaphore.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

#include "test.h"

/*===================================================================*
* Fault Injection Test: Invalid Create                               *
*====================================================================*/

/**
* @brief Fault Injection Test: Create Invalid Semaphore
*/
static void test_semaphore_invalid_create(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	char buf[NANVIX_SEM_NAME_MAX + 1];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	memset(buf, 0, NANVIX_SEM_NAME_MAX + 1);

	/* Create invalid semaphores. */
	assert(nanvix_sem_open(NULL, O_CREAT, 0, 0) == (-EINVAL));

	assert(nanvix_sem_open(buf, O_CREAT, 0, 0) == (-EINVAL));

	assert(nanvix_sem_open("cool-name", O_CREAT, 0, (SEM_MAX + 1)) == (-EINVAL));

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Bad Create                                   *
*====================================================================*/

/**
* @brief Fault Injection Test: Create Bad Semaphore
*/
static void test_semaphore_bad_create(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Create bad semaphores. */
	assert(nanvix_sem_open("", O_CREAT, 0, 0) == (-EINVAL));

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Double Create                                *
*====================================================================*/

/**
* @brief Fault Injection Test: Create Double Semaphore
*/
static void test_semaphore_double_create(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Create double semaphores. */
	assert(nanvix_sem_open("cool-name", O_CREAT, 0, 0) >= 0);
	assert(nanvix_sem_open("cool-name", O_CREAT, 0, 0) == SEM_FAILURE);
	assert(nanvix_sem_open("cool-name", (O_CREAT | O_EXCL), 0, 0) == SEM_FAILURE);

	assert(nanvix_sem_unlink("cool-name") == 0);

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Invalid Open                                 *
*====================================================================*/

/**
* @brief Fault Injection Test: Open Invalid Semaphore
*/
static void test_semaphore_invalid_open(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	char buf[NANVIX_SEM_NAME_MAX + 1];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	memset(buf, 0, NANVIX_SEM_NAME_MAX + 1);

	/* Open invalid semaphores. */
	assert(nanvix_sem_open(NULL, 0) == (-EINVAL));
	assert(nanvix_sem_open(buf, 0) == (-EINVAL));

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Bad Open                                     *
*====================================================================*/

/**
* @brief Fault Injection Test: Open Bad Semaphore
*/
static void test_semaphore_bad_open(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Open bad semaphores. */
	assert(nanvix_sem_open("", 0, 0, 0) == (-EINVAL));

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Double Open                                  *
*====================================================================*/

/**
* @brief Fault Injection Test: Open Double Semaphore
*/
static void test_semaphore_double_open(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	int nodenum;
	int inbox;
	int semid;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Open double semaphores. */
	assert((semid = nanvix_sem_open("cool-name", O_CREAT, 0, 0)) >= 0);

	assert(nanvix_sem_close(semid) == 0);

	assert(nanvix_sem_open("cool-name", 0, 0, 0) == semid);

	assert(nanvix_sem_open("cool-name", 0, 0, 0) == SEM_FAILURE);

	assert(nanvix_sem_unlink("cool-name") == 0);

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Invalid Unlink                               *
*====================================================================*/

/**
* @brief Fault Injection Test: Invalid Unlink Semaphore
*/
static void test_semaphore_invalid_unlink(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	char buf[NANVIX_SEM_NAME_MAX + 1];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	memset(buf, 0, NANVIX_SEM_NAME_MAX + 1);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Unlink invalid semaphores. */
	assert(nanvix_sem_unlink("") == (-EINVAL));
	assert(nanvix_sem_unlink(buf) == (-EINVAL));

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Bad Unlink                                   *
*====================================================================*/

/**
* @brief Fault Injection Test: Bad Unlink Semaphore
*/
static void test_semaphore_bad_unlink(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	char buf[NANVIX_SEM_NAME_MAX + 1];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	memset(buf, 0, NANVIX_SEM_NAME_MAX + 1);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Unlink bad semaphores. */
	assert(nanvix_sem_unlink("missing-name") == SEM_FAILURE);

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Double Unlink                                *
*====================================================================*/

/**
* @brief Fault Injection Test: Double Unlink Semaphore
*/
static void test_semaphore_double_unlink(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	char buf[NANVIX_SEM_NAME_MAX + 1];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	memset(buf, 0, NANVIX_SEM_NAME_MAX + 1);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Unlink double semaphores. */
	assert(nanvix_sem_open("cool-name", O_CREAT, 0, 0) >= 0);
	assert(nanvix_sem_unlink("cool-name") == 0);
	assert(nanvix_sem_unlink("cool-name") == SEM_FAILURE);

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Invalid Close                                *
*====================================================================*/

/**
* @brief Fault Injection Test: Invalid Close Semaphore
*/
static void test_semaphore_invalid_close(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Close invalid semaphores. */
	assert(nanvix_sem_close(SEM_MAX + 1) == (-EINVAL));
	assert(nanvix_sem_close(-1) == (-EINVAL));

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Bad Close                                    *
*====================================================================*/

/**
* @brief Fault Injection Test: Bad Close Semaphore
*/
static void test_semaphore_bad_close(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Close bad semaphores. */
	assert(nanvix_sem_close(5) == SEM_FAILURE);

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Double Close                                 *
*====================================================================*/

/**
* @brief Fault Injection Test: Double Close Semaphore
*/
static void test_semaphore_double_close(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	int nodenum;
	int inbox;
	int semid;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Close double semaphore. */
	assert((semid = nanvix_sem_open("cool-name", O_CREAT, 0, 0)) >= 0);
	assert(nanvix_sem_close(semid) == 0);
	assert(nanvix_sem_close(semid) == SEM_FAILURE);
	assert(nanvix_sem_unlink("cool-name") == 0);

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Invalid Wait                                 *
*====================================================================*/

/**
* @brief Fault Injection Test: Invalid Wait Semaphore
*/
static void test_semaphore_invalid_wait(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Wait invalid semaphores. */
	assert(nanvix_sem_wait(SEM_MAX + 1) == (-EINVAL));
	assert(nanvix_sem_wait(-1) == (-EINVAL));

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Bad Wait                                     *
*====================================================================*/

/**
* @brief Fault Injection Test: Bad Wait Semaphore
*/
static void test_semaphore_bad_wait(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Wait bad semaphores. */
	assert(nanvix_sem_wait(5) == SEM_FAILURE);

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Invalid Post                                 *
*====================================================================*/

/**
* @brief Fault Injection Test: Invalid Post Semaphore
*/
static void test_semaphore_invalid_post(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Post invalid semaphores. */
	assert(nanvix_sem_post(SEM_MAX + 1) == (-EINVAL));
	assert(nanvix_sem_post(-1) == (-EINVAL));

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*===================================================================*
* Fault Injection Test: Bad Post                                     *
*====================================================================*/

/**
* @brief Fault Injection Test: Bad Post Semaphore
*/
static void test_semaphore_bad_post(void)
{
	char name[NANVIX_PROC_NAME_MAX];
	int nodenum;
	int inbox;

	nodenum = sys_get_node_num();

	sprintf(name, "%d", nodenum);

	/* Initialize named inbox. */
	assert((inbox = mailbox_create(name)) >= 0);

	/* Post bad semaphores. */
	assert(nanvix_sem_post(5) == SEM_FAILURE);

	/* Unlink named inbox. */
	assert(mailbox_unlink(inbox) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test ipc_semaphore_tests_fault[] = {
	{ test_semaphore_invalid_create, "Invalid Create" },
	{ test_semaphore_bad_create,     "Bad Create"     },
	{ test_semaphore_double_create,  "Double Create"  },
	{ test_semaphore_invalid_open,   "Invalid Open"   },
	{ test_semaphore_bad_open,       "Bad Open"       },
	{ test_semaphore_double_open,    "Double Open"    },
	{ test_semaphore_invalid_unlink, "Invalid Unlink" },
	{ test_semaphore_bad_unlink,     "Bad Unlink"     },
	{ test_semaphore_double_unlink,  "Double unlink"  },
	{ test_semaphore_invalid_close,  "Invalid Close"  },
	{ test_semaphore_bad_close,      "Bad Close"      },
	{ test_semaphore_double_close,   "Double Close"   },
	{ test_semaphore_invalid_wait,   "Invalid Wait"   },
	{ test_semaphore_bad_wait,       "Bad Wait"       },
	{ test_semaphore_invalid_post,   "Invalid Post"   },
	{ test_semaphore_bad_post,       "Bad Post"       },
	{ NULL,                     NULL                  },
};
