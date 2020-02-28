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

#ifndef NANVIX_SERVERS_SPAWN_H_
#define NANVIX_SERVERS_SPAWN_H_

	/**
	 * @brief Number of RMem Servers.
	 */
	#define RMEM_SERVERS_NUM 1

	/**
	 * @brief NoC node number for Spawn Server.
	 */
	#define SPAWN_SERVER_NODE 0

	/**
	 * @brief NoC node number for Name Server.
	 */
	#define NAME_SERVER_NODE 0

	/**
	 * @brief NoC node number for RMem Server.
	 */
	#ifdef __mppa256__
		#define RMEM_SERVER_1_NODE 4
	#else
		#define RMEM_SERVER_1_NODE 1
	#endif

#ifdef SPAWN_SERVER

	/**
	 * @brief Declares the servers table.
	 *
	 * @param n Number of servers.
	 * @param x Servers table.
	 * @param y Spawner name.
	 */
	#define SPAWN_SERVERS(n, x, y)            \
		const int SERVERS_NUM = n;            \
		const struct serverinfo *SERVERS = x; \
		const char *spawner_name = y;

	/**
	 * @brief Server information.
	 */
	struct serverinfo
	{
		int (*main) (void); /**< Main function. */
	};

#endif /* SPAWN_SERVER*/

#endif /* NANVIX_SERVERS_SPAWN_H_ */
