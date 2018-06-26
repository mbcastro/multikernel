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

#define __NEED_HAL_NOC_
#include <nanvix/hal.h>

#include "core.h"

/**
 * @brief Number of running threads.
 */
static int nthreads = 0;

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
