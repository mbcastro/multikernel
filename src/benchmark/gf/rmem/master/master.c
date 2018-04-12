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
#include "../kernel.h"


/*
 * Gaussian filter.
 */
void gauss_filter(
    unsigned char *img,
    int imgsize,
    double *mask,
    int masksize)
{	
	int barrier;
	int j;               /* Loop indexes.     */ 
	int msg;             /* Message.          */
	int nchunks;         /* Number of chunks. */
	size_t n;            /* Bytes to send.    */

	if(verbose)
		printf("writing to remote memory\n");

	barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(barrier);

	/* Write parameters to remote memory. */
	memwrite(OFF_NCLUSTERS,            &nclusters,       sizeof(int));
	memwrite(OFF_MASK_SIZE,            &masksize,        sizeof(int));
	memwrite(OFF_IMAGE_SIZE,           &imgsize,         sizeof(int));
	memwrite(OFF_MASK,                 &mask[0],         masksize*masksize*sizeof(double));
	memwrite(OFF_IMAGE,                &img[0],          imgsize*imgsize*sizeof(double));

	if(verbose)
		printf("spawning slaves\n");

	spawn_slaves();
	
	join_slaves();

	/* Read parameters from remote memory. */
	memread(OFF_IMAGE, img, imgsize*imgsize*sizeof(double));

	barrier_close(barrier);
}
