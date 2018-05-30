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

#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#define NR_SLAVE 3

/**
 * @brief ID of slave processes.
 */
static int pids[NR_SLAVE];

/*===================================================================*
 * Kernel                                                            *
 *===================================================================*/

/**
 * @brief Querying the name server.
 */
int main()
{
	const char *argv[] = {
		"name-slave",
		NULL
	};

	for (int i = 0; i < NR_SLAVE; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);

	for (int i = 0; i < NR_SLAVE; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);

	return (EXIT_SUCCESS);
}
