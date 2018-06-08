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

#ifndef NANVIX_PM_H_
#define NANVIX_PM_H_

	#include <stddef.h>

/*=======================================================================*
 * Naming                                                                *
 *=======================================================================*/

	/* Forward definitions. */
	extern int name_cluster_id(char *);
	extern int name_cluster_dma(char *);
	extern int name_lookup_pathname(int, char *);
	extern void name_remotes(char *, int);
	extern void name_link(int, const char *);
	extern void name_unlink(char *);

/*=======================================================================*
 * Mailbox                                                               *
 *=======================================================================*/

	/* Mailbox types */
	#define NAME 1
	#define STD 0

	/* Forward definitions .*/
	extern int mailbox_create(char *);
	extern int mailbox_open(char *);
	extern int _mailbox_create(int);
	extern int _mailbox_open(int);
	extern int mailbox_read(int, void *);
	extern int mailbox_write(int, const void *);
	extern int mailbox_close(int);
	extern int mailbox_unlink(int);

/*=======================================================================*
 * Portal                                                                *
 *=======================================================================*/

	/* Forward definitions .*/
	extern int portal_allow(int, int);
	extern int portal_create(char *);
	extern int portal_open(char *);
	extern int portal_read(int, void *, size_t);
	extern int portal_write(int, const void *, size_t);
	extern int portal_close(int);
	extern int portal_unlink(int);

/*======================================================================*
 * Barrier                                                              *
 *=======================================================================*/

	/* Forward definitions. */
	extern int barrier_open(int);
	extern int barrier_wait(int);
	extern int barrier_close(int);

#endif /* NANVIX_PM_H_ */
