/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of Nanvix.
 * 
 * Nanvix is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * Nanvix is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nanvix/arch/mppa.h>

/**
 * @brief NoC tags offset
 */
#define NOCTAG_MAILBOX_OFF 2 /**< Mailbox. */
#define NOCTAG_PORTAL_OFF 22 /**< POrtal.  */
#define NOCTAG_SYNC_OFF   42 /**< Sync.    */

/*=======================================================================*
 * noctag_mailbox()                                                      *
 *=======================================================================*/

/**
 * @brief Returns the mailbox NoC tag for a target CPU ID.
 *
 * @param cpuid ID of the target CPU.
 */
int noctag_mailbox(int cpuid)
{
	if ((cpuid >= IOCLUSTER0) && (cpuid < (IOCLUSTER0 + NR_IOCLUSTER_DMA)))
	{
		return (NOCTAG_MAILBOX_OFF + cpuid%NR_IOCLUSTER_DMA);
	}
	else if ((cpuid >= IOCLUSTER1) && (cpuid < (IOCLUSTER1 + NR_IOCLUSTER_DMA)))
	{
		return (NOCTAG_MAILBOX_OFF + NR_IOCLUSTER_DMA + cpuid%NR_IOCLUSTER_DMA);
	}

	return (NOCTAG_MAILBOX_OFF + NR_IOCLUSTER_DMA + NR_IOCLUSTER_DMA + cpuid);
}
