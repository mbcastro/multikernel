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

#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <nanvix/mm.h>
#include <omp.h>
#include <stdlib.h>
#include "slave.h"

/* Kernel parameters. */
static int imgsize;       			                                                   /* IMG dimension.      */
static int masksize;       			                                                   /* Mask dimension.     */
static double mask[MASK_SIZE*MASK_SIZE];       			                               /* Mask.               */
static unsigned char chunk[(CHUNK_SIZE + MASK_SIZE - 1)*(CHUNK_SIZE + MASK_SIZE - 1)]; /* Image input chunk.  */
static unsigned char newchunk[CHUNK_SIZE*CHUNK_SIZE];	                               /* Image output chunk. */
static int nclusters;                                                                  /* Number of clusters. */

	
#define MASK(i, j) \
	mask[(i)*masksize + (j)]

#define CHUNK(i, j) \
	chunk[(i)*(CHUNK_SIZE + masksize - 1) + (j)]

#define NEWCHUNK(i, j) \
	newchunk[(i)*CHUNK_SIZE + (j)]

/**
 * @brief Convolutes a Gaussian filter on  an image.
 */
static void gauss_filter(void)
{
	#pragma omp for
	for (int chunkI = 0; chunkI < CHUNK_SIZE; chunkI++)
	{
		for (int chunkJ = 0; chunkJ < CHUNK_SIZE; chunkJ++)
		{
			double pixel = 0.0;
			
			for (int maskI = 0; maskI < masksize; maskI++)
			{
				for (int maskJ = 0; maskJ < masksize; maskJ++)
					pixel += CHUNK(chunkI + maskI, chunkJ + maskJ) * MASK(maskI, maskJ);
			}
		   
			NEWCHUNK(chunkI, chunkJ) = (pixel > 255) ? 255 : (int)pixel;
		}
	}
}

/**
 * @brief Stride writes data to remote memory.
 *
 * @param buffer Target buffer.
 * @param base   Base address on remote memory.
 * @param offset Stride offset.
 * @param stride Stride size.
 * @param count  Number of strides to write.
 */
static void memwrites(
	const unsigned char *buffer,
	uint64_t base,
	uint64_t offset,
	size_t stride,
	size_t count
) {
	int dsize = sizeof(unsigned char);
	
	for (size_t i = 0; i < count; i++)
	{
		memwrite(base + i*offset*dsize,
			&buffer[i*stride],
			stride*dsize
		);
	}
}

/**
 * @brief Stride reads data from remote memory.
 *
 * @param buffer Target buffer.
 * @param base   Base address on remote memory.
 * @param offset Stride offset.
 * @param stride Stride size.
 * @param count  Number of strides to read.
 */
static void memreads(
	unsigned char *buffer,
	uint64_t base,
	uint64_t offset,
	size_t stride,
	size_t count
) {
	int dsize = sizeof(unsigned char);
	
	for (size_t i = 0; i < count; i++)
	{
		memread(base + i*offset*dsize,
			&buffer[i*stride],
			stride*dsize
		);
	}
}

/**
 * @brief Convolutes a Gaussian filter on an image. 
 */
int main(int argc, char **argv)
{
	int rank;           /* Cluster rank.             */
	int nchunks;        /* Total number of chunks.   */
	int chunks_per_row; /* Number of chunks per row. */
	int chunks_per_col; /* Number of chunks per col. */

	((void)argc);
	
	rank = atoi(argv[0]);	
	
	/* Read input parameters. */
	memread(OFF_NCLUSTERS, &nclusters, sizeof(int));
	memread(OFF_MASKSIZE,  &masksize,  sizeof(int));
	memread(OFF_IMGSIZE,   &imgsize,   sizeof(int));
	memread(OFF_MASK,      mask,       masksize*masksize*sizeof(double));

	/* Find the number of chunks that will be generated. */
	chunks_per_row = (imgsize - masksize + 1)/CHUNK_SIZE;
	chunks_per_col = (imgsize - masksize + 1)/CHUNK_SIZE;
	nchunks = chunks_per_row*chunks_per_col;
	
	/* Process chunks in a round-robin. */
	for(int ck = rank; ck < nchunks; ck += nclusters)
	{
		uint64_t base;  /* Base address for remote read/write. */
		uint64_t off_y; /* Row offset for working chunk.       */
		uint64_t off_x; /* Column offset for working chunk.    */

		/* Compute offsets. */
		off_y = (ck/chunks_per_col)*CHUNK_SIZE*imgsize;
		off_x = (ck%chunks_per_row)*CHUNK_SIZE;

		base = OFF_IMAGE +
			off_y + /* Vertical skip.   */
			off_x;  /* Horizontal skip. */

		memreads(chunk,
			base,
			imgsize,
			(CHUNK_SIZE + masksize - 1),
			(CHUNK_SIZE + masksize - 1)
		);
	
		gauss_filter();
		
		base = OFF_NEWIMAGE +
			(masksize/2)*imgsize + off_y + /* Vertical skip.   */
			masksize/2 + off_x;            /* Horizontal skip. */

		memwrites(newchunk,
			base,
			imgsize,
			CHUNK_SIZE,
			CHUNK_SIZE
		);
	}
	
	return (0);
}
