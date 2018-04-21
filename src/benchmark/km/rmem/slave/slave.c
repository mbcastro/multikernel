/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * slave.c - k-means slave process.
 */

#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <nanvix/mm.h>
#include <assert.h>
#include <math.h>
#include <omp.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "slave.h"
#include "../../kernel.h"

#define NTHREADS 13

#define DELTA (NR_CCLUSTER - 1)

int rank;
static int barrier;

/* Global K-means Data */
static int   nclusters;
static float mindistance;
static float centroids[LENGTH_CENTROIDS];
static int   ncentroids;
static int   has_changed[LENGTH_HAS_CHANGED];
static int   too_far[LENGTH_TOO_FAR];
       int   dimension;

/* Local K-means Data */
static int   lnpoints;
static int   lncentroids;
static int   ltoo_far[NTHREADS];
static int   lhas_changed[NTHREADS];
static int   lmap[MAX_POINTS/4 + DELTA];
static float lpoints[(MAX_POINTS/4 + DELTA)*MAX_DIMENSION];
static float lcentroids[(MAX_CENTROIDS + DELTA)*MAX_DIMENSION];
static float lpcentroids[(MAX_CENTROIDS + DELTA)*MAX_DIMENSION];
static int   lpopulation[MAX_CENTROIDS + DELTA];
static int   lppopulation[MAX_CENTROIDS + DELTA];

/* Timing statistics. */
static uint64_t t[6] = { 0, 0, 0, 0, 0, 0 };
static uint64_t time_network[2] = { 0, 0 };
static uint64_t time_cpu = 0;
static int nwrite = 0;
static int nread = 0;
static size_t swrite = 0;
static size_t sread = 0;

/**
 * @brief Helper macros for array indexing.
 */
/**@{*/
#define LPOINT(x)        lpoints[(x)*dimension]
#define CENTROID(x,y)    centroids[((x)*(ncentroids/nclusters) + (y))*dimension]
#define LCENTROID(x,y)   lcentroids[((x)*(ncentroids/nclusters) + (y))*dimension]
#define LPCENTROID(x)    lpcentroids[(x)*dimension]
#define LPOPULATION(x,y) lpopulation[(x)*(ncentroids/nclusters) + (y)]
#define LPPOPULATION(x)  lppopulation[(x)]
/**@}*/

/* Thread communication. */
static omp_lock_t lock[NTHREADS];

/*===================================================================*
 * populate()                                                        *
 *===================================================================*/

/*
 * Populates clusters.
 */
static void populate(void)
{
	size_t n;

	barrier_wait(barrier);

	n = ncentroids*dimension*sizeof(float);
	assert(n <= sizeof(centroids));
	t[0] = k1_timer_get();
		memread(OFF_CENTROIDS, &centroids[0], n);
	t[1] = k1_timer_get();
	time_network[0] += k1_timer_diff(t[0], t[1]);
	nread++; sread += n;

	memset(ltoo_far, 0, NTHREADS*sizeof(int));
	
	/* Iterate over data points. */
	#pragma omp parallel for
	for (int i = 0; i < lnpoints; i++)
	{
		int tid;
		float distance;

		tid = omp_get_thread_num();

		distance = vector_distance(&CENTROID(0, lmap[i]), &LPOINT(i));
		
		/* Look for closest cluster. */
		for (int j = 0; j < ncentroids; j++)
		{
			float tmp;

			/* Point is in this cluster. */
			if (j == lmap[i])
				continue;
				
			tmp = vector_distance(&CENTROID(0, j), &LPOINT(i));
			
			/* Found. */
			if (tmp < distance)
			{
				lmap[i] = j;
				distance = tmp;
			}
		}
		
		/* Cluster is too far away. */
		if (distance > mindistance)
			ltoo_far[tid] = 1;
	}
}

/*====================================================================*
 * compute_pcentroids()                                               *
 *====================================================================*/

/**
 * @brief Computes partial centroids.
 */
static inline void compute_pcentroids(void)
{
	size_t n;

	memset(&lhas_changed,      0, NTHREADS*sizeof(int));
	memset(&LCENTROID(0, 0),   0, ncentroids*dimension*sizeof(float));
	memset(&LPOPULATION(0, 0), 0, ncentroids*sizeof(int));

	/* Compute partial centroids. */
	#pragma omp parallel for
	for (int i = 0; i < lnpoints; i++)
	{
		int j;

		j = lmap[i]%NTHREADS;
		
		omp_set_lock(&lock[j]);
		
			vector_add(&LCENTROID(0, lmap[i]), &LPOINT(i));
			LPOPULATION(0, lmap[i])++;
		
		omp_unset_lock(&lock[j]);
	}

	n = ncentroids*dimension*sizeof(float);
	t[4] = k1_timer_get();
		memwrite(OFF_PCENTROIDS(rank, 0),
			&LCENTROID(0, 0),
			n
		);
	t[5] = k1_timer_get();
	time_network[1] += k1_timer_diff(t[4], t[5]);
	nwrite++; swrite += n;
		
	n = ncentroids*sizeof(int);
	t[4] = k1_timer_get();
		memwrite(OFF_PPOPULATION(rank, 0),
			&LPOPULATION(0, 0),
			n
		);
	t[5] = k1_timer_get();
	time_network[1] += k1_timer_diff(t[4], t[5]);
	nwrite++; swrite += n;
}

/*===================================================================*
 * compute_centroids()                                               *
 *===================================================================*/

/**
 * Compute centroids.
 */
static void compute_centroids(void)
{
	size_t n;

	barrier_wait(barrier);

	/* Compute centroids. */
	for (int i = 0; i < nclusters; i++)
	{
		if (i == rank)
			continue;

		n = lncentroids*dimension*sizeof(float);
		assert(n <= sizeof(lpcentroids));
		t[0] = k1_timer_get();
			memread(OFF_PCENTROIDS(i, rank*(ncentroids/nclusters)*dimension),
				&LPCENTROID(0),
				n
			);
		t[1] = k1_timer_get();
		time_network[0] += k1_timer_diff(t[0], t[1]);
		nread++; sread += n;

		n = lncentroids*sizeof(int);
		assert(n <= sizeof(lppopulation));
		t[0] = k1_timer_get();
			memread(OFF_PPOPULATION(i, rank*(ncentroids/nclusters)),
				&LPPOPULATION(0),
				n
			);
		t[1] = k1_timer_get();
		time_network[0] += k1_timer_diff(t[0], t[1]);
		nread++; sread += n;

		#pragma omp parallel for
		for (int j = 0; j < lncentroids; j++)
		{
			if (LPPOPULATION(j) < 1)
				continue;
			
			LPOPULATION(rank, j) += LPPOPULATION(j);
			vector_add(&LCENTROID(rank, j), &LPCENTROID(j));
		}
	}
		
	#pragma omp parallel for
	for (int j = 0; j < lncentroids; j++)
	{
		if (LPOPULATION(rank, j) > 1)
			vector_mult(&LCENTROID(rank, j), 1.0/LPOPULATION(rank, j));
		
		/* Cluster mean has changed. */
		if (!vector_equal(&CENTROID(rank, j), &LCENTROID(rank, j)))
		{
			lhas_changed[omp_get_thread_num()] = 1;
			vector_assign(&CENTROID(rank, j), &LCENTROID(rank, j));
		}
	}

	n = lncentroids*dimension*sizeof(float);
	t[4] = k1_timer_get();
		memwrite(OFF_CENTROIDS +
				rank*(ncentroids/nclusters)*dimension*sizeof(float),
			&CENTROID(rank, 0),
			n
		);
	t[5] = k1_timer_get();
	time_network[1] += k1_timer_diff(t[4], t[5]);
	nwrite++; swrite += n;
}

/*============================================================================*
 *                                 again()                                    *
 *============================================================================*/

/*
 * Asserts if another iteration is needed.
 */
static int again(void)
{
	size_t n;

	too_far[rank] = 0;
	has_changed[rank] = 0;

	for (int i = 0; i < NTHREADS; i++)
	{
		if (ltoo_far[i] && lhas_changed[i])
		{
			too_far[rank] = has_changed[rank] = 1;
			break;
		}
	}	

	n = sizeof(int);
	t[4] = k1_timer_get();
		memwrite(OFF_HAS_CHANGED(rank), &has_changed[rank], n);
		memwrite(OFF_TOO_FAR(rank),     &too_far[rank],     n);
	t[5] = k1_timer_get();
	time_network[1] += k1_timer_diff(t[4], t[5]);
	nwrite += 2; swrite += n + n;

	barrier_wait(barrier);

	n = nclusters*sizeof(int);
	t[0] = k1_timer_get();
		memread(OFF_HAS_CHANGED(0), &has_changed[0], nclusters*sizeof(int));
		memread(OFF_TOO_FAR(0),     &too_far[0],     nclusters*sizeof(int));
	t[1] = k1_timer_get();
	time_network[0] += k1_timer_diff(t[0], t[1]);
	nwrite += 2; swrite += n + n;
	
	/* Checks if another iteration is needed. */	
	for (int i = 0; i < nclusters; i++)
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
	omp_set_num_threads(NTHREADS);

	/* Initialize locks. */
	for (int i = 0; i < NTHREADS; i++)
		omp_init_lock(&lock[i]);
	
	/* Cluster data. */
	do
	{	
		populate();
		compute_pcentroids();
		compute_centroids();
	} while (again());
}

/*============================================================================*
 *                                  main()                                    *
 *============================================================================*/

/*
 * Clusters data.
 */
int main(int argc, char **argv)
{
	size_t n;
	int npoints;
	
	((void)argc);
	
	rank = atoi(argv[0]);
	nclusters = atoi(argv[1]);

	k1_timer_init();

	barrier = barrier_open(nclusters);

	t[2] = k1_timer_get();

		/* Read global data from remote memory. */
		n = 3*sizeof(int) + sizeof(float);
		t[0] = k1_timer_get();
			memread(OFF_MINDISTANCE, &mindistance, sizeof(float));
			memread(OFF_NPOINTS,     &npoints,     sizeof(int));
			memread(OFF_NCENTROIDS,  &ncentroids,  sizeof(int));
			memread(OFF_DIMENSION,   &dimension,   sizeof(int));
		t[1] = k1_timer_get();
		time_network[0] += k1_timer_diff(t[0], t[1]);
		nread += 4; sread += n;

		lnpoints = (rank < (nclusters - 1)) ?
			npoints/nclusters : (npoints/nclusters + npoints%nclusters);

		lncentroids = (rank < (nclusters - 1)) ?
			ncentroids/nclusters : (ncentroids/nclusters + ncentroids%nclusters);


		/* Read local data from remote memory. */
		n = lnpoints*dimension*sizeof(float);
		assert(n <= sizeof(lpoints));
		t[0] = k1_timer_get();
			memread(OFF_POINTS(rank*(npoints/nclusters), dimension),
				&lpoints[0],
				n
			);
		t[1] = k1_timer_get();
		time_network[0] += k1_timer_diff(t[0], t[1]);
		nread++; sread += n;

		n = lnpoints*sizeof(int);
		assert(n <= sizeof(lmap));
		t[0] = k1_timer_get();
			memread(OFF_MAP(rank*(npoints/nclusters)),
				&lmap[0],
				n	
			);
		t[1] = k1_timer_get();
		time_network[0] += k1_timer_diff(t[0], t[1]);
		nread++; sread += n;

		kmeans();
	
	t[3] = k1_timer_get();
	time_cpu = k1_timer_diff(t[2], t[3]) - (time_network[0] + time_network[1]);

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
