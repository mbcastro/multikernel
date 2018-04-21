/*
 * Copyright (C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * mppa/slave/main.c - Slave main().
 */

#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <nanvix/mm.h>
#include <assert.h>
#include <limits.h>
#include <omp.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include "util.c"
#include "message.c"

static int barrier;

/* Timing statistics. */
long start;
long end;

/*
 * Array block.
 */
struct
{
    int size;                                   /* Size of block. */
    int elements[CLUSTER_WORKLOAD/sizeof(int)]; /* Elements.      */
} block;

/*
 * Sorts an array of numbers.
 */
extern void sort2power(int *array, int size, int chunksize);

/*
 * Obeys master.
 */
int main(int argc, char **argv)
{
    //int i;               /* Loop index. */
    //int id;              /* Bucket ID.  */
    //struct message *msg; /* Message.    */
    //int *minib; /* Working mini-bucket. */
    int nclusters;
    int num_buckets_cluster; /* Message.    */
    //struct bucket *addr;

#define NUM_THREADS 16

    omp_set_num_threads(NUM_THREADS);


    ((void)argc);

    rank = atoi(argv[0]);
    nclusters = atoi(argv[1]);
    //open_noc_connectors();
    /* Slave life. */
    k1_timer_init();
    //msg = message_create(0);

    barrier = barrier_open(nclusters);
    barrier_wait(barrier);
    memread(OFF_MSG, &num_buckets_cluster, sizeof(int));

    //memread(OFF_TODO(rank*num_buckets_cluster*sizeof(struct bucket *)), &addr, sizeof(struct bucket *));
    memread(OFF_TODO(rank*num_buckets_cluster*sizeof(int)), &block.elements, (int)(CLUSTER_WORKLOAD/sizeof(int)));

    if (rank == 0) {
	//for (i = 0; i < (int)(CLUSTER_WORKLOAD/sizeof(int)); i++) {
		//printf("Elem: %d\n", block.elements[0]);
		printf("s: %d\n", num_buckets_cluster);
	//}
    }
    mppa_exit(0);
    return(0);
}
