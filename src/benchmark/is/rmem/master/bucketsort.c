#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <nanvix/mm.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "master.h"

/* Number of buckets. */
#define NUM_BUCKETS 256
#define NUM_IO_CORES 4

/*
 * Thread's data.
 */
//static struct tdata
//{
//	/* Thread's ID. */
//	pthread_t tid;
//
//	/* Arguments. */
//	struct
//	{
//		int i0;               /* Start bucket.        */
//		int in;               /* End bucket.          */
//		int j0;               /* Start array's index. */
//		struct bucket **done; /* Buckets.             */
//		int *array;           /* Array.               */
//	} args;
//} tdata[NUM_IO_CORES];
//
///*
// * Thread's main.
// */
//static void *thread_main(void *args)
//{
//	int j;           /* Bucket index.  */
//	struct tdata *t; /* Thread's data. */
//
//	t = args;
//
//	/* Rebuild array. */
//	j = t->args.j0;
//	for (int i = t->args.i0; i < t->args.in; i++)
//	{
//		bucket_merge(t->args.done[i], &t->args.array[j]);
//		j += bucket_size(t->args.done[i]);
//	}
//
//	pthread_exit(NULL);
//	return (NULL);
//}
//
///*
// * Rebuilds array.
// */
//static void rebuild_array(struct bucket **done, int *array)
//{
//	int j; /* array[] offset. */
//	int k; /* Loop index.     */
//
//#define BUCKETS_PER_CORE (NUM_BUCKETS/NUM_IO_CORES)
//
//	/* Spawn threads. */
//	j = 0;
//	for (int i = 0; i < NUM_IO_CORES; i++)
//	{
//		tdata[i].args.i0 = i*BUCKETS_PER_CORE;
//		tdata[i].args.in = (i + 1)*BUCKETS_PER_CORE;
//		tdata[i].args.done = done;
//		tdata[i].args.array = array;
//		pthread_create(&tdata[i].tid, NULL, thread_main, (void *)&tdata[i]);
//
//		for (k = i*BUCKETS_PER_CORE; k < (i + 1)*BUCKETS_PER_CORE; k++)
//			j += bucket_size(done[k]);
//	}
//
//	/* Join threads. */
//	for (int i = 0; i < NUM_IO_CORES; i++)
//		pthread_join(tdata[i].tid, NULL);
//}

/*
 * Bucket-sort algorithm.
 */
extern void bucketsort(int *array, int n)
{
	int max;                  /* Maximum number.      */
	int i, j;                 /* Loop indexes.        */
	//int offs;                 
	int cnum_buckets;                 
	int range;                /* Bucket range.        */
	struct minibucket *minib; /* Working mini-bucket. */
	//struct message *msg;      /* Working message.     */
	struct bucket **todo;     /* Todo buckets.        */
	struct bucket **done;     /* Done buckets.        */
	int barrier;
	long start, end;          /* Timers.              */
	k1_timer_init();

	/* Setup slaves. */
	//open_noc_connectors();

	todo = smalloc(NUM_BUCKETS*sizeof(struct bucket *));
	done = smalloc(NUM_BUCKETS*sizeof(struct bucket *));
	for (i = 0; i < NUM_BUCKETS; i++)
	{
		done[i] = bucket_create();
		todo[i] = bucket_create();
	}

	/* Find max
	 * number in the
	 * array. */
	start = k1_timer_get();
	max = INT_MIN;
	for (i = 0; i < n; i++)
	{
		/* Found. */
		if (array[i] > max)
			max = array[i];
	}

	/* Distribute
	 * numbers.
	 */
	range = max/NUM_BUCKETS;
	for (i = 0; i < n; i++)
	{
		j = array[i]/range;
		if (j >= NUM_BUCKETS)
			j = NUM_BUCKETS - 1;

		bucket_insert(&todo[j], array[i]);
	}
	end = k1_timer_get();
	master += k1_timer_diff(start, end);

	/* Sort
	 * buckets.
	 */
	j = 0;

	barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(barrier);
	//offs = 0;
	cnum_buckets = (NUM_BUCKETS/nclusters);
	//
	memwrite(OFF_MSG, &cnum_buckets, sizeof(int));
	for ( i = 0; i < NUM_BUCKETS; i++) {
		while(bucket_size(todo[i]) > 0) {
			minib = bucket_pop(todo[i]);
			memwrite(OFF_TODO(j*MINIBUCKET_SIZE*sizeof(int)), &minib->elements, sizeof(int)*MINIBUCKET_SIZE);
			j++;
		//	offs+=sizeof(struct bucket *)+cnum_buckets;
		}
	}

	//offs = 0;
	//for ( i = nclusters; i < 2*nclusters; i++) {
		//memread(OFF_DONE(i*cnum_buckets*sizeof(struct bucket *)), &done[j], sizeof(struct bucket *)*cnum_buckets);
//		memread(OFF_DONE(0), &done, sizeof(struct bucket *)*NUM_BUCKETS);
	//	offs+=sizeof(struct bucket *)+cnum_buckets;
	//}

	spawn_slaves();

	start = k1_timer_get();
	//rebuild_array(done, array);
	end = k1_timer_get();
	master += k1_timer_diff(start, end);

	/* House
	 * keeping.
	 */

	for (i = 0; i < NUM_BUCKETS; i++)
	{
		bucket_destroy(todo[i]);
		bucket_destroy(done[i]);
	}
	free(done);
	free(todo);
	join_slaves();

	barrier_close(barrier);
}
