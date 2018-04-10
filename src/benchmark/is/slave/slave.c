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
//#include <timer.h>
#include "util.c"
#include "message.c"
//#include "slave.h"

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
    struct message msg; /* Message.    */
    int clusterid;
    int inbox;
    int outbox;
    char pathname [128];

#define NUM_THREADS 4

    omp_set_num_threads(NUM_THREADS);


    ((void)argc);

    rank = atoi(argv[0]);
    open_noc_connectors();
	clusterid = k1_get_cluster_id();
	assert(rank == clusterid);
	sprintf(pathname, "/cpu%d", clusterid);
	inbox = mailbox_create(pathname);
	outbox = mailbox_open("/io0");
    /* Slave life. */
    while (1)
    {
        mailbox_read(inbox, &msg);

        switch (msg.type)
        {
            /* SORTWORK. */
            case SORTWORK:
                /* Receive
                 * matrix
                 * block.
                 * */
                block.size = msg.u.sortwork.size;
                data_receive(infd, block.elements, block.size*sizeof(int));

                /* Extract
                 * message
                 * information.
                 * */
                id = msg.u.sortwork.id;

                start = k1_timer_get();
                for (i = block.size; i < (int)(CLUSTER_WORKLOAD/sizeof(int)); i++)
                    block.elements[i] = INT_MAX;
                sort2power(block.elements, CLUSTER_WORKLOAD/sizeof(int), CLUSTER_WORKLOAD/NUM_THREADS);
                end = k1_timer_get();
                total += k1_timer_diff(start, end);

                /* Send
                 * message
                 * back.*/
                msg = *(message_create(SORTRESULT, id, block.size));
                mailbox_write(outbox, &msg);

                data_send(outfd, block.elements, block.size*sizeof(int));


                break;

                /* DIE.
                 * */
            default:
                goto out;
        }
    }

out:

    data_send(outfd, &total, sizeof(uint64_t));
    close_noc_connectors();
    mppa_exit(0);
    return (0);
}

