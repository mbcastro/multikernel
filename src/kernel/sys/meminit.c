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

/**
 * @brief Underlying IPC connectors.
 */
/**@{*/
int _mem_outbox = -1;    /* Mailbox used for small transfers. */
int _mem_inportal = -1;  /* Portal used for large transfers.  */
int _mem_outportal = -1; /* Portal used for large transfers.  */
/**@}*/

/**
 * @brief Initializes the RMA engine.
 */
void meminit(void)
{
	const char *clustername;    /* Cluster ID of the calling process. */
	static int initialized = 0; /* IS RMA Engine initialized?         */

	/* Already initialized.  */
	if (initialized)
		return;

	clustername = name_cluster_name(arch_get_cluster_id());

	/* Open underlying IPC connectors. */
	_mem_inportal = portal_create(clustername);
	_mem_outbox = mailbox_open("/io1");
	_mem_outportal = portal_open("/io1");
}

