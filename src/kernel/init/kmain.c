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

#include <errno.h>
#include <stdio.h>
#include <pthread.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_MAILBOX_
#include <nanvix/hal.h>

/**
 * @brief Global kernel lock.
 */
static pthread_mutex_t kernel_lock;

/**
 * @brief Input HAL mailbox.
 */
static int inbox[HAL_NR_NOC_IONODES];

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
		for (int i = 0; i < HAL_NR_NOC_IONODES; i++)
			inbox[i] = -1;

		initialized = 1;
	}
}

/**
 * @brief Initializes kernel modules.
 */
int kernel_setup(void)
{
	int index;

	hal_setup();

	pthread_mutex_init(&kernel_lock, NULL);

	pthread_mutex_lock(&kernel_lock);

	inbox_init();

	index = (hal_get_node_id() - hal_get_cluster_id());

	/* Bad index. */
	if ((index < 0) || (index >= HAL_NR_NOC_IONODES))
	{
		pthread_mutex_unlock(&kernel_lock);
		return (-EAGAIN);
	}

	/* Nothing to do. */
	if (inbox[index] != -1)
	{
		pthread_mutex_unlock(&kernel_lock);
		return 0;
	}

	/* Create inbox. */
	inbox[index] = hal_mailbox_create(hal_get_node_id());

	pthread_mutex_unlock(&kernel_lock);

	if(inbox[index] < 0)
		return (-EAGAIN);

	return (0);
}

/**
 * @brief Cleans kernel.
 */
int kernel_cleanup(void)
{
	int index;

	pthread_mutex_lock(&kernel_lock);

	index = (hal_get_node_id() - hal_get_cluster_id());

	if (inbox[index] != -1)
	{
		if (hal_mailbox_unlink(inbox[index]) != 0)
		{
			pthread_mutex_unlock(&kernel_lock);
			return (-EAGAIN);
		}

		inbox[index] = -1;
	}

	pthread_mutex_unlock(&kernel_lock);

	hal_cleanup();

	return (0);
}

/**
 * @brief Get input mailbox.
 */
int get_inbox(void)
{
	int index;

	pthread_mutex_lock(&kernel_lock);

	index = (hal_get_node_id() - hal_get_cluster_id());

	pthread_mutex_unlock(&kernel_lock);

	return (inbox[index]);
}

