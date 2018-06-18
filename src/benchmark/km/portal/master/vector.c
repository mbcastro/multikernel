/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

