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

#ifndef SLAVE_H_
#define SLAVE_H_
#include "../../kernel.h"
#define CLUSTER_WORKLOAD 0xfffff /* 1 MB */


/*============================================================================*
 *                             Mini-Buckets Library
 *============================================================================*/


/*
 * Mini-bucket.
 */
struct minibucket
{
	int size;                      /* Current size.               */
	int elements[MINIBUCKET_SIZE]; /* Elements.                   */
	struct minibucket *next;       /* Next mini-bucket in a list. */
};

/*
 * Creates a mini-bucket.
 */
extern struct minibucket *minibucket_create(void);

/*
 * Destroys a mini bucket.
 */
extern void minibucket_destroy(struct minibucket *minib);

/*
 * Asserts if a mini-bucket is empty.
 */
#define minibucket_empty(minib) \
	((minib)->size == 0)

/*
 * Asserts if a bucket is full.
 */
#define minibucket_full(minib) \
	((minib)->size == MINIBUCKET_SIZE)

/*
 * Pushes an item onto a mini-bucket.
 */
#define minibucket_push(minib, x) \
	((minib)->elements[(minib)->size++] = (x))

/*
 * Pops an item from a mini-bucket.
 */
#define minibucket_pop(minib, x) \
	((x) = (minib)->elements[--(minib)->size])

/*
 * Returns the top element in a bucket.
 */
#define minibucket_top(minib) \
	((minib)->elements[(minib)->size - 1])

/*============================================================================*
 *                                Buckets Library
 *============================================================================*/

/*
 * Bucket.
 */
struct bucket
{
	int size;                /* Number of elements.  */
	struct minibucket *head; /* List of mini-buckets.*/
};

/*
 * Creates a bucket.
 */
extern struct bucket *bucket_create(void);

/*
 * Destroys a bucket.
 */
extern void bucket_destroy(struct bucket *b);

/*
 * Merges a bucket.
 */
extern void bucket_merge(struct bucket *b, int *array);

/*
 * Pops a mini-bucket from a bucket.
 */
extern struct minibucket *bucket_pop(struct bucket *b);

/*
 * Pushes a mini-bucket onto a bucket.
 */
extern void bucket_push(struct bucket *b, struct minibucket *minib);

/*
 * Insert an item into a bucket.
 */
extern void bucket_insert(struct bucket **b, int x);

/*
 * Returns the size of a bucket.
 */
#define bucket_size(b) \
	((b)->size)


/*===============================================================*
 * IPC                                                           *
 *===============================================================*/

/* Forward definitions. */
extern void open_noc_connectors(void);
extern void close_noc_connectors(void);
extern void data_receive(int, void *, size_t);
extern void data_send(int, void *, size_t);
extern void sync_master(void);

/* Forward definitions. */
extern int rank;
extern int infd;
extern int outfd;

extern long total;
#endif /* SLAVE_H_ */
