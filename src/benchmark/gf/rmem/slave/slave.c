/*
 * Copyright(C) 2014 Matheus M. Queiroz <matheus.miranda.queiroz@gmail.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

static unsigned char chunk[(CHUNK_SIZE + MASK_SIZE)*(CHUNK_SIZE + MASK_SIZE)]; /* Image input chunk.  */
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

	/* Chunk size is adjusted to generate at least 16 chunks. */
	nchunks = ((imgsize - masksize + 1)*(imgsize - masksize + 1))/(CHUNK_SIZE*CHUNK_SIZE);

	printf("Cluster %d: nclusters=%d, masksize=%d, imgsize=%d, nchunks=%d, CHUNK_SIZE=%d\n", rank, nclusters, masksize, imgsize, nchunks, CHUNK_SIZE);
	
	/* Process chunks in a round-robin fashion. */	
	for(int ck = rank; ck < nchunks; ck += nclusters)
	{
		for (int k = 0; k < CHUNK_SIZE + masksize; k++)
		{
			memread(OFF_IMAGE + ck*CHUNK_SIZE*CHUNK_SIZE + k*imgsize,
				&chunk[k*(CHUNK_SIZE + masksize)],
				(CHUNK_SIZE + masksize)*sizeof(unsigned char)
			);
		}
			
		gauss_filter();
		
		for (int k = 0; k < CHUNK_SIZE; k++)
		{
			memwrite(OFF_NEWIMAGE + ck*CHUNK_SIZE*CHUNK_SIZE + k*imgsize,
				 &newchunk[k*CHUNK_SIZE],
				 CHUNK_SIZE*sizeof(unsigned char));
		}
	}
	
	return (0);
}
