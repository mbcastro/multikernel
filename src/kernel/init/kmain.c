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
#include <nanvix/hal.h>
#include <nanvix/pm.h>

/**
 * @brief Global kernel lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Locks the kernel.
 */
void kernel_lock(void)
{
	pthread_mutex_lock(&lock);
}

/**
 * @brief Unlocks the kernel.
 */
void kernel_unlock(void)
{
	pthread_mutex_unlock(&lock);
}

/**
 *
 * @brief Is the kernel initialized ?
 */
static int initialized[HAL_NR_NOC_IONODES] = { 0, };

extern int name_init(void);
extern void name_finalize(void);

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

	kernel_lock();

		name_init();

		/* Create underlying input mailbox. */
		if (initialize_inbox(index) != 0)
			goto error0;

	kernel_unlock();

	initialized[index] = 1;
	return (0);

error0:
	kernel_unlock();
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
		return (-EAGAIN);

	kernel_lock();

		name_finalize();

		/* Destroy underlying input mailbox. */
		if (destroy_inbox(index) != 0)
			goto error;

	kernel_unlock();

	initialized[index] = 0;

	hal_cleanup();
	return (0);

error:
	kernel_unlock();
	return (-EAGAIN);
}
