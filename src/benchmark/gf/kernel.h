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

	#define CHUNK_SIZE (16)     /* Maximum chunk size. */
	#define MASK_SIZE   15

	#define PI 3.14159265359    /* pi */
	#define E 2.71828182845904  /* e */
	#define SD 0.8              /* Standard deviation. */

	/* Type of messages. */
	#define MSG_CHUNK 1
	#define MSG_DIE   0

	/* RMEM definitions. */
	#define SIZE_MASKSIZE    (sizeof(int))
	#define SIZE_IMGSIZE     (sizeof(int))

	#define SIZE_MASK        (masksize * masksize * sizeof(double))
	#define SIZE_IMAGE       (imgsize * imgsize * sizeof(unsigned char))
	#define SIZE_NEWIMAGE    SIZE_IMAGE

	#define OFF_MASKSIZE   (0)
	#define OFF_IMGSIZE    (OFF_MASKSIZE  + SIZE_MASKSIZE)
	#define OFF_MASK       (OFF_IMGSIZE   + SIZE_IMGSIZE) 
	#define OFF_IMAGE      (OFF_MASK      + SIZE_MASK)
	#define OFF_NEWIMAGE   (OFF_IMAGE     + SIZE_IMAGE)

#endif /* _KERNEL_H_ */
