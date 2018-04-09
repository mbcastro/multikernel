/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * Kmeans Benchmark Kernel.
 */

#include <nanvix/arch/mppa.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "master.h"

#define MICRO (1.0/1000000)

/* Timing statistics. */
long master = 0;         /* Time spent on master.        */
long slave[NR_CCLUSTER]; /* Time spent on slaves.        */
long communication = 0;  /* Time spent on communication. */

/* Data exchange statistics. */
size_t data_sent = 0;     /* Number of bytes received. */
unsigned nsend = 0;       /* Number of sends.          */
size_t data_received = 0; /* Number of bytes sent.     */
unsigned nreceive = 0;    /* Number of receives.       */

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
	int *map;         /* Map of clusters.           */
	long t[2];        /* Timings.                   */
	float *data;      /* Data points.               */
	long time_init;   /* Total initialization time. */
	long time_kernel; /* Total kernel time.         */
	
	/*---------------------------------------------------------------*
	 * Benchmark Initialization                                      *
	 *---------------------------------------------------------------*/
	
	readargs(argc, argv);
	srandnum(seed);
	k1_timer_init();

	if (verbose)
		printf("initializing...\n");

	t[0] = k1_timer_get();
		data = smalloc(p->npoints*sizeof(float));
		for (int i = 0; i < p->npoints; i++)
			vector_random(&data[i*p->dimension], dimension);
	t[1] = k1_timer_get();
	time_init = k1_timer_diff(t[0], t[1]);
	
	/*---------------------------------------------------------------*
	 * Cluster Data                                                  *
	 *---------------------------------------------------------------*/

	if (verbose)
		printf("clustering data...\n");

	t[0] = k1_timer_get();
		map = kmeans(data,
			p->npoints,
			p->dimension,
			p->ncentroids,
			p->mindistance
		);
	t[1] = k1_timer_get();
	time_kernel = k1_timer_diff(t[0], t[1]);

	/*---------------------------------------------------------------*
	 * Print Timing Statistics                                       *
	 *---------------------------------------------------------------*/

	printf("timing statistics:\n");

	printf("  initialization time: %f\n",  time_init*MICRO);
	printf("  kernel time:          %f\n", time_kernel*MICRO);

	if (verbose)
	{
		printf("  master:        %f\n", master*MICRO);
		for (int i = 0; i < nclusters; i++)
			printf("  slave %d:      %f\n", i, slave[i]*MICRO);
		printf("  communication: %f\n", communication*MICRO);
		printf("data exchange statistics:\n");
		printf("  data sent:            %d\n", data_sent);
		printf("  number sends:         %u\n", nsend);
		printf("  data received:        %d\n", data_received);
		printf("  number receives:      %u\n", nreceive);
	}
	
	/*---------------------------------------------------------------*
	 * House Keeping                                                 *
	 *---------------------------------------------------------------*/

	free(map);
	free(data);
	
	return (0);
}
