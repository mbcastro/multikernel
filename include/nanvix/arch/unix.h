/*
 * Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#ifndef UNIX_H_
#define UNIX_H_

	#include <arpa/inet.h>

	/**
	 * @brief Process address.
	 */
	struct nanvix_process_addr
	{
		in_addr_t addr;
		in_port_t port;
	};

	#include <string.h>
	#include <stdio.h>
	#include <sys/socket.h>

	/**
	 * @brief Kernel puts().
	 *
	 * @param str Message.
	 *
	 * @return see puts().
	 */
	#define kputs(str) { fprintf(stderr, "%s\n", str); }

	/**
	 * @brief Non blocking IPC channel.
	 */
	#define _CHANNEL_NONBLOCK SOCK_NONBLOCK

#endif /* UNIX_H_ */
