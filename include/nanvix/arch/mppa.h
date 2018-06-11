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
 * auint64_t with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NANVIX_ARCH_MPPA256
#define NANVIX_ARCH_MPPA256

	#ifndef _KALRAY_MPPA256
		#error "bad target"
	#endif

	/**
	 * @brief Number of mailboxes.
	 */
	#ifdef _KALRAY_MPPA_CCLUSTER_
		#define NR_MAILBOX 24
	#else
		#define NR_MAILBOX (4*24)
	#endif

	/**
	 * @brief Size (in bytes) of a mailbox message.
	 */
	#define MAILBOX_MSG_SIZE 64

#endif /* NANVIX_ARCH_MPPA256 */
