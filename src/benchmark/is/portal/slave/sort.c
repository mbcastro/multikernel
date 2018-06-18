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

#include <stdio.h>
#include <stdlib.h>
#include <nanvix/arch/mppa.h>

/*
 *  * Exchange two numbers.
 *   */
#define exch(a, b, t) \
{ (t) = (a); (a) = (b); (b) = (t); }

/*
 *  * Compare and exchange two numbers.
 *   */
#define compexgh(a, b, t)    \
	if ((b) < (a))           \
exch((b), (a), (t)); \

#define M 10

/*
 *  * Quicksort partition.
 *   */
static int partition(int *a, int l, int r)
{
	int v;    /* Partitioning element. */
	int t;    /* Temporary element.    */
	int i, j; /* Loop index.           */

	i = l - 1;
	j = r;
	v = a[r];

	while (1)
	{
		while (a[++i] < v)
			/* NOOP.*/ ;

		while (a[--j] > v)
		{
			if (j == l)
				break;
		}

		if (i >= j)
			break;

		exch(a[i], a[j], t);
	}

	exch(a[i], a[r], t);

	return (i);
}

/*
 *  * Quicksort.
 *   */
static void quicksort(int *a, int l, int r)
{
	int i; /* Pivot.             */
	int t; /* Temporary element. */

	/* Fine grain stop. */
	if ((r - l) <= M)
		return;

	/* Avoid anomalous partition. */
	exch(a[(l + r) >> 1], a[r - 1], t);

	/* Median of three. */
	compexgh(a[l], a[r - 1], t);
	compexgh(a[l], a[r], t);
	compexgh(a[r - 1], a[r], t);

	/* Sort. */
	i = partition(a, l + 1 , r - 1);
	quicksort(a, l, i - 1);
	quicksort(a, i + 1, r);
}

/*
 *  * Insertion sort.
 *   */
static void insertion(int *a, int l, int r)
{
	int t;    /* Temporary value. */
	int v;    /* Working element. */
	int i, j; /* Loop indexes.    */

	for (i = r; i > l; i--)
		compexgh(a[i - 1], a[i], t);

	for (i = l + 2; i <= r; i++)
	{
		j = i;
		v = a[i];

		while (v < a[j - 1])
		{
			a[j] = a[j - 1];
			j--;
		}

		a[j] = v;
	}
}

/*
 *  * Sorts an array of numbers.
 *   */
void _sort(int *a, int n)
{
	quicksort(a, 0, n - 1);
	insertion(a, 0, n - 1);
}

/*
 *  * Merges two arrays.
 *   */
void merge(int *a, int *b, int size)
{
	int tmp; /* Temporary value. */
	int i, j;/* Loop indexes.    */


	/* Merge. */
//	int *tmp;
//	tmp = malloc(sizeof(int)*size) ;
	i = 0; j = 0;
//	for ( i = 0; i < size; i++) {
//		if (j < size && k < size) {
//			if (a[j] < b[k]) {
//				tmp[i] = a[j];
//				j++;
//			} else {
//				tmp[i] = b[k];
//				k++;
//			}
//			i++;
//		} else if (j == size) {
//			for (; i < size;) {
//				tmp[i] = b[k];
//				k++;
//				i++;
//			}
//		} else {
//			for (; i < size;) {
//				tmp[i] = a[j];
//				j++;
//				i++;
//			}
//		}
//	}
//	for (i = 0; i < size; i++) {
//		a[i] = tmp[i];
//	}
	while ((i < size) && (j < size))
	{
		if (b[j] < a[i])
		{
			exch(b[j], a[i], tmp);
				j++;
		}
		else
		{
			i++;
		}
	}
}

/*
 *  * Mergesort algorithm.
 *   */
void sort2power(int rank, int *array, int size, int chunksize)
{
	int i; /* Loop index.         */
	int N; /* Working array size. */

	/* Sort. */
	//if (rank == 0)
	//printf("TOTAL: %d, %d, %d\n", rank, size, chunksize);
#pragma omp parallel for private(i) default(shared)
	for (i = 0; i < size; i += chunksize)
		_sort(&array[i], (i + chunksize < size) ? chunksize : size - i);

	if (rank == 0) {
//		printf("CHUNK: %d\n", chunksize);
//		for (i = 0; i < chunksize; i+=500) {
//			printf("ARRAYSORT:%d\n", array[i]);
//		}
//		printf("CHUNK: %d\n", 2*chunksize);
//		for (i = chunksize; i < 2*chunksize; i+=500) {
//			printf("ARRAYSORT:%d\n", array[i]);
//		}
	}
	/* Merge. */
	for (N = chunksize; N < size; N += N)
	{
#pragma omp parallel for private(i) default(shared)
		for (i = 0; i < size; i += N)
		{
			//if (rank == 0)
			//printf("RELATIVE: %d, %d, %d, %d \n",rank, i, N, i+N);
			/* TODO: allow non-multiple of 2. */
			if (i+N < size) 
				merge(&array[i], &array[i+N], N);
		}

//			if (rank == 0 && N == N) {
//				printf("CHUNK: %d\n", N);
//				for (i = 0; i < size; i+=500)
//					printf("ARRAY:%d\n", array[i]);
//			}
	}
}

