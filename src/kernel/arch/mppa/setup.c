/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of Nanvix.
 * 
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <pthread.h>

#include <nanvix/hal.h>

/**
 * @brief Number of running threads.
 */
static int nthreads = 0;

/**
 * @brief Lock for critical region.
 */
static pthread_spinlock_t lock;

/**
 * @brief Threads table.
 */
pthread_t __threads[4];

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
		pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE);

	tid = pthread_self();

	pthread_spin_lock(&lock);
		__threads[nthreads] = tid;
		nthreads++;
	pthread_spin_unlock(&lock);
}

