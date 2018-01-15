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

#include <errno.h>
#include <stdlib.h>

#include <nanvix/name.h>
#include <nanvix/klib.h>

/**
 * @brief Lookup table of addresses.
 */
static struct {
	const char *name; /**< Process name. */
	unsigned rank;    /**< Cluster rank. */
} addresses[] = {
	{ "/proc/0",         0 },
	{ "/proc/1",         1 },
	{ "/proc/2",         2 },
	{ "/proc/3",         3 },
	{ "/proc/4",         4 },
	{ "/proc/5",         7 },
	{ "/proc/6",         8 },
	{ "/proc/7",        11 },
	{ "/proc/8",        12 },
	{ "/proc/9",        13 },
	{ "/proc/10",       14 },
	{ "/proc/11",       15 },
	{ "/sys/rmem0",      5 },
	{ "/sys/rmem1",      6 },
	{ "/sys/rmem2",      9 },
	{ "/sys/rmem3",     10 },
	{ "/dev/mem0",     128 },
	{ "/dev/mem1",     192 },
	{ NULL, 0 }
};

/**
 * @brief Resolves a process name into an address.
 *
 * @param name Process name.
 * @param addr Address stor elocation.
 *
 * @returns Upon successful completion zero is returned;
 * otherwise a negative error code number is returned instead.
 */
int nanvix_lookup(const char *name, struct nanvix_process_addr *addr)
{
	/* Sanity check. */
	if ((name == NULL) || (addr == NULL))
		return (-EINVAL);

	/* Resolve process name. */
	for (int i = 0; addresses[i].name != NULL; i++)
	{
		/* Found. */
		if (!kstrcmp(addresses[i].name, name))
		{
			addr->tx = addresses[i].rank;
			return (0);
		}
	}

	return (-EINVAL);
}
