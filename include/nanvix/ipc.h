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

#ifndef NANVIX_IPC_H_
#define NANVIX_IPC_H_

	#include <sys/types.h>

	/* Forward definitions .*/
	extern int nanvix_mailbox_open(const char *);
	extern int nanvix_mailbox_send(int, const void *, size_t);
	extern int nanvix_mailbox_receive(void *, size_t n);
	extern int nanvix_mailbox_unlink(int);

#endif /* NANVIX_IPC_H_ */
