/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * slave.c - k-means slave process.
 */

#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <omp.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "slave.h"

#define NUM_THREADS 16

#define DIMENSION          16
#define NUM_POINTS     262144
#define NUM_CENTROIDS    1024

/* Size of arrays. */
#define MAP_SIZE (NUM_POINTS/NR_CCLUSTER)                /* map[]         */
#define POINTS_SIZE ((NUM_POINTS/NR_CCLUSTER)*DIMENSION) /* points[]      */
#define CENTROIDS_SIZE (NUM_CENTROIDS*DIMENSION)         /* centroids[]   */
#define PPOPULATION_SIZE (NUM_CENTROIDS)                 /* ppopulation[] */
#define LCENTROIDS_SIZE ((NUM_CENTROIDS/1)*DIMENSION)    /* lcentroids[]  */

/* Delta of array sizes. */
#define DELTA (NR_CCLUSTER - 1)

/*
 * Returns the ith data point.
 */
#define POINT(i) \
	(&points[(i)*dimension])

/*
 * Returns the ith centroid.
 */
#define CENTROID(i) \
	(&centroids[(i)*dimension])

/* K-means. */
int dimension;
static int nprocs;
static float mindistance;
static int ncentroids;
static float points[POINTS_SIZE + DELTA*DIMENSION];                    /* Data points.               */
static float centroids[CENTROIDS_SIZE + NR_CCLUSTER*DELTA*DIMENSION]; /* Data centroids.            */
static int map[MAP_SIZE + DELTA];                                      /* Map of clusters.           */
static int too_far[NR_CCLUSTER*NUM_THREADS];                          /* Are points too far?        */
static int has_changed[NR_CCLUSTER*NUM_THREADS];                      /* Hash any centroid changed? */
static int lncentroids[NR_CCLUSTER];                                  /* Local number of centroids. */
static int lnpoints;                                                   /* Local number of points.    */
static int ppopulation[PPOPULATION_SIZE + NR_CCLUSTER*DELTA];         /* Partial population.        */
static float lcentroids[LCENTROIDS_SIZE + DELTA*DIMENSION];            /* Local centroids.           */

/* Thread communication. */
static omp_lock_t lock[NUM_THREADS];

/* Timing statistics. */
static uint64_t t[6] = { 0, 0, 0, 0, 0, 0 };
static uint64_t time_network[2] = { 0, 0 };
static uint64_t time_cpu = 0;
static int nwrite = 0;
static int nread = 0;
static size_t swrite = 0;
static size_t sread = 0;

/*===================================================================*
 * populate()                                                        *
 *===================================================================*/

/*
 * Populates clusters.
 */
static void populate(void)
{
	memset(&too_far[rank*NUM_THREADS], 0, NUM_THREADS*sizeof(int)); 
	
	/* Iterate over data points. */
	#pragma omp parallel for
	for (int i = 0; i < lnpoints; i++)
	{
		float distance;

		distance = vector_distance(CENTROID(map[i]), POINT(i));
		
		/* Look for closest cluster. */
		for (int j = 0; j < ncentroids; j++)
		{
			float tmp;

			/* Point is in this cluster. */
			if (j == map[i])
				continue;
				
			tmp = vector_distance(CENTROID(j), POINT(i));
			
			/* Found. */
			if (tmp < distance)
			{
				map[i] = j;
				distance = tmp;
			}
		}
		
		/* Cluster is too far away. */
		if (distance > mindistance)
			too_far[rank*NUM_THREADS + omp_get_thread_num()] = 1;
	}
}

/*===================================================================*
 * compute_centroids()                                               *
 *===================================================================*/

/*
 * Returns the population for clusther ith, vector jth.
 */
#define POPULATION(i, j) \
	(&ppopulation[(i)*lncentroids[rank] + (j)])

/*
 * Returns the jth input centroid from the ith cluster.
 */
#define PCENTROID(i, j) \
	(&centroids[((i)*lncentroids[rank] + (j))*dimension])

/*
 * Returns the ith local centroid.
 */
#define LCENTROID(i) \
	(&lcentroids[(i)*dimension])

/*
 * Synchronizes partial centroids.
 */
static void sync_pcentroids(void)
{	
	ssize_t n;
	
	/* Send partial centroids. */
	n = ncentroids*dimension*sizeof(float);
	t[4] = k1_timer_get();
		data_send(outfd, centroids, n);
	t[5] = k1_timer_get();
	time_network[1] += k1_timer_diff(t[4], t[5]);
	nwrite++; swrite += n;
	
	/* Receive partial centroids. */
	n = nprocs*lncentroids[rank]*dimension*sizeof(float);
	t[0] = k1_timer_get();
		data_receive(infd, centroids, n);
	t[1] = k1_timer_get();
	time_network[0] += k1_timer_diff(t[0], t[1]);
	nread++; sread += n;
}

/*
 * Synchronizes partial population.
 */
static void sync_ppopulation(void)
{
	ssize_t n;
	
	/* Send partial population. */
	n = ncentroids*sizeof(int);
	t[4] = k1_timer_get();
		data_send(outfd, ppopulation, n);
	t[5] = k1_timer_get();
	time_network[1] += k1_timer_diff(t[4], t[5]);
	nwrite++; swrite += n;
	
	/* Receive partial population. */
	n = nprocs*lncentroids[rank]*sizeof(int);
	t[0] = k1_timer_get();
		data_receive(infd, ppopulation, n);
	t[1] = k1_timer_get();
	time_network[0] += k1_timer_diff(t[0], t[1]);
	nread++; sread += n;
}

/*
 * Synchronizes centroids.
 */
static void sync_centroids(void)
{
	ssize_t n;
	
	n = lncentroids[rank]*dimension*sizeof(float);
	t[4] = k1_timer_get();
		data_send(outfd, lcentroids, n);
	t[5] = k1_timer_get();
	time_network[1] += k1_timer_diff(t[4], t[5]);
	nwrite++; swrite += n;
	
	n = ncentroids*dimension*sizeof(float);
	t[0] = k1_timer_get();
		data_receive(infd, centroids, n);
	t[1] = k1_timer_get();
	time_network[0] += k1_timer_diff(t[0], t[1]);
	nread++; sread += n;
}

/*
 * Synchronizes status.
 */
static void sync_status(void)
{
	ssize_t n;
	
	n = NUM_THREADS*sizeof(int);
	t[4] = k1_timer_get();
		data_send(outfd, &has_changed[rank*NUM_THREADS], n);
		data_send(outfd, &too_far[rank*NUM_THREADS], n);
	t[5] = k1_timer_get();
	time_network[1] += k1_timer_diff(t[4], t[5]);
	nwrite += 2; swrite += n + n;
		
	n = nprocs*NUM_THREADS*sizeof(int);
	t[0] = k1_timer_get();
		data_receive(infd, has_changed, n);
		data_receive(infd, too_far, n);
	t[1] = k1_timer_get();
	time_network[0] += k1_timer_diff(t[0], t[1]);
	nread += 2; sread += n + n;
}

/*
 * Computes clusters' centroids.
 */
static void compute_centroids(void)
{
	int population;
	
	memcpy(lcentroids, CENTROID(rank*(ncentroids/nprocs)), lncentroids[rank]*dimension*sizeof(float));
	memset(&has_changed[rank*NUM_THREADS], 0, NUM_THREADS*sizeof(int));
	memset(centroids, 0, (ncentroids + DELTA*nprocs)*dimension*sizeof(float));
	memset(ppopulation, 0, (ncentroids + nprocs*DELTA)*sizeof(int));

	/* Compute partial centroids. */
	#pragma omp parallel for
	for (int i = 0; i < lnpoints; i++)
	{
		int j;

		j = map[i]%NUM_THREADS;
		
		omp_set_lock(&lock[j]);
		
		vector_add(CENTROID(map[i]), POINT(i));
			
		ppopulation[map[i]]++;
		
		omp_unset_lock(&lock[j]);
	}
	
	sync_pcentroids();
	sync_ppopulation();
	
	/* Compute centroids. */
	#pragma omp parallel for private(population)
	for (int j = 0; j < lncentroids[rank]; j++)
	{
		population = 0;
		
		for (int i = 0; i < nprocs; i++)
		{
			if (*POPULATION(i, j) == 0)
				continue;
			
			population += *POPULATION(i, j);
			
			if (i == rank)
				continue;
			
			vector_add(PCENTROID(rank, j), PCENTROID(i, j));
		}
		
		if (population > 1)
			vector_mult(PCENTROID(rank, j), 1.0/population);
		
		/* Cluster mean has changed. */
		if (!vector_equal(PCENTROID(rank, j), LCENTROID(j)))
		{
			has_changed[rank*NUM_THREADS + omp_get_thread_num()] = 1;
			vector_assign(LCENTROID(j), PCENTROID(rank, j));
		}
	}
	
	sync_centroids();
		
	sync_status();
}

/*============================================================================*
 *                                 again()                                    *
 *============================================================================*/

/*
 * Asserts if another iteration is needed.
 */
static int again(void)
{
	/* Checks if another iteration is needed. */	
	for (int i = 0; i < nprocs*NUM_THREADS; i++)
	{
		if (has_changed[i] && too_far[i])
			return (1);
	}
			
	return (0);
}

/*============================================================================*
 *                                kmeans()                                    *
 *============================================================================*/

/*
 * Clusters data.
 */
static void kmeans(void)
{	
	omp_set_num_threads(NUM_THREADS);

	for (int i = 0; i < NUM_THREADS; i++)
		omp_init_lock(&lock[i]);
	
	/* Cluster data. */
	do
	{	
		populate();
		compute_centroids();
	} while (again());
}

/*============================================================================*
 *                                 getwork()                                  *
 *============================================================================*/

/*
 * Receives work from master process.
 */
static void getwork(void)
{	
	ssize_t n;
	
	t[0] = k1_timer_get();
		data_receive(infd, &lnpoints, sizeof(int));
		data_receive(infd, &nprocs, sizeof(int));
		data_receive(infd, &ncentroids, sizeof(int));
		data_receive(infd, &mindistance, sizeof(float));
		data_receive(infd, &dimension, sizeof(int));
		sread += n = nprocs*sizeof(int);
		data_receive(infd, lncentroids, n);
		n = dimension*sizeof(float);
		for (int i = 0; i < lnpoints; i++)
		{
			data_receive(infd, &points[i*dimension], n);
			nread++; sread += n;
		}
		sread += n = ncentroids*dimension*sizeof(float);
		data_receive(infd, centroids, n);
		sread += n = lnpoints*sizeof(int);
		data_receive(infd, map, n);
	t[1] = k1_timer_get();
	time_network[0] += k1_timer_diff(t[0], t[1]);
	nread += 8; sread += sizeof(int) + sizeof(float);
}

/*============================================================================*
 *                                  main()                                    *
 *============================================================================*/

/*
 * Clusters data.
 */
int main(int argc, char **argv)
{
	((void)argc);
	
	rank = atoi(argv[0]);
	
	k1_timer_init();
	
	open_noc_connectors();
	t[2] = k1_timer_get();
		getwork();
		kmeans();
	t[3] = k1_timer_get();
	time_cpu = k1_timer_diff(t[2], t[3]) - (time_network[0] + time_network[1]);

	/* House keeping. */
	close_noc_connectors();

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

	return (0);
}
