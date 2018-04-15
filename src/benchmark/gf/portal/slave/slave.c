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

/* Timing statistics. */
long start;
long end;
long total = 0;

/* Gaussian Filter. */
static double *mask;       			/* Mask.               */
static int masksize;       			/* Dimension of mask.  */
static unsigned char *chunk;		/* Image input chunk.	 */
static unsigned char *newchunk;	/* Image output chunk. */
	
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
	double pixel;
	int chunkI, chunkJ, maskI, maskJ;

	#pragma omp parallel default(shared) private(chunkI,chunkJ,maskI,maskJ,pixel)
	{
		#pragma omp for
		for (chunkI = 0; chunkI < CHUNK_SIZE; chunkI++)
		{
			for (chunkJ = 0; chunkJ < CHUNK_SIZE; chunkJ++)
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
	
	int chunk_with_halo_size = CHUNK_SIZE + masksize - 1;
	chunk = (unsigned char *) smalloc(chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char));
	newchunk = (unsigned char *) smalloc(CHUNK_SIZE * CHUNK_SIZE * sizeof(unsigned char));
	
	assert(newchunk != NULL);
	assert(chunk != NULL);
	
	/* Receives filter mask. */
	data_receive(infd, mask, masksize * masksize * sizeof(double));

	/* Process chunks. */
  while (1)
	{
		data_receive(infd, &msg, sizeof(int));

		/* Parse message. */
		switch (msg)
		{
			case MSG_CHUNK:
				data_receive(infd, chunk, chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char));
				start = k1_timer_get();
				gauss_filter();
				end = k1_timer_get();
				total += k1_timer_diff(start, end);
				data_send(outfd, newchunk, CHUNK_SIZE * CHUNK_SIZE * sizeof(unsigned char));
				break;
			
			default:
				goto out;
		}
	}

out:
	
	data_send(outfd, &total, sizeof(long));
	
	close_noc_connectors();

	free(mask);
	free(chunk);
	free(newchunk);
	
	mppa_exit(0);
	return (0);
}
	
