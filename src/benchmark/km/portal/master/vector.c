/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of CAP Benchmarks.
 * 
 * CAP Benchmarks is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any
 * later version.
 * 
 * CAP Benchmarks is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * CAP Benchmarks. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include "master.h"
	
/**
 * @brief Vector.
 */
struct vector
{
	int size;        /* Size.     */
	float *elements; /* Elements. */
};

/*
 * Returns the element [i] in a vector.
 */
#define VECTOR(v, i) \
	(((vector_t)(v))->elements[(i)])
	
/**
 * @brief Returns the size of a vector.
 *
 * @param v Target vector.
 */
int vector_size(struct vector *v)
{
	return (v->size);
}

/**
 * Returns the elements of a vector.
 */
float *vector_get(struct vector *v)
{
	return (v->elements);
}

/*
 * Creates a vector.
 */
struct vector *vector_create(int n)
{
	struct vector *v;
	
	v = smalloc(sizeof(struct vector));
	
	/* Initilize vector. */
	v->size = n;
	v->elements = scalloc(n, sizeof(float));

	return (v);
}

/*
 * @param Destroys a vector.
 *
 * @param v Target vector.
 */
void vector_destroy(struct vector *v)
{
	free(v->elements);
	free(v);
}

/*
 * Fills up vector with random numbers.
 */
struct vector *vector_random(struct vector *v)
{
	/* Fill vector. */
	for (int i = 0; i < vector_size(v); i++)
		VECTOR(v, i) = randnum() & 0xffff;

	return (v);
}

