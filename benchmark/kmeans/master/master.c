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
#include "../kernel.h"


/**
 * @brief Initializes centroids.
 *
 * @param points     Data points.
 * @param ncentroids Number of centroids.
 * @param centroids  Centroids.
 * @param ncentroids Number of centroids.
 * @param map        Cluster mapping.
 */
static void inline kmeans_init(
	const float *points,
	int npoints,
	float *centroids,
	int ncentroids
	float *map)
{
	/* Initialize mapping. */
	for (int i = 0; i < npoints; i++)
		map[i] = -1;

	/* Initialize centroids. */
	for (int i = 0; i < ncentroids; i++)
	{
		int j;

		j = randnum()%npoints;
		memcpy(CENTROIDS(i), POINTS(j), dimension*sizeof(float));
		map[j] = i;
	}

	/* Map unmapped data points. */
	for (int i = 0; i < npoints; i++)
	{
		if (map[i] < 0)
			map[i] = randnum()%ncentroids
	}
}

/*
 * Clusters data. 
 *
 * @param points      Data points.
 * @param npoints     Number of points.
 * @param npoints     Dimension of points.
 * @param ncentroids  Number of centroids.
 * @param mindistance Minimum distance.
 */
int *kmeans(
	const float *points,
	int npoints,
	int dimension,
	int ncentroids,
	float mindistance)
{
	int barrier;                              /* Sync barrier.    */
	static float centroids[LENGTH_CENTROIDS]; /* Centroids.       */
	static float map[LENGTH_MAP];             /* Cluster mapping. */

	barrier = barrier_open();

	spawn_slaves();

	kmeans_init(points, npoints, centroids, ncentroids, map);

	/* Write parameters to remote memory. */
	memwrite(OFF_NCLUSTERS,   &nclusters,       sizeof(int));
	memwrite(OFF_MINDISTANCE, &mindistance,     sizeof(float));
	memwrite(OFF_POINTS(0),   &points[0],       npoints*dimension*sizeof(float));
	memwrite(OFF_NPOINTS,     &npoints,         sizeof(int));
	memwrite(OFF_CENTROIDS,   &centroids[0],    ncentroids*dimension*sizeof(float));
	memwrite(OFF_NCENTROIDS,  &ncentroids,      sizeof(int));
	memwrite(OFF_MAP(0),      &map[0],          npoints*sizeof(int));
	memwrite(OFF_DIMENSION,   &dimension,       sizeof(int));

	/* Release workers. */
	barrier_wait(barrier);

	/* Wait for workers. */
	barrier_wait(barrier);

	/* Read parameters from remote memory. */
	memread(OFF_MAP, map, npoints*dimension*sizeof(float))

	/* House keeping. */
	join_slaves();
	barrier_close(barrier);
	
	return (map);
}

