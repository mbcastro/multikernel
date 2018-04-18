/*
 * Copyright(C) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *                        Márcio Castro <mbcastro@gmail.com>
 * 
 * This file is part of CAP Bench.
 * 
 * CAP Bench is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 * 
 * CAP Bench is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * CAP Bench. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MASTER_H_
#define _MASTER_H_

  #include <nanvix/arch/mppa.h>
  #include <stddef.h>
  #include "../../kernel.h"

	/*===============================================================*
	 * IPC                                                           *
	 *===============================================================*/
	
	/* Forward definitions. */
	extern void spawn_slaves(void);
	extern void join_slaves(void);

	/* Forward definitions. */
	extern int nclusters;
	
	/*===============================================================*
	 * Utility                                                       *
	 *===============================================================*/

	/* Forward definitions. */
	extern void *smalloc(size_t);
	extern void srandnum(int);
	extern unsigned randnum(void);

	/*===============================================================*
	 * Kernel                                                        *
	 *===============================================================*/

	/* Forward definitions. */
	extern void gauss_filter(unsigned char *, int, const double *, int);

#endif /* _MASTER_H_ */
