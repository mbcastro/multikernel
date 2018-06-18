/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
static uint64_t t[6] = { 0, 0, 0, 0, 0, 0 };
static uint64_t time_network[2] = { 0, 0 };
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
 * @param offset Chunk offset.
 */
static void memwrites(
	const unsigned char *buffer,
	uint64_t base,
	uint64_t offset
) {
		t[4] = k1_timer_get();
			memwrite(base + offset,
				buffer,
				sizeof(newchunk)
			);
		t[5] = k1_timer_get();
		time_network[1] += k1_timer_diff(t[4], t[5]);
		nwrite++;
		swrite += sizeof(newchunk);
}

/*===========================================================================*
 * memreads()                                                               *
 *===========================================================================*/

/**
 * @brief Stride reads data from remote memory.
 *
 * @param buffer Target buffer.
 * @param base   Base address on remote memory.
 * @param offset Chunk offset.
 */
static void memreads(
	unsigned char *buffer,
	uint64_t base,
	uint64_t offset
) {
		t[0] = k1_timer_get();
			memread(base + offset,
				buffer,
				sizeof(chunk)
			);
		t[1] = k1_timer_get();
		time_network[0] += k1_timer_diff(t[0], t[1]);
		nread++;
		sread += sizeof(chunk);
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
		time_network[0] += k1_timer_diff(t[0], t[1]);
		nread += 3; sread += 2*sizeof(int) + masksize*masksize*sizeof(double);

		/* Find the number of chunks that will be generated. */
		chunks_per_row = (imgsize - masksize + 1)/CHUNK_SIZE;
		chunks_per_col = (imgsize - masksize + 1)/CHUNK_SIZE;
		nchunks = chunks_per_row*chunks_per_col;
		
		/* Process chunks in a round-robin. */
		for(int ck = rank; ck < nchunks; ck += nclusters)
		{
            uint64_t base;   /* Base address for remote read/write. */
			uint64_t offset; /* Offset to working chunk.            */

			/* Compute offsets. */
			offset = ck * (CHUNK_SIZE+masksize-1) * (CHUNK_SIZE+masksize-1);

			base = OFF_CHUNKS;

			memreads(chunk,
				base,
                offset
			);

			gauss_filter();

            /* Compute offsets. */
            offset = ck*CHUNK_SIZE2;

            base = OFF_IMAGE;
			memwrites(newchunk,
				base,
				offset
			);
		}
	
	t[3] = k1_timer_get();
	time_cpu = k1_timer_diff(t[2], t[3]) - (time_network[0] - time_network[1]);

	printf("%d;%" PRIu64 ";%" PRIu64 ";%" PRIu64 ";%d;%d;%d;%d\n",
		rank,
		time_network[0],
		time_network[1],
		time_cpu,
		nread,
		sread,
		nwrite,
		swrite
	);
	
	return (0);
}
