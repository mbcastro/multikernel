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

#include <nanvix/klib.h>
#include <nanvix/ipc.h>
#include <nanvix/ramdisk.h>

/**
 * RAM Disk device driver unit test.
 */
int main(int argc, char **argv)
{
	int channel;
	char buf[128];

	((void) argc);
	((void) argv);

	while (1)
	{
		channel = nanvix_ipc_connect(RAMDISK_NAME);

		strncpy(buf, "hello world", sizeof(buf)); 
		nanvix_ipc_send(channel, buf, sizeof(buf));

		nanvix_ipc_close(channel);
	}

	return (NANVIX_SUCCESS);
}


