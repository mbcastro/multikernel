#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <nanvix/arch/mppa.h>

#define LENGTH_MAP         MAX_POINTS
#define LENGTH_POINTS      MAX_POINTS*MAX_DIMENSION
#define LENGTH_CENTROIDS   MAX_CENTROIDS*MAX_DIMENSION
#define LENGTH_HAS_CHANGED NR_CCLUSTER 
#define LENGTH_TOO_FAR     NR_CCLUSTER

#define SIZE_NCLUSTERS   (sizeof(int))
#define SIZE_MINDISTANCE (sizeof(float))
#define SIZE_DIMENSION   (sizeof(int))
#define SIZE_NPOINTS     (sizeof(int))
#define SIZE_NCENTROIDS  (sizeof(int))
#define SIZE_CENTROIDS   (LENGTH_CENTROIDS*sizeof(float))
#define SIZE_POINTS      (LENGTH_POINTS*sizeof(float))
#define SIZE_MAP         (LENGTH_MAP*sizeof(int))
#define SIZE_HAS_CHANGED (LENGTH_HAS_CHANGED*sizeof(int))
#define SIZE_TOO_FAR     (LENGTH_TOO_FAR*sizeof(int))
#define SIZE_PCENTROIDS  (NR_CCLUSTER*SIZE_CENTROIDS)
#define SIZE_PPOPULATION (NR_CCLUSTER_LENGTH_CENTROIDS*sizeof(int))

#define OFF_NCLUSTERS     0
#define OFF_MINDISTANCE   (OFF_NCLUSTERS + SIZE_NCLUSTERS)
#define OFF_POINTS(x,y)   ((OFF_MINDISTANCE + SIZE_MINDISTANCE) + (x)*(y)*sizeof(float))
#define OFF_POINTS(x)     (OFF_POINTS(x, MAX_DIMENSION))
#define OFF_NPOINTS       (OFF_POINTS(0) + SIZE_POINTS)
#define OFF_CENTROIDS     (OFF_NPOINTS + SIZE_NPOINTS)
#define OFF_NCENTROIDS    (OFF_CENTROIDS + SIZE_CENTROIDS)
#define OFF_MAP(x)        ((OFF_NCENTROIDS + SIZE_NCENTROIDS) + (x)*sizeof(int))
#define OFF_HAS_CHANGED   (OFF_MAP(0) + SIZE_MAP)
#define OFF_TOO_FAR       (OFF_HAS_CHANGED + SIZE_HAS_CHANGED)
#define OFF_DIMENSION     (OFF_TOO_FAR + SIZE_TOO_FAR)
#define OFF_PCENTROIDS(x)  ((OFF_DIMENSION + SIZE_DIMENSION) + (x)*(y)*sizeof(float))
#define OFF_PPOPULATION(x) ((OFF_PCENTROIDS(0) + SIZE_PCENTROIDS) + (x)*(y)*sizeof(float))

#endif /* _KERNEL_H_ */
