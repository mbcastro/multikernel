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

#include <pthread.h>
#include <errno.h>
#include <stdio.h>

#define __NEED_HAL_NOC_
#define __NEED_HAL_MAILBOX_
#include <nanvix/hal.h>

#include "core.h"

/**
 * @brief Number of running threads.
 */
static int nthreads = 0;

/**
 * @brief Input HAL mailbox.
 */
static int inbox[NR_IOCLUSTER_CORES];

/**
 *
 * @brief Is mailboxes initialized ?
 */
static int initialized = 0;

/**
 * @brief Initializes inboxes.
 */
static void inbox_init(void)
{
	if (!initialized)
	{
		for (int i = 0; i < NR_IOCLUSTER_CORES; i++)
			inbox[i] = -1;

		initialized = 1;
	}
}

/**
 * @brief Initializes platform-dependent structures.
 */
void hal_setup(void)
{
	pthread_t tid;

	/*
	 * Master thread initializes the lock. This is safe
	 * because it means that we are sequential.
	 */
	if (nthreads == 0)
		pthread_mutex_init(&core_lock, NULL);

	tid = pthread_self();

	pthread_mutex_lock(&core_lock);
		if (k1_is_iocluster(__k1_get_cluster_id()))
		{
			for (int i = 0; i < NR_IOCLUSTER_CORES; i++)
			{
				if (__threads[i] == 0)
				{
					__threads[i] = tid;
					nthreads++;
					break;
				}
			}
		}
	pthread_mutex_unlock(&core_lock);
}

/**
 * @brief Cleans platform-dependent structures.
 */
void hal_cleanup(void)
{
	pthread_t tid;

	/*
	 * Master thread initializes the lock. This is safe
	 * because it means that we are sequential.
	 */
	if (nthreads == 1)
		pthread_mutex_destroy(&core_lock);

	tid = pthread_self();

	pthread_mutex_lock(&core_lock);
		if (k1_is_iocluster(__k1_get_cluster_id()))
		{
			for (int i = 0; i < NR_IOCLUSTER_CORES; i++)
			{
				if (__threads[i] == tid)
				{
					__threads[i] = 0;
					nthreads--;
					break;
				}
			}
		}
	pthread_mutex_unlock(&core_lock);
}

/**
 * @brief Initializes kernel.
 */
int kernel_setup(void)
{
	int index;

	pthread_mutex_init(&core_lock, NULL);

	hal_setup();

	pthread_mutex_lock(&core_lock);

	inbox_init();

	index = (hal_get_node_id() - __k1_get_cluster_id());

	/* Bad index. */
	if ((index < 0) || (index >= NR_IOCLUSTER_CORES))
	{
		pthread_mutex_unlock(&core_lock);
		return (-EAGAIN);
	}

	/* Nothing to do. */
	if (inbox[index] != -1)
	{
		pthread_mutex_unlock(&core_lock);
		return 0;
	}

	/* Create inbox. */
	inbox[index] = hal_mailbox_create(hal_get_node_id());

	pthread_mutex_unlock(&core_lock);

	if(inbox[index] < 0)
		return (-EAGAIN);

	return 0;
}

/**
 * @brief Cleans kernel.
 */
int kernel_cleanup(void)
{
	int index;

	pthread_mutex_init(&core_lock, NULL);

	pthread_mutex_lock(&core_lock);

	index = (hal_get_node_id() - __k1_get_cluster_id());

	if (inbox[index] != -1)
	{
		if (hal_mailbox_unlink(inbox[index]) != 0)
		{
			pthread_mutex_unlock(&core_lock);
			return (-EAGAIN);
		}

		inbox[index] = -1;
	}

	pthread_mutex_unlock(&core_lock);

	hal_cleanup();

	return 0;
}

/**
 * @brief Get input mailbox.
 */
int get_inbox(void)
{
	int index;

	pthread_mutex_init(&core_lock, NULL);

	pthread_mutex_lock(&core_lock);

	index = (hal_get_node_id() - __k1_get_cluster_id());

	pthread_mutex_unlock(&core_lock);

	return (inbox[index]);
}
