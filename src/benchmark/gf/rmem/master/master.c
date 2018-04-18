/*
* Copyright(C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
* 
* master.c - gf master process.
*/

#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "master.h"
#include "../../kernel.h"

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
	int barrier;

	/* Setup parameters. */
	img = img_;
	mask = mask_;
	imgsize = imgsize_;
	masksize = masksize_;

	// for(int i = 0; i < imgsize; i++) {
	// 	for(int j = 0; j < imgsize; j++)
	// 		printf("%d ", img[imgsize * i + j]);
	// 	printf("\n");
	// }

	/* RMEM barrier. */
	barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(barrier);

	/* Write parameters to remote memory. */
	memwrite(OFF_NCLUSTERS, &nclusters, sizeof(int));
	memwrite(OFF_MASKSIZE,  &masksize,  sizeof(int));
	memwrite(OFF_IMGSIZE,   &imgsize,   sizeof(int));
	memwrite(OFF_MASK,      mask,       masksize*masksize*sizeof(double));
	memwrite(OFF_IMAGE,     img,        imgsize*imgsize*sizeof(unsigned char));

	/* Spawn slave processes. */
	spawn_slaves();

	/* Clean up before getting the output image result. */
	memset(img, 0, imgsize*imgsize*sizeof(unsigned char)); 

	/* Wait for all slave processes to finish. */
	join_slaves();

	memread(OFF_NEWIMAGE, img, imgsize*imgsize*sizeof(unsigned char));

	// for(int i = 0; i < imgsize; i++) {
	// 	for(int j = 0; j < imgsize; j++)
	// 		printf("%d ", img[imgsize * i + j]);
	// 	printf("\n");
	// }
	
	/* House keeping. */
	barrier_close(barrier);
}
