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
#include <stdio.h>
#include <inttypes.h>
#include "slave.h"

/* Kernel parameters. */
static int imgsize;                         /* IMG dimension.      */
static int masksize;                        /* Mask dimension.     */
static double mask[MASK_SIZE2];             /* Mask.               */
static unsigned char chunk[TILE_SIZE2];     /* Image input chunk.  */
static unsigned char newchunk[CHUNK_SIZE2]; /* Image output chunk. */
static int nclusters;                       /* Number of clusters. */

/* Timing statistics. */
static uint64_t t[4];
static uint64_t time_network = 0;
static uint64_t time_cpu = 0;
static int nwrite = 0;
static int nread = 0;
static size_t swrite = 0;
static size_t sread = 0;

/*===========================================================================*
 * memwrites()                                                               *
 *===========================================================================*/

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
		t[0] = k1_timer_get();
			memwrite(base + i*offset*dsize,
				&buffer[i*stride],
				stride*dsize
			);
		t[1] = k1_timer_get();
		time_network += k1_timer_diff(t[0], t[1]);
		nwrite++;
		swrite += stride*dsize;
	}
}

/*===========================================================================*
 * memreads()                                                               *
 *===========================================================================*/

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
		t[0] = k1_timer_get();
			memread(base + i*offset*dsize,
				&buffer[i*stride],
				stride*dsize
			);
		t[1] = k1_timer_get();
		time_network += k1_timer_diff(t[0], t[1]);
		nread++;
		sread += stride*dsize;
	}
}

/*===========================================================================*
 * gauss_filter()                                                            *
 *===========================================================================*/

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

/*===========================================================================*
 * main()                                                                    *
 *===========================================================================*/

/**
 * @brief Convolutes a Gaussian filter on an image. 
 */
int main(int argc, char **argv)
{
	int rank;           /* Cluster rank.             */
	int halosize;       /* Halo size.                */
	int tilesize;       /* Tile size.                */
	int nchunks;        /* Total number of chunks.   */
	int chunks_per_row; /* Number of chunks per row. */
	int chunks_per_col; /* Number of chunks per col. */

	((void)argc);
	
	rank = atoi(argv[0]);	
	nclusters = atoi(argv[1]);

	k1_timer_init();

	t[2] = k1_timer_get();
		
		/* Read input parameters. */
		t[0] = k1_timer_get();
			memread(OFF_MASKSIZE,  &masksize, sizeof(int));
			memread(OFF_IMGSIZE,   &imgsize,  sizeof(int));
			memread(OFF_MASK,      mask,      masksize*masksize*sizeof(double));
		t[1] = k1_timer_get();
		time_network += k1_timer_diff(t[0], t[1]);
		nread += 3; sread += 2*sizeof(int) + masksize*masksize*sizeof(double);

		halosize = masksize/2;
		tilesize = (CHUNK_SIZE + masksize - 1);

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
				tilesize,
				tilesize
			);
		
			gauss_filter();
			
			base = OFF_NEWIMAGE +
				halosize*imgsize + off_y + /* Vertical skip.   */
				halosize + off_x;          /* Horizontal skip. */

			memwrites(newchunk,
				base,
				imgsize,
				CHUNK_SIZE,
				CHUNK_SIZE
			);
		}
	
	t[3] = k1_timer_get();
	time_cpu = k1_timer_diff(t[2], t[3]) - time_network;

	printf("%d;%" PRIu64 ";%" PRIu64 ";%d;%d;%d;%d\n",
		rank,
		time_network,
		time_cpu,
		nread,
		sread,
		nwrite,
		swrite
	);
	
	return (0);
}
