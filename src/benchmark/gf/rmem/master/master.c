/*
 * Copyright(C) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *                        Márcio Castro <mbcastro@gmail.com>
 * 
 * This file is part of CAP Bench.
 * 
 * CAP Bench is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 * 
 * CAP Bench is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * CAP Bench. If not, see <http://www.gnu.org/licenses/>.
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

	/* RMEM barrier. */
	barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(barrier);

	/* Write parameters to remote memory. */
	memwrite(OFF_NCLUSTERS, &nclusters, sizeof(int));
	memwrite(OFF_MASKSIZE,  &masksize,  sizeof(int));
	memwrite(OFF_IMGSIZE,   &imgsize,   sizeof(int));
	memwrite(OFF_MASK,      mask,       masksize*masksize*sizeof(double));
	memwrite(OFF_IMAGE,     img,        imgsize*imgsize*sizeof(unsigned char));

	/* Clean up before getting the output image result. */
	memset(img, 0, imgsize*imgsize*sizeof(unsigned char)); 
	memwrite(OFF_NEWIMAGE,     img,        imgsize*imgsize*sizeof(unsigned char));

	/* Spawn slave processes. */
	spawn_slaves();

	/* Wait for all slave processes to finish. */
	join_slaves();

	memread(OFF_NEWIMAGE,
		img,
		imgsize*imgsize*sizeof(unsigned char)
	);

	/* House keeping. */
	barrier_close(barrier);
}
