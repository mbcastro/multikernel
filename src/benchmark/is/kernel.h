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

#ifndef _KERNEL_H_
#define _KERNEL_H_

	#define MICRO (1.0/1000000)

	/* Size of buckets. */
	#define NUM_BUCKETS 256
	/*
	 * Size of mini-bucket.
	 */
	#define MINIBUCKET_SIZE 262144

	/* RMEM definitions. */
	#define SIZE_MSG     (sizeof(int))

	#define OFF_MSG   (0)
	#define OFF_TODO(x)    (SIZE_MSG + x)
	#define OFF_DONE(x)    (NUM_BUCKETS*MINIBUCKET_SIZE + x)

#endif /* _KERNEL_H_ */

