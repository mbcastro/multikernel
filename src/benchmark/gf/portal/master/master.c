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
	int msg;     /* Message.          */
	unsigned char *newimg;

	/* Setup parameters. */
	img = img_;
	mask = mask_;
	imgsize = imgsize_;
	masksize = masksize_;

	newimg = smalloc(imgsize*imgsize*sizeof(unsigned char));
	
	open_noc_connectors();
	spawn_slaves();
	
	 /* Send mask. */
	for (int i = 0; i < nclusters; i++)
	{
		data_send(outfd[i], &masksize, sizeof(int));
		data_send(outfd[i], mask, sizeof(double)*masksize*masksize);
	}
    
    /* Process image in chunks. */
	int half = masksize/2;
	msg = MSG_CHUNK;
	for (int i = half; i < imgsize - half; i += CHUNK_SIZE)
	{
		for (int j = half; j < imgsize - half; j += CHUNK_SIZE)
		{
			unsigned char chunk[(CHUNK_SIZE + masksize)*(CHUNK_SIZE + masksize)];

			/* Build chunk. */
			for (int k = 0; k < CHUNK_SIZE + masksize; k++)
				memcpy(&chunk[k*(CHUNK_SIZE + masksize)], &img[(i - half + k)*imgsize + j - half], CHUNK_SIZE + masksize);

			data_send(outfd[nchunks], &msg, sizeof(int));
			data_send(outfd[nchunks], chunk, (CHUNK_SIZE + masksize)*(CHUNK_SIZE + masksize));

					data_receive(infd,
						nchunks,
						&chunk,
						CHUNK_SIZE*CHUNK_SIZE
					);

					/* Build chunk. */
					for (int k = 0; k < CHUNK_SIZE; k++)
						memcpy(&newimg[(i -half + k)*imgsize + j - half], &chunk, CHUNK_SIZE);

			nchunks = (nchunks == nclusters - 1) ? 0 : nchunks + 1;
		}
	}
	for (int i = 0; i < masksize; i++)
	{
		for (int j = 0; j < masksize; j++)
			printf("%lf", mask[i*masksize + j]);
		printf("\n");
	}
	for (int i = 0; i < imgsize; i++)
	{
		for (int j = 0; j < imgsize; j++)
			printf("%d", img[i*imgsize + j]);
		printf("\n");
	}
	for (int i = 0; i < imgsize; i++)
	{
		for (int j = 0; j < imgsize; j++)
			printf("%d", newimg[i*imgsize + j]);
		printf("\n");
	}
	
	/* House keeping. */
	msg = MSG_DIE;
	for (int i = 0; i < nclusters; i++)
		data_send(outfd[i], &msg, sizeof(int));
	join_slaves();
	close_noc_connectors();
	free(newimg);
}
