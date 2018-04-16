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

#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <stdlib.h>

/**
 * @brief Safe calloc().
 *
 * @param nmemb Number of elements.
 * @param size  Size of each element.
 */
void *scalloc(size_t nmemb, size_t size)
{
	void *p;
	
	assert((p = calloc(nmemb, size)) != NULL);
	
	return (p);
}

/**
 * @brief Safe malloc().
 *
 * @param size Number of bytes to allocate.
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

/**
 * @brief Initializes the random number generator.
 *
 * @param seed Seed value.
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
 * @brief Generates a random number.
 */
unsigned randnum(void)
{
	unsigned u;
	
	/* u in [0, 2^32) */
	randum_z = 36969 * (randum_z & 65535) + (randum_z >> 16);
	randum_w = 18000 * (randum_w & 65535) + (randum_w >> 16);
	u = (randum_z << 16) + randum_w;
	
	return u;
}

