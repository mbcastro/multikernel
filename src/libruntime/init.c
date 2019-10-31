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
#include <nanvix/ulib.h>
#include <posix/errno.h>

/**
 * @brief Current runtime ring.
 */
static int current_ring = -1;

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __runtime_setup(int ring)
{
	/* Invalid runtime ring. */
	if (ring < 0)
		return (-EINVAL);

	/* Nothing to do. */
	if (current_ring > ring)
		return (0);

	/* Initialize unnamed IKC services. */
	if ((current_ring < 0) && (ring >= 0))
	{
		uprintf("[nanvix] initalizing ring 0");
		uassert(__stdsync_setup() == 0);
		uassert(__stdmailbox_setup() == 0);
		uassert(__stdportal_setup() == 0);
	}

	/* Initialize Name Service client. */
	if ((current_ring < 1) && (ring >= 1))
	{
		uprintf("[nanvix] initalizing ring 1");
		__name_setup();
	}

	/* Initialize Named IKC facilities. */
	if ((current_ring < 2) && (ring >= 2))
	{
		uprintf("[nanvix] initalizing ring 2");
		__nanvix_mailbox_setup();
		__nanvix_portal_setup();
	}

	/* Initialize RMem Service client. */
	if ((current_ring < 3) && (ring >= 3))
	{
		uprintf("[nanvix] initalizing ring 3");
		__nanvix_rmem_setup();
	}

	current_ring = ring;

	return (0);
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __runtime_cleanup(void)
{
	/* Cleanup RMem Service client. */
	if (current_ring >= 3)
		__nanvix_rmem_cleanup();

	/* Clean up IKC facilities. */
	if (current_ring >= 2)
	{
		__nanvix_portal_cleanup();
		__nanvix_mailbox_cleanup();
	}

	/* Clean up Name Service client. */
	if (current_ring >= 1)
		__name_cleanup();

	uassert(__stdportal_cleanup() == 0);
	uassert(__stdmailbox_cleanup() == 0);
	uassert(__stdsync_cleanup() == 0);

	current_ring = 0;

	return (0);
}
