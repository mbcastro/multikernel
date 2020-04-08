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

#define __NEED_MM_MANAGER
#define __SHM_SERVICE
#define SPAWN_SERVER

#include <nanvix/runtime/rmem.h>
#include <nanvix/runtime/shm.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/servers/spawn.h>
#include <nanvix/sys/thread.h>
#include <nanvix/sys/excp.h>
#include <nanvix/sys/page.h>
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

/**
 * @brief Current runtime ring.
 */
static int current_ring[THREAD_MAX + 1] = {
	[0 ... (THREAD_MAX)] = -1
};

/**
 * @brief ID of exception handler thread.
 */
static kthread_t exception_handler_tid;

/**
 * @brief User-space exception handler.
 *
 * @param args Arguments for the thread (unused).
 *
 * @returns Always return NULL.
 */
static void *nanvix_exception_handler(void *args)
{
	vaddr_t vaddr;
	struct exception excp;

	UNUSED(args);

	uassert(__stdsync_setup() == 0);
	uassert(__stdmailbox_setup() == 0);
	uassert(__stdportal_setup() == 0);
	uassert(__name_setup() == 0);
	uassert(__nanvix_mailbox_setup() == 0);
	uassert(__nanvix_portal_setup() == 0);

	while (1)
	{
		if (excp_pause(&excp) != 0)
			break;

		vaddr = exception_get_addr(&excp);
		uassert(nanvix_rfault(vaddr) == 0);

		uassert(excp_resume() == 0);
	}

	return (NULL);
}

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
 * @todo TODO: provide a detailed description for this function.
 */
int __runtime_setup(int ring)
{
	int tid;

	tid = kthread_self();

	/* Invalid runtime ring. */
	if (ring < 0)
		return (-EINVAL);

	/* Nothing to do. */
	if (current_ring[tid] > ring)
		return (0);

	/* Initialize Ring 0. */
	if ((current_ring[tid] < SPAWN_RING_0) && (ring >= SPAWN_RING_0))
	{
		uprintf("[nanvix][thread %d] initalizing ring 0", tid);
		uassert(__stdsync_setup() == 0);
		uassert(__stdmailbox_setup() == 0);
		uassert(__stdportal_setup() == 0);
	}

	/* Initialize Ring 1. */
	if ((current_ring[tid] < SPAWN_RING_1) && (ring >= SPAWN_RING_1))
	{
		uprintf("[nanvix][thread %d] initalizing ring 1", tid);
		delay(CLUSTER_FREQ);
		uassert(__name_setup() == 0);
	}

	/* Initialize Ring 2. */
	if ((current_ring[tid] < SPAWN_RING_2) && (ring >= SPAWN_RING_2))
	{
		uprintf("[nanvix][thread %d] initalizing ring 2", tid);
		delay(CLUSTER_FREQ);
		uassert(__nanvix_mailbox_setup() == 0);
		uassert(__nanvix_portal_setup() == 0);
	}

	/* Initialize Ring 3. */
	if ((current_ring[tid] < SPAWN_RING_3) && (ring >= SPAWN_RING_3))
	{
		uprintf("[nanvix][thread %d] initalizing ring 3", tid);
		delay(CLUSTER_FREQ);
		uassert(__nanvix_shm_setup() == 0);
	}

	/* Initialize Ring 4. */
	if ((current_ring[tid] < SPAWN_RING_4) && (ring >= SPAWN_RING_4))
	{
		uprintf("[nanvix][thread %d] initalizing ring 4", tid);
		delay(CLUSTER_FREQ);
		uassert(__nanvix_rmem_setup() == 0);
		uassert(kthread_create(&exception_handler_tid, &nanvix_exception_handler, NULL) == 0);
	}

	current_ring[tid] = ring;

	return (0);
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __runtime_cleanup(void)
{
	int tid;

	tid = kthread_self();

	/* Initialize Ring 4. */
	if (current_ring[tid] >= SPAWN_RING_4)
	{
		uprintf("[nanvix][thread %d] shutting down ring 4", tid);
		uassert(__nanvix_rmem_cleanup() == 0);
		uassert(kthread_join(exception_handler_tid, NULL) == 0);
	}

	/* Initialize Ring 3. */
	if (current_ring[tid] >= SPAWN_RING_3)
	{
		uprintf("[nanvix][thread %d] shutting down ring 3", tid);
		uassert(__nanvix_shm_cleanup() == 0);
	}

	/* Initialize Ring 2. */
	if (current_ring[tid] >= SPAWN_RING_2)
	{
		uprintf("[nanvix][thread %d] shutting down ring 2", tid);
		uassert(__nanvix_portal_cleanup() == 0);
		uassert(__nanvix_mailbox_cleanup() == 0);
	}

	/* Initialize Ring 1. */
	if (current_ring[tid] >= SPAWN_RING_1)
	{
		uprintf("[nanvix][thread %d] shutting down ring 1", tid);
		uassert(__name_cleanup() == 0);
	}

	/* Spawn Ring 0.*/
	uassert(__stdportal_cleanup() == 0);
	uassert(__stdmailbox_cleanup() == 0);
	uassert(__stdsync_cleanup() == 0);

	current_ring[tid] = -1;

	return (0);
}
