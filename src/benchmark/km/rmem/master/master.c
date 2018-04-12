/*
* Copyright(C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
* 
* master.c - Master process.
*/

#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "master.h"
#include "../../kernel.h"


/**
 * @brief Initializes centroids.
 *
 * @param points     Data points.
 * @param ncentroids Number of centroids.
 * @param centroids  Centroids.
 * @param ncentroids Number of centroids.
 * @param map        Cluster mapping.
 */
static inline void kmeans_init(
	const float *points,
	int npoints,
	int dimension,
	float *centroids,
	int ncentroids,
	int *map)
{
	/* Initialize mapping. */
	for (int i = 0; i < npoints; i++)
		map[i] = -1;

	/* Initialize centroids. */
	for (int i = 0; i < ncentroids; i++)
	{
		int j;

		j = randnum()%npoints;
		memcpy(&centroids[i*dimension], &points[j*dimension], dimension*sizeof(float));
		map[j] = i;
	}

	/* Map unmapped data points. */
	for (int i = 0; i < npoints; i++)
	{
		if (map[i] < 0)
			map[i] = randnum()%ncentroids;
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
void kmeans(
	const float *points,
	int npoints,
	int dimension,
	int ncentroids,
	float mindistance)
{
	int barrier;
	static float centroids[LENGTH_CENTROIDS]; /* Centroids.       */
	static int  map[LENGTH_MAP];              /* Cluster mapping. */

	kmeans_init(points, npoints, dimension, centroids, ncentroids, map);

	barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(barrier);

	/* Write parameters to remote memory. */
	memwrite(OFF_NCLUSTERS,            &nclusters,       sizeof(int));
	memwrite(OFF_MINDISTANCE,          &mindistance,     sizeof(float));
	memwrite(OFF_POINTS(0, dimension), &points[0],       npoints*dimension*sizeof(float));
	memwrite(OFF_NPOINTS,              &npoints,         sizeof(int));
	memwrite(OFF_CENTROIDS,            &centroids[0],    ncentroids*dimension*sizeof(float));
	memwrite(OFF_NCENTROIDS,           &ncentroids,      sizeof(int));
	memwrite(OFF_MAP(0),               &map[0],          npoints*sizeof(int));
	memwrite(OFF_DIMENSION,            &dimension,       sizeof(int));
	memwrite(OFF_DIMENSION,            &dimension,       sizeof(int));
	memwrite(OFF_DIMENSION,            &dimension,       sizeof(int));

	spawn_slaves();

	join_slaves();

	/* Read parameters from remote memory. */
	memread(OFF_MAP(0), map, npoints*dimension*sizeof(int));

	barrier_close(barrier);
}

