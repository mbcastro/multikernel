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

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_PORTAL_
#include <nanvix/hal.h>

#include "test.h"

/*============================================================================*
 * Fault Injection Test: Invalid Create                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Create
 */
static void test_hal_portal_invalid_create(void)
{
	int inbox;

	TEST_ASSERT((inbox = hal_portal_create(-1)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Create                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Create
 */
static void test_hal_portal_bad_create(void)
{
	int inbox;

	TEST_ASSERT((inbox = hal_portal_create(0)) < 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test portal_tests_fault[] = {
	{ test_hal_portal_invalid_create, "Invalid Create" },
	{ test_hal_portal_bad_create,     "Bad Create"     },
	{ NULL,                           NULL             },
};
