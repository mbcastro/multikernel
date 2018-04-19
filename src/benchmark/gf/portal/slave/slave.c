/*
 * Copyright(C) 2014 Matheus M. Queiroz <matheus.miranda.queiroz@gmail.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include "slave.h"

/* Kernel parameters. */
static int masksize;       			                                                       /* Mask dimension.     */
static double mask[MASK_SIZE*MASK_SIZE];       			                                   /* Mask.               */
static unsigned char chunk[(CHUNK_SIZE + MASK_SIZE - 1)*(CHUNK_SIZE + MASK_SIZE - 1)]; /* Image input chunk.  */
static unsigned char newchunk[CHUNK_SIZE*CHUNK_SIZE];	                                 /* Image output chunk. */

/* Timing statistics. */
static uint64_t t[6] = { 0, 0, 0, 0, 0, 0 };
static uint64_t time_network[2] = { 0, 0 };
static uint64_t time_cpu = 0;
static int nwrite = 0;
static int nread = 0;
static size_t swrite = 0;
static size_t sread = 0;
	
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

	t[2] = k1_timer_get();

		/* Receives filter mask. */
		t[0] = k1_timer_get();
			data_receive(infd, &masksize, sizeof(int));
			data_receive(infd, mask, masksize*masksize*sizeof(double));
		t[1] = k1_timer_get();
		time_network[0] += k1_timer_diff(t[0], t[1]);
		nread += 2; sread += sizeof(int) + masksize*masksize*sizeof(double);

		/* Process chunks. */
		while (1)
		{
			t[0] = k1_timer_get();
				data_receive(infd, &msg, sizeof(int));
			t[1] = k1_timer_get();
			time_network[0] += k1_timer_diff(t[0], t[1]);
			nread++; sread += sizeof(int);

			/* Parse message. */
			switch (msg)
			{
				case MSG_CHUNK:
					t[0] = k1_timer_get();
						data_receive(infd, chunk, (CHUNK_SIZE + masksize - 1) * (CHUNK_SIZE + masksize - 1) * sizeof(unsigned char));
					t[1] = k1_timer_get();
					time_network[0] += k1_timer_diff(t[0], t[1]);
					nread++; sread += (CHUNK_SIZE + masksize - 1) * (CHUNK_SIZE + masksize - 1) * sizeof(unsigned char);
					gauss_filter();
					t[4] = k1_timer_get();
						data_send(outfd, newchunk, CHUNK_SIZE * CHUNK_SIZE * sizeof(unsigned char));
					t[5] = k1_timer_get();
					time_network[1] += k1_timer_diff(t[4], t[5]);
					nwrite++; swrite += CHUNK_SIZE * CHUNK_SIZE * sizeof(unsigned char);
					break;
					
				default:
					goto out;
			}
		}

out:
	
	t[3] = k1_timer_get();
	time_cpu = k1_timer_diff(t[2], t[3]) - (time_network[0] - time_network[1]);

	printf("%d;%" PRIu64 ";%" PRIu64 ";%" PRIu64 ";%d;%d;%d;%d\n",
		rank,
		time_network[0],
		time_network[1],
		time_cpu,
		nread,
		sread,
		nwrite,
		swrite
	);
	close_noc_connectors();
	
	return (0);
}
	
