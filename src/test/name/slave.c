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

#include <nanvix/arch/mppa.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/*====================================================================*
 * main                                                               *
 *====================================================================*/

/**
 * @brief Remote name unit test.
 */
int main()
{
  char pathname[15];
  char process_name[50];             /* Process name */

  /* Primitives test */

  sprintf(pathname, "/cpu%d", k1_get_cluster_id());
  sprintf(process_name, "process_on_cpu%d", k1_get_cluster_id());

  /* Ask for an unregistered entry */
  assert(name_cluster_id(pathname) == -2);
  assert(name_cluster_dma(pathname) == -2);
  assert(!strcmp(id_cluster_name(k1_get_cluster_id()), " "));
  assert(!strcmp(id_process_name(k1_get_cluster_id()), " "));

  /* Register this cluster */
  register_name(k1_get_cluster_id(), pathname, process_name);

  /* Ask for a registered entry */
  assert(name_cluster_id(pathname) == k1_get_cluster_id());
  assert(name_cluster_dma(pathname) == k1_get_cluster_id());
  assert(!strcmp(id_cluster_name(k1_get_cluster_id()), pathname));
  assert(!strcmp(id_process_name(k1_get_cluster_id()), process_name));

  /* Remove the entry */
  remove_name(pathname);

  /* Verify that the entry is removed */
  assert(name_cluster_id(pathname) == -2);
  assert(name_cluster_dma(pathname) == -2);
  assert(!strcmp(id_cluster_name(k1_get_cluster_id()), " "));
  assert(!strcmp(id_process_name(k1_get_cluster_id()), " "));

	return (EXIT_SUCCESS);
}
