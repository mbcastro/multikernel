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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "master.h"

/*
 * Problem.
 */
struct problem
{
	int masksize; /* mask size.  */
	int imgsize;  /* Image size. */
};

/* Problem sizes. */
static struct problem tiny     = {  7,    70};  /*   64  + (7-1)  =    70 */
static struct problem small    = {  7,  2054 }; /* 2048  + (7-1)  =  2054 */
static struct problem standard = {  7,  4102 }; /* 4096  + (7-1)  =  4102 */
static struct problem large    = { 11,  8202 }; /* 8192  + (11-1) =  8202 */
static struct problem huge     = { 15, 16398 }; /* 16384 + (15-1) = 16398 */

/* Benchmark parameters. */
int nclusters = NR_CCLUSTER;      /* Number of threads. */
static int verbose = 0;           /* Be verbose?        */
static int seed = 0;              /* Seed value.        */
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
	double sec;
	double first;
	double total;
	
	first = 1.0/(2.0*PI*SD*SD);
	half = p->masksize >> 1;
	total = 0;
	
	#define MASK(i, j) \
		mask[(i)*p->masksize + (j)]

	for (int i = -half; i <= half; i++)
	{
		for (int j = -half; j <= half; j++)
		{
			sec = -((i*i + j*j)/2.0*SD*SD);
			sec = pow(E, sec);

			MASK(i + half, j + half) = first*sec;
			total += MASK(i + half, j + half);
		}
	}
	
	for (int i = 0 ; i < p->masksize; i++)
	{
		for (int j = 0; j < p->masksize; j++)
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

	img = smalloc(p->imgsize*p->imgsize*sizeof(unsigned char));
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
