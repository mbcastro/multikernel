/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>
#include <posix/stdint.h>
#include "test.h"

/**
 * Horizontal line for tests.
 */
const char *HLINE =
	"------------------------------------------------------------------------";

/**
 * @brief Forces a platform-independent delay.
 *
 * @param cycles Delay in cycles.
 *
 * @author João Vicente Souto
 */
static void delay(uint64_t cycles)
{
	uint64_t t0, t1;

	for (int i = 0; i < PROCESSOR_CLUSTERS_NUM; ++i)
	{
		kclock(&t0);

		do
			kclock(&t1);
		while ((t1 - t0) < cycles);
	}
}

/**
 * @brief Test Server
 */
int __main2(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	__runtime_setup(0);

		/* Unblock spawner. */
		uprintf("[nanvix][test] server alive");
		uassert(stdsync_fence() == 0);

		delay(CLUSTER_FREQ);

		__runtime_setup(1);
		test_name();

		__runtime_setup(3);
		test_rmem();
		test_rmem_cache();
		test_rmem_interface();

		uprintf("[nanvix][test] shutting down server");

	__runtime_cleanup();

	return (0);
}
