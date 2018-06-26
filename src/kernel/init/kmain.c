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
pthread_mutex_t kernel_lock;

/**
 * @brief Input HAL mailbox.
 */
static int inboxes[HAL_NR_NOC_IONODES];

/**
 *
 * @brief Is the kernel initialized ?
 */
static int initialized[HAL_NR_NOC_IONODES] = { 0, };

/**
 * @brief Initializes kernel modules.
 */
int kernel_setup(void)
{
	int index;
	int nodeid;

	hal_setup();

	nodeid = hal_get_node_id();
	index = nodeid - hal_get_cluster_id();

	/*
	 * Kernel was already initialized.
	 * There is nothing else to do.
	 */
	if (initialized[index])
		return (0);

	/* Master thread. */
	if (index == 0)
		pthread_mutex_init(&kernel_lock, NULL);

	pthread_mutex_lock(&kernel_lock);

		/* Create underlying input mailbox. */
		if ((inboxes[index] = hal_mailbox_create(nodeid)) < 0)
			goto error0;

	pthread_mutex_unlock(&kernel_lock);

	initialized[index] = 1;

	return (0);

error0:
	pthread_mutex_unlock(&kernel_lock);
	return (-EAGAIN);
}

/**
 * @brief Cleans kernel.
 */
int kernel_cleanup(void)
{
	int index;

	index = (hal_get_node_id() - hal_get_cluster_id());

	/* Kernel was not initialized. */
	if (!initialized[index])
		goto end;

	pthread_mutex_lock(&kernel_lock);

		/* Destroy underlying input mailbox. */
		if (hal_mailbox_unlink(inboxes[index]) != 0)
			goto error;

	pthread_mutex_unlock(&kernel_lock);

	initialized[index] = 0;

end:
	/* Master thread. */
	if (index == 0)
		pthread_mutex_destroy(&kernel_lock);

	hal_cleanup();
	return (0);

error:
	pthread_mutex_unlock(&kernel_lock);
	return (-EAGAIN);
}

/**
 * @brief Get input mailbox.
 */
int get_inbox(void)
{
	int index;

	index = hal_get_node_id() - hal_get_cluster_id();

	/* Kernel was not initialized. */
	if (!initialized[index])
		return (-EINVAL);

	return (inboxes[index]);
}

/**
 * @brief Unset initialized flag.
 */
void unset_inbox()
{
	int index;

	index = hal_get_node_id() - hal_get_cluster_id();

	initialized[index] = 0;
}
