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
#include <string.h>
#include <stdlib.h>
#include "master.h"

#define MICRO (1.0/1000000)

/*
 * Problem.
 */
struct problem
{
	int npoints;       /* Number of points.    */
	int dimension;     /* Data dimension.      */
	int ncentroids;    /* Number of centroids. */
	float mindistance; /* Minimum distance.    */
};

/* Problem sizes. */
static struct problem tiny     = {  4096, 16,  256, 0.0 };
static struct problem small    = {  8192, 16,  512, 0.0 };
static struct problem standard = { 16384, 16, 1024, 0.0 };
static struct problem large    = { 32768, 16, 1024, 0.0 };
static struct problem huge     = { 65536, 16, 1024, 0.0 };

/* Benchmark parameters. */
static int verbose = 0;           /* Be verbose?        */
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
	printf("Usage: kmeans [options]\n");
	printf("Brief: Kmeans Benchmark Kernel\n");
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
 * main()                                                            *
 *===================================================================*/

/*
 * Runs benchmark.
 */
int main(int argc, char **argv)
{
	float *data;
	
	/*---------------------------------------------------------------*
	 * Benchmark Initialization                                      *
	 *---------------------------------------------------------------*/
	
	readargs(argc, argv);
	srandnum(seed);

	printf("Number of Points:    %d\n", p->npoints);
	printf("Number of Centroids: %d\n", p->ncentroids);
	printf("Dimension:           %d\n", p->dimension);
	printf("Number of Clusters:  %d\n", nclusters);

	if (verbose)
		printf("initializing...\n");

	data = smalloc(p->npoints*p->dimension*sizeof(float));
	for (int i = 0; i < p->npoints; i++)
		vector_random(&data[i*p->dimension], p->dimension);
	
	/*---------------------------------------------------------------*
	 * Cluster Data                                                  *
	 *---------------------------------------------------------------*/

	if (verbose)
		printf("clustering data...\n");

	kmeans(data,
		p->npoints,
		p->dimension,
		p->ncentroids,
		p->mindistance
	);
	
	/*---------------------------------------------------------------*
	 * House Keeping                                                 *
	 *---------------------------------------------------------------*/

	free(data);
	
	return (0);
}
