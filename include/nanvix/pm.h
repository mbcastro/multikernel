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

	#include <inttypes.h>

/*=======================================================================*
 * Naming                                                                *
 *=======================================================================*/

	/* Forward definitions. */
	extern int name_lookup(const char *);
	extern const char *name_lookdown(int);
	extern void name_remotes(char *, int);

/*=======================================================================*
 * Mailbox                                                               *
 *=======================================================================*/

	/* Forward definitions .*/
	extern int mailbox_create(const char *);
	extern int mailbox_open(const char *);
	extern int mailbox_read(int, void *);
	extern int mailbox_write(int, const void *);
	extern int mailbox_close(int);
	extern int mailbox_unlink(int);

/*=======================================================================*
 * Portal                                                                *
 *=======================================================================*/

	/* Forward definitions .*/
	extern int portal_allow(int, int);
	extern int portal_create(const char *);
	extern int portal_open(const char *);
	extern int portal_read(int, void *, size_t);
	extern int portal_write(int, const void *, size_t);
	extern int portal_close(int);
	extern int portal_unlink(int);

/*======================================================================*
 * Timer                                                                *
 *=======================================================================*/

	/* Forward defnitions. */
	extern long timer_get(void);
	extern long timer_diff(long, long);
	extern void timer_init(void);

#endif /* NANVIX_IPC_H_ */
