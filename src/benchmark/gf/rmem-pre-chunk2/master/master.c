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
#include <stdlib.h>
#include <string.h>
#include "master.h"

/**
 * @brief Convolutes a Gaussian filter on an image. 
 *
 * @param img     Input image.
 * @param imgsize Size of input image.
 * @param mask    Gaussian mask.
 * @param massize Size of mask.
 */
void gauss_filter(unsigned char *img, int imgsize, const double *mask, int masksize, unsigned char *chunk, int chunksize)
{	
	int barrier;
	const int imgsize2 = imgsize*imgsize;
	const int masksize2 = masksize*masksize;

	/* RMEM barrier. */
	barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(barrier);

	/* Write parameters to remote memory. */
	memwrite(OFF_MASKSIZE,  &masksize,  sizeof(int));
    memwrite(OFF_IMGSIZE,   &imgsize,   sizeof(int));
	memwrite(OFF_CHUNKSIZE, &chunksize, sizeof(int));
	memwrite(OFF_MASK,      mask,       masksize2*sizeof(double));
	memwrite(OFF_CHUNKS,    chunk,      chunksize*sizeof(unsigned char));

	/* Clean up before getting the output image. */
	memset(img, 0, imgsize2*sizeof(unsigned char)); 
	memwrite(OFF_IMAGE, img, imgsize2*sizeof(unsigned char));

	/* Spawn slave processes. */
	spawn_slaves();

	/* Wait for slaves processes. */
	join_slaves();

	/* Read output image. */
	memread(OFF_IMAGE,
		img,
		imgsize2*sizeof(unsigned char)
	);

    unsigned char *newimg = smalloc(imgsize*imgsize*sizeof(unsigned char));
    memset(newimg, 0, imgsize2*sizeof(unsigned char)); 

    /*
     * Rearrange chunks.
     */
    int half_mask = masksize >> 1;
    int chunk_offset = 0;
    for (int chunkI = 0; chunkI < imgsize-masksize+1; chunkI += CHUNK_SIZE) {
        for (int chunkJ = 0; chunkJ < imgsize-masksize+1; chunkJ += CHUNK_SIZE) {
            for (int chunk_rows = 0; chunk_rows < CHUNK_SIZE; ++chunk_rows) {
                memcpy(&newimg[(chunkI+half_mask)*imgsize + (chunk_rows*imgsize) + chunkJ+half_mask], &img[chunk_offset], CHUNK_SIZE);
                chunk_offset += CHUNK_SIZE;
            }
        }
    }

	/* House keeping. */
	barrier_close(barrier);
    free(newimg);
}
