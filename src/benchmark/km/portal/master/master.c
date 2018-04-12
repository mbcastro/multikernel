/*
* Copyright(C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
* 
* master.c - Master process.
*/

#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "master.h"

#define NUM_THREADS 1

/* Size of arrays. */
#define CENTROIDS_SIZE ((ncentroids*dimension)+nclusters*(nclusters-1)*dimension)
#define POPULATION_SIZE (ncentroids + nclusters*(nclusters-1))
#define PCENTROIDS_SIZE (nclusters*ncentroids*dimension)
#define PPOPULATION_SIZE (nclusters*ncentroids)

/* K-means. */
static float mindistance; /* Minimum distance.          */
static int ncentroids;    /* Number of centroids.       */
static int npoints;       /* Number of data points.     */
static int dimension;     /* Dimension of data points.  */
static int *map;          /* Map of clusters.           */
static vector_t *data;    /* Data points.               */
static float *centroids;  /* Data centroids.            */
static int *population;   /* Population of centroids.   */
static float *pcentroids; /* Partial centroids.         */
static int *ppopulation;  /* Partial population.        */
static int *has_changed;  /* Has any centroid changed?  */
static int *too_far;      /* Are points too far?        */
static int *lnpoints;     /* Local number of points.    */
static int *lncentroids;  /* Local number of centroids. */

/*===================================================================*
 * sendwork()                                                        *
 *===================================================================*/

/*
 * Returns the ith centroid.
 */
#define CENTROID(i) \
	(&centroids[(i)*dimension])

/*
 * Sends work to slave processes.
 */
static void sendwork(void)
{
	ssize_t n; /* Bytes to send/receive. */

	/* Distribute work among slave processes. */
	for (int i = 0; i < nclusters; i++)
	{
		lnpoints[i] = ((i + 1) < nclusters) ? 
			npoints/nclusters :  npoints - i*(npoints/nclusters);
			
		lncentroids[i] = ((i + 1) < nclusters) ? 
			ncentroids/nclusters : ncentroids - i*(ncentroids/nclusters);
	}

	/* Send work to slave processes. */
	for (int i = 0; i < nclusters; i++)
	{
		data_send(outfd[i], &lnpoints[i], sizeof(int));
		data_send(outfd[i], &nclusters, sizeof(int));
		data_send(outfd[i], &ncentroids, sizeof(int));
		data_send(outfd[i], &mindistance, sizeof(float));
		data_send(outfd[i], &dimension, sizeof(int));
		n = nclusters*sizeof(int);
		data_send(outfd[i], lncentroids, n);

		n = dimension*sizeof(float);
		for (int j = 0; j < lnpoints[i]; j++)
			data_send(outfd[i], vector_get(data[i*(npoints/nclusters)+j]), n);
		
		n = ncentroids*dimension*sizeof(float);
		data_send(outfd[i], centroids, n);
		
		n = lnpoints[i]*sizeof(int);
		data_send(outfd[i], &map[i*(npoints/nclusters)], n);
	}
}

/*===================================================================*
 * sync_pcentroids()                                                 *
 *===================================================================*/

#define PCENTROID(i, j) \
(&pcentroids[((i)*ncentroids + (j))*dimension])

#define PPOPULATION(i, j) \
(&ppopulation[(i)*ncentroids + (j)])

/*
 * Synchronizes partial centroids.
 */
static void sync_pcentroids(void)
{
	ssize_t n;           /* Bytes to send/receive. */
	long start, end; /* Timer.                 */
	
	/* Receive partial centroids. */
	n = ncentroids*dimension*sizeof(float);
	for (int i = 0; i < nclusters; i++)
		data_receive(infd, i, PCENTROID(i, 0), n);

	/* 
	 * Send partial centroids to the
	 * slave process that is assigned to it.
	 */
	for (int i = 0; i < nclusters; i++)
	{
		/* Build partial centroid. */
		start = k1_timer_get();
		n = lncentroids[i]*dimension*sizeof(float);
		for (int j = 0; j < nclusters; j++)
		{
			memcpy(CENTROID(j*lncentroids[i]), 
									PCENTROID(j, i*(ncentroids/nclusters)), n);
		}
		end = k1_timer_get();
		master += k1_timer_diff(start, end);

		n = nclusters*lncentroids[i]*dimension*sizeof(float);
		data_send(outfd[i], centroids, n);
	}
}

/*===================================================================*
 * sync_population()                                                 *
 *===================================================================*/

/*
 * Synchronizes partial population.
 */
static void sync_ppopulation(void)
{
	ssize_t n;           /* Bytes to send/receive. */
	long start, end; /* Timer.                 */

	/* Receive temporary population. */
	n = ncentroids*sizeof(int);
	for (int i = 0; i < nclusters; i++)
		data_receive(infd, i, PPOPULATION(i, 0), n);

	/* 
	 * Send partial population to the
	 * slave process that assigned to it.
	 */
	for (int i = 0; i < nclusters; i++)
	{
		/* Build partial population. */
		start = k1_timer_get();
		n = lncentroids[i]*sizeof(int);
		for (int j = 0; j < nclusters; j++)
		{
			memcpy(&population[j*lncentroids[i]], 
								  PPOPULATION(j, i*(ncentroids/nclusters)), n);
		}
		end = k1_timer_get();
		master += k1_timer_diff(start, end);

		n = nclusters*lncentroids[i]*sizeof(int);
		data_send(outfd[i], population, n);
	}
}

/*===================================================================*
 * sync_centroids()                                                  *
 *===================================================================*/

/*
* Synchronizes centroids.
*/
static void sync_centroids(void)
{
	ssize_t n;

	/* Receive centroids. */
	for (int i = 0; i < nclusters; i++)
	{
		n = lncentroids[i]*dimension*sizeof(float);
		
		data_receive(infd, i, CENTROID(i*(ncentroids/nclusters)), n);
	}

	/* Broadcast centroids. */
	n = ncentroids*dimension*sizeof(float);
	for (int i = 0; i < nclusters; i++)
		data_send(outfd[i], centroids, n);
}

/*===================================================================*
 * sync_status()                                                     *
 *===================================================================*/

/*
* Synchronizes slaves' status.
*/
static void sync_status(void)
{
	ssize_t n;

	/* Receive data. */
	n = NUM_THREADS*sizeof(int);
	for (int i = 0; i < nclusters; i++)
	{
		data_receive(infd, i, &has_changed[i*NUM_THREADS], n);
		data_receive(infd, i, &too_far[i*NUM_THREADS], n);
	}

	/* Broadcast data to slaves. */
	n = nclusters*NUM_THREADS*sizeof(int);
	for (int i = 0; i < nclusters; i++)
	{
		data_send(outfd[i], has_changed, n);
		data_send(outfd[i], too_far, n);
	}
}

/*===================================================================*
 * again()                                                           *
 *===================================================================*/

/*
 * Asserts if another iteration is needed.
 */
static int again(void)
{
	long start, end;
	
	start = k1_timer_get();
		for (int i = 0; i < nclusters*NUM_THREADS; i++)
		{
			if (has_changed[i] && too_far[i])
			{
				end = k1_timer_get();
				master += k1_timer_diff(start, end);
				return (1);
			}
		}
	end = k1_timer_get();
	master += k1_timer_diff(start, end);
	
	return (0);
}

/*===================================================================*
 * kmeans()                                                          *
 *===================================================================*/

/*
 * Internal kmeans().
 */
static void _kmeans(void)
{
	int it = 0;
	long start, end;
	
	/* Create auxiliary structures. */
	map = scalloc(npoints, sizeof(int));
	too_far = smalloc(nclusters*NUM_THREADS*sizeof(int));
	has_changed = smalloc(nclusters*NUM_THREADS*sizeof(int));
	centroids = smalloc(CENTROIDS_SIZE*sizeof(float));
	population = smalloc(POPULATION_SIZE*sizeof(int));
	pcentroids = smalloc(PCENTROIDS_SIZE*sizeof(float));
	ppopulation = smalloc(PPOPULATION_SIZE*sizeof(int));
	lnpoints = smalloc(nclusters*sizeof(int));
	lncentroids = smalloc(nclusters*sizeof(int));
	
	start = k1_timer_get();
	/* Initialize mapping. */
	for (int i = 0; i < npoints; i++)
		map[i] = -1;

	/* Initialize centroids. */
	for (int i = 0; i < ncentroids; i++)
	{
		int j;

		j = randnum()%npoints;
		memcpy(CENTROID(i), vector_get(data[j]), dimension*sizeof(float));
		map[j] = i;
	}
	
	/* Map unmapped data points. */
	for (int i = 0; i < npoints; i++)
	{
		if (map[i] < 0)
			map[i] = randnum()%ncentroids;
	}
	end = k1_timer_get();
	master += k1_timer_diff(start, end);
	
	sendwork();
	
	do
	{
		sync_pcentroids();
		sync_ppopulation();
		sync_centroids();
		sync_status();
		printf("kmeans it: %d\n", ++it);
	} while (again());
	
	/* House keeping. */
	free(lncentroids);
	free(lnpoints);
	free(ppopulation);
	free(pcentroids);
	free(population);
	free(centroids);
	free(has_changed);
	free(too_far);
}

/*
 * Clusters data. 
 */
int *kmeans(vector_t *_data, int _npoints, int _ncentroids, float _mindistance)
{
	/* Setup parameters. */
	data = _data;
	npoints = _npoints;
	ncentroids = _ncentroids;
	mindistance = _mindistance;
	dimension = vector_size(data[0]);
	
	open_noc_connectors();
	spawn_slaves();

	_kmeans();

	join_slaves();
	close_noc_connectors();
	
	return (map);
}
