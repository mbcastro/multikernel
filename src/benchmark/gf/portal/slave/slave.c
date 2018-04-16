/*
 * Copyright(C) 2014 Matheus M. Queiroz <matheus.miranda.queiroz@gmail.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include "slave.h"

/* Gaussian Filter. */
static double *mask;       			/* Mask.               */
static int masksize;       			/* Dimension of mask.  */
static unsigned char *chunk;		/* Image input chunk.  */
static unsigned char *newchunk;	    /* Image output chunk. */
static int chunksize;               /* Chunk size.         */
	
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
	int msg;
	
	k1_timer_init();

	((void)argc);
	
	rank = atoi(argv[0]);	
	
	/* Setup interprocess communication. */
	open_noc_connectors();

	/* Receives filter mask size. */
	data_receive(infd, &masksize, sizeof(int));

	/* Allocates filter mask and chunks. */
	mask = (double *) smalloc(masksize * masksize * sizeof(double));

	/* Receives filter mask. */
	data_receive(infd, mask, masksize * masksize * sizeof(double));

	/* Receives chunk size. */
	data_receive(infd, &chunksize, sizeof(int));
		
	int chunk_with_halo_size = chunksize + masksize - 1;

	chunk = (unsigned char *) smalloc(chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char));
	newchunk = (unsigned char *) smalloc(chunksize * chunksize * sizeof(unsigned char));
	
	/* Process chunks. */
	while (1)
	{
		data_receive(infd, &msg, sizeof(int));

		/* Parse message. */
		switch (msg)
		{
			case MSG_CHUNK:
				data_receive(infd, chunk, chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char));
				gauss_filter();
				data_send(outfd, newchunk, chunksize * chunksize * sizeof(unsigned char));
				break;
			
			default:
				goto out;
		}
	}

out:
	
	
	close_noc_connectors();

	free(mask);
	free(chunk);
	free(newchunk);
	
	return (0);
}
	
