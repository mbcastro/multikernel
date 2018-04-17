/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * Gaussian Filter Benchmark Kernel.
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

/* Timing statistics. */
long master = 0;          /* Time spent on master.        */
long slave[NR_CCLUSTER];  /* Time spent on slaves.        */
long communication = 0;   /* Time spent on communication. */

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
	int masksize; /* mask size.  */
	int imgsize;  /* Image size. */
};

/* Problem sizes. */
/* OUTPUT_IMG_SIZE + (MASK_SIZE-1) = INPUT_IMAGE_SIZE */
static struct problem tiny     = {  7,  1030 }; /* 1024  + (7-1)  = 1030  */
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
	long t[2];          /* Timings.                   */
	long time_init;     /* Total initialization time. */
	long time_kernel;   /* Total kernel time.         */
	double *mask;       /* Mask.                      */
	unsigned char *img; /* Image.                     */
	
	/*---------------------------------------------------------------*
	 * Benchmark Initialization                                      *
	 *---------------------------------------------------------------*/
	
	readargs(argc, argv);
	srandnum(seed);
	k1_timer_init();
	
	if (verbose)
		printf("initializing...\n");

	t[0] = k1_timer_get();
		img = smalloc(p->imgsize*p->imgsize*sizeof(char));
		for (int i = 0; i < p->imgsize*p->imgsize; i++)
			img[i] = randnum() & 0xff;
		mask = smalloc(p->masksize*p->masksize*sizeof(double));
		generate_mask(mask);
	t[1] = k1_timer_get();
	time_init = k1_timer_diff(t[0], t[1]);
		
	/*---------------------------------------------------------------*
	 * Applying filter                                               *
	 *---------------------------------------------------------------*/

	if (verbose)
		printf("applying filter...\n");

	t[0] = k1_timer_get();
		gauss_filter(img, p->imgsize, mask, p->masksize);
	t[1] = k1_timer_get();
	
	time_kernel = k1_timer_diff(t[0], t[1]);

	/*---------------------------------------------------------------*
	 * Print Timing Statistics                                       *
	 *---------------------------------------------------------------*/

	if (verbose)
	{
		printf("timing statistics:\n");
		printf("  initialization time: %f\n",  time_init*MICRO);
		printf("  kernel time:          %f\n", time_kernel*MICRO);
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
	 
	free(mask);
	free(img);
	
	return (0);
}
