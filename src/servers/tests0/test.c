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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Kernel unit-tests. */
extern void test_hal_core(void);
extern void test_hal_sync(void);
extern void test_hal_mailbox(void);
extern void test_hal_portal(void);

/* Runtime unit-tests. */
extern void test_name(int);
extern void test_mailbox(int);

/**
 * @brief Generic test driver.
 */
void test_kernel(const char *module)
{
	printf("[nanvix][spawner0] running low-level self-tests\n");

	if (!strcmp(module, "--hal-core"))
		test_hal_core();
	else if (!strcmp(module, "--hal-sync"))
		test_hal_sync();
	else if (!strcmp(module, "--hal-mailbox"))
		test_hal_mailbox();
	else if (!strcmp(module, "--hal-portal"))
		test_hal_portal();

	exit(EXIT_SUCCESS);
}

/**
 * @brief Generic test driver.
 */
void test_runtime(const char *module, int nservers)
{
	printf("[nanvix][spawner0] running high-level self-tests\n");

	if (!strcmp(module, "--name"))
		test_name(nservers);
	else if (!strcmp(module, "--mailbox"))
		test_mailbox(nservers);

	exit(EXIT_SUCCESS);
}
