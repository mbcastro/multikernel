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
  unsigned char *newimg; /* Output image.    */
  int barrier;
	
	/* Setup parameters. */
	img = img_;
	mask = mask_;
	imgsize = imgsize_;
	masksize = masksize_;

  /* Allocate output image. */
  newimg = scalloc(imgsize * imgsize, sizeof(unsigned char));

  /* RMEM barrier. */
  barrier = barrier_open(NR_IOCLUSTER);
  barrier_wait(barrier);

  /* Write parameters to remote memory. */
  memwrite(OFF_NCLUSTERS, &nclusters, sizeof(int));
  memwrite(OFF_MASKSIZE,  &masksize,  sizeof(int));
  memwrite(OFF_IMGSIZE,   &imgsize,   sizeof(int));
  memwrite(OFF_MASK,      &mask[0],   sizeof(double));
  memwrite(OFF_IMAGE,     &img[0],    sizeof(unsigned char));
  memwrite(OFF_NEWIMAGE,  &newimg[0], sizeof(unsigned char));

  /* Spawn slave processes. */
	spawn_slaves();
  
  /* Wait for all slave processes to finish. */
  join_slaves();

  /* House keeping. */
	barrier_close(barrier);
	free(newimg);
}
