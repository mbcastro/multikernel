/*
 * Copyright (C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * mppa/slave/main.c - Slave main().
 */

#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <limits.h>
#include <omp.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include "util.c"
#include "message.c"

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
    int i;               /* Loop index. */
    int id;              /* Bucket ID.  */
    struct message *msg; /* Message.    */

#define NUM_THREADS 4

    omp_set_num_threads(NUM_THREADS);


    ((void)argc);

    rank = atoi(argv[0]);
    open_noc_connectors();
    /* Slave life. */
    k1_timer_init();
    msg = message_create(0);
    while (1)
    {
        data_receive(infd, msg, sizeof(struct message));

        switch (msg->type)
        {
            /* SORTWORK. */
            case SORTWORK:
                /* Receive
                 * matrix
                 * block.
                 * */
                block.size = msg->u.sortwork.size;
                data_receive(infd, block.elements, block.size*sizeof(int));
                /* Extract
                 * message
                 * information.
                 * */
                id = msg->u.sortwork.id;

                msg = message_create(SORTRESULT, id, block.size);
                data_send(outfd, msg, sizeof(struct message));


                start = k1_timer_get();
                for (i = block.size; i < (int)(CLUSTER_WORKLOAD/sizeof(int)); i++)
                    block.elements[i] = INT_MAX;
                sort2power(block.elements, CLUSTER_WORKLOAD/sizeof(int), CLUSTER_WORKLOAD/NUM_THREADS);
                end = k1_timer_get();
                total += k1_timer_diff(start, end);
                /* Send
                 * message
                 * back.*/
                data_send(outfd, block.elements, block.size*sizeof(int));


                break;

                /* DIE.
                 * */
            default:
		data_send(outfd, &total, sizeof(long));
		close_noc_connectors();
		mppa_exit(0);
		return (0);
        }
    }


}

