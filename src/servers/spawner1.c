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

#include <mppa/osconfig.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_MAILBOX_
#include <nanvix/config.h>
#include <nanvix/hal.h>

/* Unit-tests. */
extern void test_hal_sync(void);
extern void test_hal_mailbox(void);

/**
 * @brief Generic test driver.
 */
static void test(const char *module)
{
	((void)module);

	printf("[nanvix][spawner1] running self-tests\n");

	test_hal_sync();
	test_hal_mailbox();
}

/**
 * @brief Resolves process names.
 */
int main(int argc, char **argv)
{
	int debug = 0;

	/* Debug mode? */
	if (argc >= 2)
	{
		if (!strcmp(argv[1] , "--debug"))
			debug = 1;
	}

	hal_setup();

	printf("[nanvix][spawner1] booting up server\n");

	/* Run self-tests. */
	if (debug)
		test(argv[2]);

	printf("[nanvix][spawner1] server alive\n");

	while(1);

	hal_cleanup();
	return (EXIT_SUCCESS);
}
