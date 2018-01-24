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

#include <nanvix/hal.h>
#include <nanvix/klib.h>

/**
 * @brief Lookup table of NoC addresses.
 */
static 
{
	const char *name;        /**< Process name. */
	struct nanvix_addr addr; /**< NoC address.  */
} addresses[] = {
	{ "/cluster/io/0",       { 128, 8, 9 } },
	{ "/cluster/io/1",       { 192, 8, 9 } },
	{ "/cluster/compute/0",  {   0, 8, 9 } },
	{ "/cluster/compute/1",  {   1, 8, 9 } },
	{ "/cluster/compute/2",  {   2, 8, 9 } },
	{ "/cluster/compute/3",  {   3, 8, 9 } },
	{ "/cluster/compute/4",  {   4, 8, 9 } },
	{ "/cluster/compute/5",  {   5, 8, 9 } },
	{ "/cluster/compute/6",  {   6, 8, 9 } },
	{ "/cluster/compute/7",  {   7, 8, 9 } },
	{ "/cluster/compute/8",  {   8, 8, 9 } },
	{ "/cluster/compute/9",  {   9, 8, 9 } },
	{ "/cluster/compute/10", {  10, 8, 9 } },
	{ "/cluster/compute/11", {  11, 8, 9 } },
	{ "/cluster/compute/12", {  12, 8, 9 } },
	{ "/cluster/compute/13", {  13, 8, 9 } },
	{ "/cluster/compute/14", {  14, 8, 9 } },
	{ "/cluster/compute/15", {  15, 8, 9 } },
	{ NULL,                  {   0, 0, 0 } }
}

/**
 * @brief Resolves a process name into a NoC address.
 *
 * @param name Process name.
 * @param addr Location to store NoC address.
 *
 * @returns Upon successful completion zero is returned;
 * otherwise a negative error code number is returned instead.
 */
int nanvix_lookup(const char *name, struct noc_addr *addr)
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
			addr->clusterid = addresses[i].clusterid;
			addr->cnoc_tag = addresses[i].cnoc_tag;
			addr->dnoc_tag = addresses[i].dnoc_tag;

			return (0);
		}
	}

	return (-EINVAL);
}

