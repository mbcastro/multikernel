/*
* Copyright(C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
* 
* master.c - Master process.
*/

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
	int j;               /* Loop indexes.     */ 
	size_t n;            /* Bytes to send.    */
	int msg;             /* Message.          */
	int nchunks;         /* Number of chunks. */

	/* Setup parameters. */
	img = img_;
	mask = mask_;
	imgsize = imgsize_;
	masksize = masksize_;
	
	open_noc_connectors();
	spawn_slaves();
	
	 /* Send mask. */
    n = sizeof(double)*masksize*masksize;	
	for (int i = 0; i < nclusters; i++)
	{
		data_send(outfd[i], &masksize, sizeof(int));
		data_send(outfd[i], mask, n);
	}
    
    /* Process image in chunks. */
    j = 0; n = CHUNK_SIZE*CHUNK_SIZE; msg = MSG_CHUNK;
    nchunks = (imgsize*imgsize)/(CHUNK_SIZE*CHUNK_SIZE);
    for (int i = 0; i < nchunks; i++)
    {		
		data_send(outfd[j], &msg, sizeof(int));
		data_send(outfd[j], &img[i*(CHUNK_SIZE*CHUNK_SIZE)],n);
		
		j++;
		
		/* 
		 * Slave processes are busy.
		 * So let's wait for results.
		 */
		if (j == nclusters)
		{
			for (/* NOOP */ ; j > 0; j--)
			{
				data_receive(infd,nclusters-j,
								  &img[(nclusters-j)*CHUNK_SIZE*CHUNK_SIZE], n);
			}
		}
	}
	
	/* Receive remaining results. */
	for (/* NOOP */ ; j > 0; j--)
	{
		data_receive(infd, j-1, 
						   &img[(nchunks - j)*CHUNK_SIZE*CHUNK_SIZE], n);
	}
	
	/* House keeping. */
	msg = MSG_DIE;
	for (int i = 0; i < nclusters; i++)
		data_send(outfd[i], &msg, sizeof(int));
	join_slaves();
	close_noc_connectors();
}
