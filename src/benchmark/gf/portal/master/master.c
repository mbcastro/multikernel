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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "master.h"

/* Gaussian Filter. */
static unsigned char *img;    /* Input image.        */
static int imgsize;           /* Dimension of image. */
static double *mask;          /* Mask.               */
static int masksize;          /* Dimension of mask.  */

/*
 * Gaussian filter.
 */
void gauss_filter(unsigned char *img_, int imgsize_, double *mask_, int masksize_)
{	
	unsigned char *newimg; /* Output image.  */
	int msg;               /* Message.       */
	int nchunks = 0;
	
	/* Setup parameters. */
	img = img_;
	mask = mask_;
	imgsize = imgsize_;
	masksize = masksize_;

	newimg = scalloc(imgsize * imgsize, sizeof(unsigned char));
	
	open_noc_connectors();
	spawn_slaves();
	
	/* Send mask and chunk size. */
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
			for (int k = 0; k < chunk_with_halo_size; k++)
				memcpy(&chunk[k * chunk_with_halo_size], &img[(i - half + k) * imgsize + j - half], chunk_with_halo_size * sizeof(unsigned char));
			
			data_send(outfd[nchunks], &msg, sizeof(int));
			data_send(outfd[nchunks], chunk, chunk_with_halo_size * chunk_with_halo_size * sizeof(unsigned char));

			/* Receives chunk without halo. */
			if (++nchunks == nclusters)
			{
				for (int ck = 0; ck < nchunks; ck++)
				{
					int ii;
					int jj;

					data_receive(infd, ck, chunk, CHUNK_SIZE*CHUNK_SIZE*sizeof(unsigned char));

					ii = i;
					jj = j - (nclusters - ck + 1)*CHUNK_SIZE;
					if (jj < 0)
					{
						jj = imgsize - half - jj;
						ii -= CHUNK_SIZE;
					}

					/* Build chunk. */
					for (int k = 0; k < CHUNK_SIZE; k++)
						memcpy(&newimg[(ii + k)*imgsize + jj], &chunk[k * CHUNK_SIZE], CHUNK_SIZE * sizeof(unsigned char));
				}

				nchunks = 0;
			}
		}
	}

	for (int ck = 0; ck < nchunks; ck++)
	{
		int ii;
		int jj;

		data_receive(infd, ck, chunk, CHUNK_SIZE*CHUNK_SIZE*sizeof(unsigned char));

		ii = imgsize - half - CHUNK_SIZE;
		jj = imgsize - half - (nclusters - ck + 1)*CHUNK_SIZE;
		if (jj < 0)
		{
			jj = imgsize - half - jj;
			ii -= CHUNK_SIZE;
		}

		/* Build chunk. */
		for (int k = 0; k < CHUNK_SIZE; k++)
			memcpy(&newimg[(ii + k)*imgsize + jj], &chunk[k * CHUNK_SIZE], CHUNK_SIZE * sizeof(unsigned char));
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
