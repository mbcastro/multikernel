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

#ifdef _KALRAY_MPPA256__
	#include <nanvix/arch/nodeos.h>
#endif

#ifdef UNIX
	#include <nanvix/arch/unix.h>
#endif


/*===========================================================================*
 * NoC                                                                       *
 *===========================================================================*/

	/**
	 * @brief Connector flags.
	 */
	/**@{*/
	#define CONNECTOR_FREE   (1 << 0)
	#define CONNECTOR_OUTPUT (1 << 1)
	#define CONNECTOR_DATA   (1 << 2)
	/**@}*/

	/* Forward definitions. */
	extern int nanvix_connector_open(struct noc_addr addr, int flags)
	extern int nanvix_connector_close(int id)
	extern int nanvix_connector_read(int id, void *ptr, size_t size)
	extern int nanvix_connector_write(int id, const void *buf, size_t size)

/*===========================================================================*
 * NoC                                                                       *
 *===========================================================================*/

	/* Forward definitions. */
	int nanvix_lookup(const char *, struct noc_addr *);
