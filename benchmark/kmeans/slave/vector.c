/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * vector.c - Vector library.
 */

#include <math.h>
#include "slave.h"

/*===================================================================*
 * vector_distance()                                                 *
 *===================================================================*/

/*
 * Calculates the distance between two points.
 */
float vector_distance(float *a, float *b)
{
	float distance;
	
	distance = 0;
	
	/* Computes the euclidean distance. */
	for (int i = 0; i < dimension; i++)
		distance += pow(a[i] - b[i], 2);
	distance = sqrt(distance);
	
	return (distance);
}

/*===================================================================*
 * vector_add()                                                      *
 *===================================================================*/

/*
 * Adds two vectors.
 */
float *vector_add(float *v1, const float *v2)
{
	for (int i = 0; i < dimension; i++)
		v1[i] += v2[i];
	
	return (v1);
}

/*===================================================================*
 * vector_mult()                                                     *
 *===================================================================*/

/*
 * Multiplies a vector by a scalar.
 */
float *vector_mult(float *v, float scalar)
{
	for (int i = 0; i < dimension; i++)
		v[i] *= scalar;
	
	return (v);
}

/*===================================================================*
 * vector_assign()                                                   *
 *===================================================================*/

/*
 * Assigns a vector to another.
 */
float *vector_assign(float *v1, const float *v2)
{
	for (int i = 0; i < dimension; i++)
		v1[i] = v2[i];
	
	return (v1);
}

/*===================================================================*
 * vector_equal()                                                    *
 *===================================================================*/

/*
 * Tests if two vectors are equal.
 */
int vector_equal(const float *v1, const float *v2)
{
	for (int i = 0; i < dimension; i++)
	{
		if (fabs(v1[i] - v2[i]) < 0.00001)
			return (1);
	}
	
	return (0);
}
