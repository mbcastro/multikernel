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

/* Forward definitions. */
extern void test_sys_sync(void);
extern void test_sys_mailbox(void);
extern void test_sys_portal(void);
extern void test_nanvix_ipc_name(void);
extern void test_nanvix_ipc_barrier(void);
extern void test_nanvix_ipc_mailbox(void);
extern void test_nanvix_ipc_portal(void);
extern void test_mm_rmem(void);
extern void test_semaphore(void);
extern void test_shm(void);

/**
 * @brief Launches automated tests.
 */
int main2(int argc, const char **argv)
{
	/* Missing parameters. */
	if (argc != 3)
		return (EXIT_FAILURE);

	/* Bad usage. */
	if (strcmp(argv[1] , "--debug"))
		return (EXIT_FAILURE);

	/* Launch test driver. */
	if (!strcmp(argv[2], "--hal-sync"))
		test_sys_sync();
	else if (!strcmp(argv[2], "--hal-mailbox"))
		test_sys_mailbox();
	else if (!strcmp(argv[2], "--hal-portal"))
		test_sys_portal();
	else if (!strcmp(argv[2], "--name"))
		test_nanvix_ipc_name();
	else if (!strcmp(argv[2], "--barrier"))
		test_nanvix_ipc_barrier();
	else if (!strcmp(argv[2], "--mailbox"))
		test_nanvix_ipc_mailbox();
	else if (!strcmp(argv[2], "--portal"))
		test_nanvix_ipc_portal();
	else if (!strcmp(argv[2], "--rmem"))
		test_mm_rmem();
	else if (!strcmp(argv[2], "--semaphore"))
		test_semaphore();
	else if (!strcmp(argv[2], "--shm"))
		test_shm();

	return (EXIT_SUCCESS);
}