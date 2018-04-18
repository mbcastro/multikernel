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

#ifndef _SLAVE_H_
#define _SLAVE_H_
		
	#include "../../kernel.h"

	#define CHUNK_SIZE2 (CHUNK_SIZE*CHUNK_SIZE)    /**< (Chunk Size)^2        */
	#define MASK_SIZE2 (MASK_SIZE*MASK_SIZE)       /**< (Mask Size)^2         */
	#define TILE_SIZE (CHUNK_SIZE + MASK_SIZE - 1) /**< Size of chunk + mask. */
	#define TILE_SIZE2 (TILE_SIZE*TILE_SIZE)       /**< (Tile Size)^2         */

#endif /* _SLAVE_H_ */
