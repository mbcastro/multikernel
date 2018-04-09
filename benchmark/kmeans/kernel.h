#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <nanvix/arch/mppa.h>

#define MAX_CENTROIDS (1024)
#define MAX_POINTS    (64*1024)
#define MAX_DIMENSION (16)

#define LENGTH_MAP         MAX_POINTS
#define LENGTH_POINTS      (MAX_POINTS*MAX_DIMENSION)
#define LENGTH_CENTROIDS   (MAX_CENTROIDS*MAX_DIMENSION)
#define LENGTH_HAS_CHANGED NR_CCLUSTER 
#define LENGTH_TOO_FAR     NR_CCLUSTER
#define LENGTH_POPULATION  MAX_CENTROIDS

#define SIZE_NCLUSTERS   (sizeof(int))
#define SIZE_MINDISTANCE (sizeof(float))
#define SIZE_DIMENSION   (sizeof(int))
#define SIZE_NPOINTS     (sizeof(int))
#define SIZE_NCENTROIDS  (sizeof(int))
#define SIZE_CENTROIDS   (LENGTH_CENTROIDS*sizeof(float))
#define SIZE_POPULATION  (LENGTH_POPULATION*sizeof(int))
#define SIZE_POINTS      (LENGTH_POINTS*sizeof(float))
#define SIZE_MAP         (LENGTH_MAP*sizeof(int))
#define SIZE_HAS_CHANGED (LENGTH_HAS_CHANGED*sizeof(int))
#define SIZE_TOO_FAR     (LENGTH_TOO_FAR*sizeof(int))
#define SIZE_PCENTROIDS  (NR_CCLUSTER*SIZE_CENTROIDS)
#define SIZE_PPOPULATION (NR_CCLUSTER*SIZE_POPULATION)

#define OFF_NCLUSTERS     0
#define OFF_MINDISTANCE   (OFF_NCLUSTERS + SIZE_NCLUSTERS)
#define OFF_POINTS(x,y)   ((OFF_MINDISTANCE + SIZE_MINDISTANCE) + (x)*(y)*sizeof(float))
#define OFF_NPOINTS       (OFF_POINTS(0, MAX_DIMENSION) + SIZE_POINTS)
#define OFF_CENTROIDS     (OFF_NPOINTS + SIZE_NPOINTS)
#define OFF_NCENTROIDS    (OFF_CENTROIDS + SIZE_CENTROIDS)
#define OFF_MAP(x)        ((OFF_NCENTROIDS + SIZE_NCENTROIDS) + (x)*sizeof(int))
#define OFF_HAS_CHANGED(x) ((OFF_MAP(0) + SIZE_MAP) + (x)*sizeof(int))
#define OFF_TOO_FAR(x)       ((OFF_HAS_CHANGED(0) + SIZE_HAS_CHANGED) + (x)*sizeof(int))
#define OFF_DIMENSION     (OFF_TOO_FAR(0) + SIZE_TOO_FAR)
#define OFF_PCENTROIDS(x,y)  ((OFF_DIMENSION + SIZE_DIMENSION) + ((x)*MAX_CENTROIDS*MAX_DIMENSION + (y))*sizeof(float))
#define OFF_PPOPULATION(x,y) ((OFF_PCENTROIDS(0, MAX_DIMENSION) + SIZE_PCENTROIDS) + ((x)*MAX_CENTROIDS + (y))*sizeof(int))

#endif /* _KERNEL_H_ */
