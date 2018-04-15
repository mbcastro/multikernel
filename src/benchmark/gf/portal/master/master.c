/*
* Copyright(C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
* 
* master.c - Master process.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "master.h"

/* Gaussian Filter. */
static unsigned char *img; /* Image.              */
static int imgsize;        /* Dimension of image. */
static double *mask;       /* Mask.               */
static int masksize;       /* Dimension of mask.  */

/*
 * Gaussian filter.
 */
void gauss_filter(unsigned char *img_, int imgsize_, double *mask_, int masksize_)
{	
	int nchunks = 0;
	int msg;     /* Message. */
	unsigned char *newimg;

	/* Setup parameters. */
	img = img_;
	mask = mask_;
	imgsize = imgsize_;
	masksize = masksize_;

	newimg = scalloc(imgsize * imgsize, sizeof(unsigned char));
	
	open_noc_connectors();
	spawn_slaves();
	
	/* Send mask. */
	for (int i = 0; i < nclusters; i++)
	{
		data_send(outfd[i], &masksize, sizeof(int));
		data_send(outfd[i], mask, masksize * masksize * sizeof(double));
	}

	int half = masksize/2;
	
	/* Process image in chunks. Each chunk includes a halo zone: total size (CHUNK_SIZE + masksize - 1)^2. */
	int chunk_with_halo_size = CHUNK_SIZE + masksize - 1;
	unsigned char *chunk = (unsigned char *) smalloc(chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char));
	msg = MSG_CHUNK;
	for (int i = half; i < imgsize - half; i += CHUNK_SIZE)
	{
		for (int j = half; j < imgsize - half; j += CHUNK_SIZE)
		{
			/* Build chunk. */
			printf("Building chunk[%d, %d]:\n", i - half, j - half);
			for (int k = 0; k < chunk_with_halo_size; k++)
				memcpy(&chunk[k * chunk_with_halo_size], &img[(i - half + k) * imgsize + j - half], chunk_with_halo_size);
			
			data_send(outfd[nchunks], &msg, sizeof(int));
			data_send(outfd[nchunks], chunk, chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char));

			/* Receives chunk without halo. */
			data_receive(infd, nchunks, chunk, CHUNK_SIZE * CHUNK_SIZE * sizeof(unsigned char));

			/* Build chunk. */
			for (int k = 0; k < CHUNK_SIZE; k++)
				memcpy(&newimg[(i + k)*imgsize + j], &chunk[k * CHUNK_SIZE], CHUNK_SIZE);

			nchunks = (nchunks == nclusters - 1) ? 0 : nchunks + 1;
		}
	}
	
	printf("OUTPUT MPPA:\n");
	for (int i = 0; i < imgsize; i++)
	{
		for (int j = 0; j < imgsize; j++)
			printf("%d ", newimg[i*imgsize + j]);
		printf("\n");
	}
	
	/* House keeping. */
	msg = MSG_DIE;
	for (int i = 0; i < nclusters; i++)
		data_send(outfd[i], &msg, sizeof(int));
	join_slaves();
	close_noc_connectors();
	free(chunk);
	free(newimg);
}
