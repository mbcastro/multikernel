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

#include <mppa/osconfig.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "master.h"

/* Gaussian filter. */
extern void gauss_filter
(unsigned char *img, int imgsize, double *mask, int masksize);

/*
 * Problem.
 */
struct problem
{
	int masksize; /* mask size.  */
	int imgsize;  /* Image size. */
};

/* Problem sizes. */
static struct problem tiny     = {  7,  70   }; /* 64  + (7-1)  = 70  */
static struct problem small    = {  7,  2054 }; /* 2048  + (7-1)  = 2054  */
static struct problem standard = {  7,  4102 }; /* 4096  + (7-1)  = 4102  */
static struct problem large    = { 11,  8202 }; /* 8192  + (11-1) = 8202  */
static struct problem huge     = { 15, 16398 }; /* 16384 + (15-1) = 16398 */

/* Benchmark parameters. */
int verbose = 0;                  /* Be verbose?        */
static int seed = 0;              /* Seed value.        */
int nclusters = NR_CCLUSTER;      /* Number of threads. */
static struct problem *p = &tiny; /* Problem.           */

/*===================================================================*
 * usage()                                                           *
 *===================================================================*/

/*
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: gf [options]\n");
	printf("Brief: Gaussian Filter Benchmark Kernel\n");
	printf("Options:\n");
	printf("  --help             Display this information and exit\n");
	printf("  --nclusters <value> Set number of threads\n");
	printf("  --class <name>     Set problem class:\n");
	printf("                       - small\n");
	printf("                       - standard\n");
	printf("                       - large\n");
	printf("                       - huge\n");
	printf("  --verbose          Be verbose\n");
	exit(0);
}

/*===================================================================*
 * readargs()                                                        *
 *===================================================================*/

/*
 * Reads command line arguments.
 */
static void readargs(int argc, char **argv)
{
	char *arg; /* Working argument. */
	int state; /* Processing state. */

	enum readargs_states {
		READ_ARG,      /* Read argument.         */
		SET_nclusters, /* Set number of threads. */
		SET_CLASS      /* Set problem class.     */
	};
	
	state = READ_ARG;
	/* Read command line arguments. */
	for (int i = 1; i < argc; i++)
	{
		arg = argv[i];
		
		/* Set value. */
		if (state != READ_ARG)
		{
			switch (state)
			{
				/* Set problem class. */
				case SET_CLASS :
					if (!strcmp(argv[i], "tiny"))
						p = &tiny;
					else if (!strcmp(argv[i], "small"))
						p = &small;
					else if (!strcmp(argv[i], "standard"))
						p = &standard;
					else if (!strcmp(argv[i], "large"))
						p = &large;
					else if (!strcmp(argv[i], "huge"))
						p = &huge;
					else 
						usage();
					state = READ_ARG;
					break;
				
				/* Set number of threads. */
				case SET_nclusters :
					nclusters = atoi(arg);
					state = READ_ARG;
					break;
				
				default:
					usage();			
			}
			
			continue;
		}
		/* Parse argument. */
		if (!strcmp(arg, "--verbose"))
			verbose = 1;
		else if (!strcmp(arg, "--nclusters"))
			state = SET_nclusters;
		else if (!strcmp(arg, "--class"))
			state = SET_CLASS;
		else
			usage();
	}
	
	/* Invalid argument(s). */
	if (nclusters < 1)
		usage();
}

/*===================================================================*
 * generate_mask()                                                   *
 *===================================================================*/

/*
 * Generates mask.
 */
static void generate_mask(double *mask)
{
	int half;
	int i, j;
	double sec;
	double first;
	double total;
	
	first = 1.0/(2.0*PI*SD*SD);
	half = p->masksize >> 1;
	total = 0;
	
	#define MASK(i, j) \
		mask[(i)*p->masksize + (j)]

	for (i = -half; i <= half; i++)
	{
		for (j = -half; j <= half; j++)
		{
			sec = -((i*i + j*j)/2.0*SD*SD);
			sec = pow(E, sec);

			MASK(i + half, j + half) = first*sec;
			total += MASK(i + half, j + half);
		}
	}
	
	for (i = 0 ; i < p->masksize; i++)
	{
		for (j = 0; j < p->masksize; j++)
			MASK(i, j) /= total;
	}
}

/*===================================================================*
 * main()                                                            *
 *===================================================================*/

/*
 * Runs benchmark.
 */
int main(int argc, char **argv)
{
	double *mask;       /* Mask.  */
	unsigned char *img; /* Image. */
	
	/*---------------------------------------------------------------*
	 * Benchmark Initialization                                      *
	 *---------------------------------------------------------------*/
	
	readargs(argc, argv);
	srandnum(seed);
	
	if (verbose)
		printf("initializing...\n");

	img = smalloc(p->imgsize*p->imgsize*sizeof(char));
	for (int i = 0; i < p->imgsize*p->imgsize; i++)
		img[i] = randnum() & 0xff;
	mask = smalloc(p->masksize*p->masksize*sizeof(double));
	generate_mask(mask);
		
	/*---------------------------------------------------------------*
	 * Applying filter                                               *
	 *---------------------------------------------------------------*/

	if (verbose)
		printf("applying filter...\n");

	gauss_filter(img, p->imgsize, mask, p->masksize);

	/*---------------------------------------------------------------*
	 * House Keeping                                                 *
	 *---------------------------------------------------------------*/
	 
	free(mask);
	free(img);
	
	return (0);
}
