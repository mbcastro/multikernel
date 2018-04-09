/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * vector.c - Vector Library implementation.
 */

#include <stdlib.h>

/*
 * Fills up vector with random numbers.
 */
float *vector_random(float *v, int dimension)
{
	/* Fill vector. */
	for (int i = 0; i < dimension; i++)
		v[i] = randnum() & 0xffff;

	return (v);
}

