/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
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

#define __NEED_NAME_CLIENT
#define __NEED_MM_STUB

#include <nanvix/servers/spawn.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/rmem.h>
#include <nanvix/servers/name.h>
#include <nanvix/ulib.h>

/**
 * The nanvix_shutdown() function shuts down sends a shutdown signal
 * to all system services, asking them to terminate.
 */
int nanvix_shutdown(void)
{
	__runtime_setup(3);

	/* Broadcast shutdown signal. */
	for (int i = 0; i < RMEM_SERVERS_NUM; i++)
		uassert(nanvix_rmem_shutdown(i) == 0);

	uassert(name_shutdown() == 0);

	return (0);
}
