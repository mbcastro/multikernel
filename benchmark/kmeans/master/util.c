/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * util.c - Utility library implementation.
 */

#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <stdlib.h>

/*
 * Safe calloc().
 */
void *scalloc(size_t nmemb, size_t size)
{
	void *p;
	
	assert((p = calloc(nmemb, size)) != NULL);
	
	return (p);
}

/*
 * Safe malloc().
 */
void *smalloc(size_t size)
{
	void *p;
	
	assert((p = malloc(size)) != NULL);
	
	return (p);
}

#define RANDNUM_W 521288629;
#define RANDNUM_Z 362436069;

static unsigned randum_w = RANDNUM_W;
static unsigned randum_z = RANDNUM_Z;

/*
 * Initializes the random number generator.
 */
void srandnum(int seed)
{
	unsigned w, z;

	w = (seed * 104623) & 0xffffffff;
	randum_w = (w) ? w : RANDNUM_W;
	z = (seed * 48947) & 0xffffffff;
	randum_z = (z) ? z : RANDNUM_Z;
}

/*
 * Generates a random number.
 */
unsigned randnum(void)
{
	unsigned u;
	
	/* 0 <= u < 2^32 */
	randum_z = 36969 * (randum_z & 65535) + (randum_z >> 16);
	randum_w = 18000 * (randum_w & 65535) + (randum_w >> 16);
	u = (randum_z << 16) + randum_w;
	
	return u;
}

