/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * vector.c - Vector Library implementation.
 */

#include <stdlib.h>
#include "master.h"
	
/*
 * Vector.
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
	
/*
 * Returns the size of a vector.
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
 * Destroys a vector.
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

