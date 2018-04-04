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
#include "slave.h"
#include "../kernel.h"

#define NTHREADS 16

#define DELTA(NR_CCLUSTER - 1)

/* Global K-means Data */
static int   nclusters;
static float mindistance;
static float centroids[LENGTH_CENTROIDS];
static float population[LENGTH_POPULATION];
static int   ncentroids;
static int   has_changed[LENGTH_HAS_CHANGED];
static int   too_far[LENGTH_TOO_FAR];
static int   dimension;

/* Local K-means Data */
static int   lnpoints;
static int   ltoo_far[NTHREADS];
static int   lhas_changed[NTHREADS];
static int   lmap[MAX_POINTS/NR_CCLUSTER + DELTA];
static float lpoints[(MAX_POINTS/NR_CCLUSTER + DELTA)*MAX_DIMENSION];
static float lcentroids[(MAX_CENTROIDS/NR_CCLUSTER + DELTA)*MAX_DIMENSION];
static float lpopulation[MAX_CENTROIDS];
static float lppopulation[MAX_CENTROIDS/NR_CCLUSTER + DELTA];

/**
 * @brief Helper macros for array indexing.
 */
/**@{*/
#define LPOINT(x)        lpoints[(x)*dimension]
#define CENTROID(x)      centroids[(x)*dimension]
#define CENTROID(x,y)    centroids[((x)*(ncentroids/nclusters) + (y))*dimension]
#define LCENTROID(x)     lcentroids[(x)*dimension]
#define LPCENTROID(x)    lpcentroids[(x)*dimension]
#define LPOPULATION(x)   lpopulation[(x)]
#define LPOPULATION(x,y) lpopulation[(x)*(ncentroids/nclusters) + (y)]
#define LPPOPULATION(x)  lppopulation[(x)]
/**@}*/

/* Thread communication. */
static omp_lock_t lock[NTHREADS];

/* Timing statistics. */
long total = 0;

/*===================================================================*
 * populate()                                                        *
 *===================================================================*/

/*
 * Populates clusters.
 */
static void populate(void)
{
	long t[2];
	
	t[0] = k1_timer_get();

		memread(OFF_CENTROIDS, &centroids[0], ncentroids*dimension*sizeof(float));

		memset(ltoo_far, 0, NTHREADS*sizeof(int));
		
		/* Iterate over data points. */
		#pragma omp parallel for
		for (int i = 0; i < lnpoints; i++)
		{
			int tid;
			float distance;

			tid = omp_get_thread_num();

			distance = vector_distance(CENTROID(lmap[i]), LPOINT(i));
			
			/* Look for closest cluster. */
			for (int j = 0; j < ncentroids; j++)
			{
				float tmp;

				/* Point is in this cluster. */
				if (j == lmap[i])
					continue;
					
				tmp = vector_distance(CENTROID(j), LPOINT(i));
				
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

	t[1] = k1_timer_get();
	total += k1_timer_diff(t[0], t[1]);
}

/*====================================================================*
 * compute_pcentroids()                                               *
 *====================================================================*/

/**
 * @brief Computes partial centroids.
 */
static inline void compute_pcentroids(void)
{
	long t[2];

	t[0] = k1_timer_get();

		memset(&lhas_changed,  0, NTHREADS*sizeof(int));
		memset(&LCENTROID(0),  0, ncentroids*dimension*sizeof(float));
		memset(&LPOPULATION(0), 0, ncentroids*sizeof(int));

		/* Compute partial centroids. */
		#pragma omp parallel for
		for (int i = 0; i < lnpoints; i++)
		{
			int j;

			j = lmap[i]%NTHREADS;
			
			omp_set_lock(&lock[j]);
			
				vector_add(LCENTROID(i), LPOINT(i));
				LPOPULATION(lmap[i])++;
			
			omp_unset_lock(&lock[j]);
		}
	
		memwrite(OFF_PCENTROIDS(rank*ncentroids, dimension),
			&LCENTROID(0),
			ncentroids*dimension*sizeof(float)
		);
		
		memwrite(OFF_PPOPULATION(rank*ncentroids, dimension),
			&LPOPULATION(0),
			ncentroids*sizeof(int)
		);
	
	t[1] = k1_timer_get();
	total += k1_timer_diff(t[0], t[1]);
}

/*===================================================================*
 * compute_centroids()                                               *
 *===================================================================*/

/**
 * Compute centroids.
 */
static void compute_centroids(void)
{
	long t[2];

	t[0] = k1_timer_get();

		/* Compute centroids. */
		for (int i = 0; i < nclusters; i++)
		{
			if (i == rank)
				continue;

			memread(OFF_PCENTROIDS(i*ncentroids + rank*(ncentroids/nclusters), dimension),
				&LPCENTROID(0),
				lncentroids*dimension*sizeof(float)
			);

			memread(OFF_PPOPULATION(i*ncentroids + rank*(ncentroids/nclusters), dimension),
				&LPPOPULATION(0),
				lncentroids*sizeof(int)
			);
			
			#pragma omp parallel for private(population)
			for (int j = 0; j < lncentroids; j++)
			{
				if (LPOPULATION(j) == 0)
					continue;
				
				LPOPULATION(rank, j) += LPPOPULATION(j);
				vector_add(LCENTROID(j), LPCENTROID(j));
			}
		}
			
		#pragma omp parallel for private(population)
		for (int j = 0; j < lncentroids; j++)
		{
			if (LPOPULATION(rank, j) > 1)
				vector_mult(LCENTROID(j), 1.0/LPOPULATION(rank, j));
			
			/* Cluster mean has changed. */
			if (!vector_equal(CENTROID(rank, j), LCENTROID(j)))
			{
				lhas_changed[omp_get_thread_num()] = 1;
				vector_assign(CENTROID(rank, j), LCENTROID(j));
			}
		}
	
	t[1] = k1_timer_get();
	total += k1_timer_diff(t[0], t[1]);
}

/*============================================================================*
 *                                 again()                                    *
 *============================================================================*/

/*
 * Asserts if another iteration is needed.
 */
static int again(void)
{
	t[0] = k1_timer_get();

		too_far[rank] = 0;
		has_changed[rank] = 0;

		for (int i = 0; i < NTHREADS; i++)
		{
			too_far[rank] |= ltoo_far[i];
			has_changed[rank] |= lhas_changed[i];
		}	

		memwrite(OFF_HAS_CHANGED, &has_changed[rank], *sizeof(int));
		memwrite(OFF_TOO_FAR,     &too_far[rank],     sizeof(int));

		barrier_wait(barrier_workers);

		memread(OFF_HAS_CHANGED, &has_changed[0], nclusters*sizeof(int));
		memread(OFF_TOO_FAR,     &too_far[0],     nclusters*sizeof(int));
		
		/* Checks if another iteration is needed. */	
		for (int i = 0; i < nclusters; i++)
		{
			if (has_changed[i] && too_far[i])
			{
				t[1] = k1_timer_get();
				total += k1_timer_diff(t[0], t[1]);
				return (1);
			}
		}
	
	t[1] = k1_timer_get();
	total += k1_timer_diff(t[0], t[1]);
			
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
		barrier_wait(barrier_workers);
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
	int npoints;
	int barrier_master;
	int barrier_workers
	
	((void)argc);
	
	rank = atoi(argv[0]);

	barrier_master = barrier_open();
	barrier_workers = barrier_open();

	barrier_wait(barrier_master);

	/* Read global data from remote memory. */
	memread(OFF_NCLUSTERS,   &nclusters,   sizeof(int));
	memread(OFF_MINDISTANCE, &mindistance, sizeof(float));
	memread(OFF_NPOINTS,     &npoints,     sizeof(int));
	memread(OFF_NCENTROIDS,  &ncentroids,  sizeof(int));
	memread(OFF_DIMENSION,   &dimension,   sizeof(int));

	lnpoints = (rank < (nclusters - 1)) ?
		npoints/nclusters : (npoints/nclusters + npoints%nclusters);

	lncentroids = (rank < (nclusters - 1)) ?
		ncentroids/nclusters : (ncentroids/nclusters + ncentroids%nclusters);

	/* Read local data from remote memory. */
	memread(OFF_POINTS(rank*(npoints/nclusters), dimension),
		&lpoints[0],
		lnpoints*dimension*sizeof(float)
	);
	
	memread(OFF_MAP(rank*(npoints/nclusters)),
		&lmap[0],
		lnpoints*sizeof(int)
	);
	
	kmeans();

	memwrite(OFF_MAP(rank*(npoints/nclusters)), &lmap[0], lnpoints*sizeof(int));

	barrier_wait(barrier_master);

	return (0);
}
