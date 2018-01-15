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

#ifndef HAL_H_
#define HAL_H_

#ifdef UNIX

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

#endif

#endif /* */
