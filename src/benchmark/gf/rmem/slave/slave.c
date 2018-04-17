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

static double *mask;       			/* Mask.               */
static int masksize;       			/* Mask dimension.     */

static unsigned char *chunk;		/* Image input chunk.  */
static unsigned char *newchunk;	/* Image output chunk. */

static int chunksize;           /* Chunk size.         */

static int nclusters;           /* Number of clusters. */
	
#define MASK(i, j) \
	mask[(i)*masksize + (j)]

#define CHUNK(i, j) \
	chunk[(i)*(chunksize + masksize - 1) + (j)]

#define NEWCHUNK(i, j) \
	newchunk[(i)*chunksize + (j)]

/*
 * Gaussian filter.
 */
void gauss_filter(void)
{
	double pixel;
	int chunkI, chunkJ, maskI, maskJ;

	#pragma omp parallel default(shared) private(chunkI,chunkJ,maskI,maskJ,pixel)
	{
		#pragma omp for
		for (chunkI = 0; chunkI < chunksize; chunkI++)
		{
			for (chunkJ = 0; chunkJ < chunksize; chunkJ++)
			{
				pixel = 0.0;
				
				for (maskI = 0; maskI < masksize; maskI++)
					for (maskJ = 0; maskJ < masksize; maskJ++)
						pixel += CHUNK(chunkI + maskI, chunkJ + maskJ) * MASK(maskI, maskJ);
			   
				NEWCHUNK(chunkI, chunkJ) = (pixel > 255) ? 255 : (int)pixel;
			}
		}
	}
}

int main(int argc, char **argv)
{
	((void)argc);
	
	rank = atoi(argv[0]);	
	
	/* Read input parameters. */
	memread(OFF_NCLUSTERS, &nclusters, sizeof(int));
	memread(OFF_MASKSIZE,  &masksize,  sizeof(int));
	memread(OFF_IMGSIZE,   &imgsize,   sizeof(int));
	
	/* Allocate filter mask. */
	mask = (double *) smalloc(masksize * masksize * sizeof(double));

	/* Read filter mask. */
	memread(OFF_MASK,  &mask[0], masksize * masksize * sizeof(double));

	/* Chunk size is adjusted to generate at least 16 chunks. */
	chunksize = ((imgsize - masksize + 1) * (imgsize - masksize + 1)) / (CHUNK_SIZE * CHUNK_SIZE) < 16 ? (imgsize - masksize + 1) / 4 : CHUNK_SIZE;
	int chunk_with_halo_size = chunksize + masksize - 1;
	int nb_chunks = ((imgsize - masksize + 1) * (imgsize - masksize + 1)) / (chunksize * chunksize);
	
	/* Allocate chunks. */
	chunk = (unsigned char *) smalloc(chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char));
	newchunk = (unsigned char *) smalloc(chunksize * chunksize * sizeof(unsigned char));

	printf("Cluster %d: nclusters=%d, masksize=%d, imgsize=%d, nb_chunks=%d, chunksize=%d, chunksize_halo=%d\n", rank, nclusters, masksize, imgsize, nb_chunks, chunksize, chunk_with_halo_size);
	
	/* Process chunks in a round-robin fashion. */	
	/*
	for(int ck = rank; ck < nb_chunks; ck += nclusters)
	{
		// should compute an offset from OFF_IMAGE here based on the chunk number (ck) and process rank (img_offset)
		for (int k = 0; k < chunk_with_halo_size; k++)
			memread(OFF_IMAGE + img_offset + k * imgsize,
              &chunk[k * chunk_with_halo_size],
              chunk_with_halo_size * sizeof(unsigned char));
			
		gauss_filter();
		
		// should compute an offset from OFF_NEWIMAGE here based on the chunk number (ck) and process rank (newimg_offset)
		for (int k = 0; k < chunksize; k++)
		memwrite(OFF_NEWIMAGE + newimg_offset + k * imgsize,
             &newchunk[k * chunksize],
						 chunksize * sizeof(unsigned char));
	}
	*/
	
	free(mask);
 	free(chunk);
 	free(newchunk);
	
	return (0);
}
	
