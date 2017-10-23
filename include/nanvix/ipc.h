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

#ifndef IPC_H_
#define IPC_H_

	#include <sys/types.h>

	/**
	 * @brief Maximum number of IPC channels.
	 */
	#define NANVIX_IPC_MAX 128

	/* Forward definitions. */
	extern int nanvix_ipc_create(const char *);
	extern int nanvix_ipc_close(int);
	extern int nanvix_ipc_unlink(int);
	extern int nanvix_ipc_open(int);
	extern int nanvix_ipc_connect(const char *);
	extern int nanvix_ipc_send(int, const char *, size_t);
	extern int nanvix_ipc_receive(int, char *, size_t);

#endif /* IPC_H_ */
