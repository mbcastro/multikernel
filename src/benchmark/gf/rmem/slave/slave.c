/*
 * Copyright(C) 2011-2018 Matheus Queiroz <matheus.miranda.queiroz@gmail.com>
 *                        Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
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
#include <assert.h>
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include "slave.h"

/* Cluster rank. */
int rank;

/* Gaussian Filter. */
static int imgsize;       			/* IMG dimension.      */

static double mask[MASK_SIZE*MASK_SIZE];       			/* Mask.               */
static int masksize;       			/* Mask dimension.     */

static unsigned char chunk[(CHUNK_SIZE + MASK_SIZE - 1)*(CHUNK_SIZE + MASK_SIZE - 1)]; /* Image input chunk.  */
static unsigned char newchunk[CHUNK_SIZE*CHUNK_SIZE];	                       /* Image output chunk. */

static int nclusters;           /* Number of clusters. */
	
#define MASK(i, j) \
	mask[(i)*masksize + (j)]

#define CHUNK(i, j) \
	chunk[(i)*(CHUNK_SIZE + masksize - 1) + (j)]

#define NEWCHUNK(i, j) \
	newchunk[(i)*CHUNK_SIZE + (j)]

/*
 * Gaussian filter.
 */
void gauss_filter(void)
{
	#pragma omp for
	for (int chunkI = 0; chunkI < CHUNK_SIZE; chunkI++)
	{
		for (int chunkJ = 0; chunkJ < CHUNK_SIZE; chunkJ++)
		{
			double pixel = 0.0;
			
			for (int maskI = 0; maskI < masksize; maskI++)
				for (int maskJ = 0; maskJ < masksize; maskJ++)
					pixel += CHUNK(chunkI + maskI, chunkJ + maskJ) * MASK(maskI, maskJ);
		   
			NEWCHUNK(chunkI, chunkJ) = (pixel > 255) ? 255 : (int)pixel;
		}
	}
}

void memwrites(unsigned char *buffer, uint64_t base, uint64_t offset, size_t stride, size_t count)
{
	int dsize = sizeof(unsigned char);
	
	for (size_t i = 0; i < count; i++)
	{
		memwrite(base + i*(offset)*dsize,
			&buffer[i*stride],
			stride*dsize
		);
	}
}

void memreads(unsigned char *buffer, uint64_t base, uint64_t offset, size_t stride, size_t count)
{
	int dsize = sizeof(unsigned char);
	
	for (size_t i = 0; i < count; i++)
	{
		memread(base + i*(offset)*dsize,
			&buffer[i*stride],
			stride*dsize
		);
	}
}
#include <string.h>
int main(int argc, char **argv)
{
	int nchunks;

	((void)argc);
	
	rank = atoi(argv[0]);	
	
	/* Read input parameters. */
	memread(OFF_NCLUSTERS, &nclusters, sizeof(int));
	memread(OFF_MASKSIZE,  &masksize,  sizeof(int));
	memread(OFF_IMGSIZE,   &imgsize,   sizeof(int));
	memread(OFF_MASK,      mask,       masksize*masksize*sizeof(double));

	/* Find the number of chunks that will be generated. */
	nchunks = ((imgsize - masksize + 1)*(imgsize - masksize + 1))/(CHUNK_SIZE*CHUNK_SIZE);

	if (rank == 0)
	 printf("Cluster %d: nclusters=%d, masksize=%d, imgsize=%d, nchunks=%d, CHUNK_SIZE=%d\n", rank, nclusters, masksize, imgsize, nchunks, CHUNK_SIZE);
	
	
	/* Process chunks in a round-robin fashion. */	
	for(int ck = rank; ck < nchunks; ck += nclusters)
	{
		int chunks_per_row = (imgsize - masksize + 1)/CHUNK_SIZE;
		int chunks_per_col = (imgsize - masksize + 1)/CHUNK_SIZE;

		memreads(chunk,
			OFF_IMAGE + (ck/chunks_per_col)*(CHUNK_SIZE)*imgsize + (ck%chunks_per_row)*CHUNK_SIZE,
			imgsize,
			(CHUNK_SIZE + masksize - 1),
			(CHUNK_SIZE + masksize - 1)
		);

	
		if(rank == 0) {
		 	for(int i = 0; i < CHUNK_SIZE + masksize - 1; i++) {
		 		for(int j = 0; j < CHUNK_SIZE + masksize - 1; j++)
		 			printf("%2d ", chunk[(CHUNK_SIZE + masksize - 1) * i + j]);
		 		printf("\n");
		 	}
		 		printf("------\n");
		}

		gauss_filter();
		
		
		if(rank == 0) {
		 	for(int i = 0; i < CHUNK_SIZE; i++) {
		 		for(int j = 0; j < CHUNK_SIZE; j++)
		 			printf("%2d ", newchunk[(CHUNK_SIZE) * i + j]);
		 		printf("\n");
		 	}
		 		printf("------\n");
		}

			memwrites(newchunk,
			OFF_NEWIMAGE + ((masksize/2)*imgsize) + (ck/chunks_per_col)*(CHUNK_SIZE)*imgsize + masksize/2 + (ck%chunks_per_row)*(CHUNK_SIZE),
			imgsize,
			CHUNK_SIZE,
			CHUNK_SIZE
		);
		
	}
	
	return (0);
}
