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

#include <nanvix/arch/mppa.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <timer.h>
#include "master.h"

#define MICROSEC (1.0/1000000)

/* Bucket sort algorithm. */
extern void bucketsort(int *array, int n);

/* Timing statistics. */
long master = 0;          /* Time spent on master.        */
long slave[NR_CCLUSTER]; /* Time spent on slaves.        */
long communication = 0;   /* Time spent on communication. */
long total = 0;           /* Total time.                  */

/* Data exchange statistics. */
size_t data_sent = 0;     /* Number of bytes received. */
unsigned nsend = 0;       /* Number of sends.          */
size_t data_received = 0; /* Number of bytes sent.     */
unsigned nreceive = 0;    /* Number of receives.       */

/* Problem. */
struct problem
{
    int n; /* Number of elements. */
};

/* Problem sizes. */
static struct problem tiny     = {   8388608 };
static struct problem small    = {  16777216 };
static struct problem standard = {  33554432 };
static struct problem large    = {  67108864 };
static struct problem huge     = { 134217728 };

/* Benchmark parameters. */
int verbose = 0;                  /* Be verbose?        */
int nclusters = 16;                /* Number of threads. */
static int seed = 0;              /* Seed number.       */
static struct problem *p = &tiny; /* Problem.           */

/* Prints program usage and exits. */
static void usage(void)
{
    printf("Usage: insertion_sort [options]\n");
    printf("Brief: Insertion Sort Benchmark Kernel\n");
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

/* Reads command line arguments. */
static void readargs(int argc, char **argv)
{
    int i;     /* Loop index.       */
    char *arg; /* Working argument. */
    int state; /* Processing state. */

    /* State values. */
#define READ_ARG     0 /* Read argument.         */
#define SET_nclusters 1 /* Set number of threads. */
#define SET_CLASS    2 /* Set problem class.     */

    state = READ_ARG;

    /* Read command line arguments. */
    for (i = 1; i < argc; i++)
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

/* Runs benchmark. */
int main(int argc, char **argv)
{
    int i;          /* Loop index.         */
    int *a;         /* Array to be sorted. */
    long end;       /* End time.           */
    long start;     /* Start time.         */

    readargs(argc, argv);

    k1_timer_init();
    srandnum(seed);

    /* Benchmark initialization. */
    if (verbose)
        printf("initializing...\n");
    start = k1_timer_get();
    a = smalloc(p->n*sizeof(int));
    for (i = 0; i < p->n; i++)
        a[i] = randnum() & 0xfffff;
    end = k1_timer_get();
    if (verbose)
        printf("  time spent: %f\n", k1_timer_diff(start, end)*MICROSEC);

    /* Cluster data. */
    if (verbose)
        printf("sorting...\n");
    start = k1_timer_get();
    bucketsort(a, p->n);
    end = k1_timer_get();
    total = k1_timer_diff(start, end);

    /* Print timing statistics. */
    printf("timing statistics:\n");
    printf("  master:        %f\n", master*MICROSEC);
    for (i = 0; i < nclusters; i++)
        printf("  slave %d:      %f\n", i, slave[i]*MICROSEC);
    printf("  communication: %f\n", communication*MICROSEC);
    printf("  total time:    %f\n", total*MICROSEC);
    printf("data exchange statistics:\n");
    printf("  data sent:            %d\n", data_sent);
    printf("  number sends:         %u\n", nsend);
    printf("  data received:        %d\n", data_received);
    printf("  number receives:      %u\n", nreceive);

    /* House keeping. */
    free(a);

    return (0);
}
