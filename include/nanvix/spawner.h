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

#ifndef NANVIX_SPAWNER_H_
#define NANVIX_SPAWNER_H_

	#include <pthread.h>

	/**
	 * @brief Server information.
	 */
	struct serverinfo
	{
		int (*main) (int, int); /**< Main function.   */
		int nodenum;            /**< Node number.     */
		int runlevel;           /**< Server runlevel. */
	};

	/**
	 * @brief Spawner message.
	 */
	struct spawner_message
	{
		char unused[60]; /**< Not used. */
		int status;      /**< Status.   */
	};

	/* Forward definitions. */
	extern const char *spawner_name;
	extern const int spawner_shutdown;
	extern const int spawner_nservers;
	extern struct serverinfo *spawner_servers;
	extern void (*test_kernel_fn)(const char *);
	extern int (*main2_fn)(int, const char **);
	extern void spawner_init(void);
	extern void spawner_finalize(void);
	extern void spawners_sync(int);
	extern void spawner_ack(void);

		/**
	 * @brief Number of runlevels.
	 */
	#define NR_RUNLEVELS 4

	/**
	 * @brief Shutdown flags.
	 */
	/**@{*/
	#define SHUTDOWN_DISABLE 0 /**< Disable shutdown. */
	#define SHUTDOWN_ENABLE  1 /**< Enable shutdown.  */
	/**@}*/

	/**
	 * @brief Declares the name of the spawner.
	 *
	 * @param x Spawner name.
	 */
	#define SPAWNER_NAME(x) \
		const char *spawner_name = x;

	/**
	 * @brief Declares shutdown flag in the spawner.
	 *
	 * @param x Enable shutdown?
	 */
	#define SPAWNER_SHUTDOWN(x) \
		const int spawner_shutdown = x;

	/**
	 * @brief Declares the user-level main function.
	 *
	 * @param x User-level main function.
	 */
	#define SPAWNER_MAIN2(x) int (*main2_fn)(int, const char **) = x;

	/**
	 * @brief Declares the servers table.
	 *
	 * @param n Number of servers.
	 * @param x Servers table.
	 */
	#define SPAWNER_SERVERS(n, x)       \
		const int spawner_nservers = n;   \
		struct serverinfo *spawner_servers = x;

	/**
	 * @brief Declares test-driver for the kernel.
	 *
	 * @param x Kernel test-driver.
	 */
	#define SPAWNER_KERNEL_TESTS(x) \
		void (*test_kernel_fn)(const char *) = x;

	/* Forward definitions. */
	extern pthread_barrier_t spawner_barrier;

#endif /* NANVIX_SPAWNER_H_*/
