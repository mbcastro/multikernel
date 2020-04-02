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

#include <nanvix/runtime/rmem.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>
#include <posix/stddef.h>

/* Import definitions. */
extern void *nanvix_malloc(size_t size);
extern void nanvix_free(void *ptr);
extern int nanvix_dummy(int example);

/**
 * @brief Syntetic Benchmark
 */
int __main2(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);



	__runtime_setup(0);

		/* Unblock spawner. */
		uassert(stdsync_fence() == 0);
		uprintf("[nanvix][benchmark] server alive");

		__runtime_setup(3);

		uassert(nanvix_dummy(2) == 0);

		nanvix_shutdown();

	__runtime_cleanup();

	return (0);
}
