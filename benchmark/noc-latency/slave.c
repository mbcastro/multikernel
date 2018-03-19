#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <nanvix/arch/mppa.h>
#include <string.h>
#include "common.h"
#include "interface_mppa.h"

static char buffer[MAX_BUFFER_SIZE];

int main(int argc,char **argv)
{
	int dma;
	int clusterid;
	char path[128];

	((void) argc);
	((void) argv);

	memset(buffer, 0, MAX_BUFFER_SIZE);

	clusterid  =  arch_get_cluster_id();

	/* Initialize barrier. */
	barrier_t *global_barrier = mppa_create_slave_barrier(BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE);

	/* Initialize NoC connectors. */
	dma = 128 + (clusterid % 4);
	sprintf(path, "/mppa/portal/%d:3", dma);
	portal_t *write_portal = mppa_create_write_portal(path, buffer, MAX_BUFFER_SIZE, dma);
	sprintf(path, "/mppa/portal/%d:%d", clusterid, 4 + clusterid);
	portal_t *read_portal = mppa_create_read_portal(path, buffer, MAX_BUFFER_SIZE, 1);

	mppa_barrier_wait(global_barrier);
  
	/* Master -> Slaves */
	for (int i = MIN_BUFFER_SIZE; i <= MAX_BUFFER_SIZE; i *= 2)
	{
		for (int nb_exec = 1; nb_exec <= NITERATIONS; nb_exec++)
		{
			mppa_barrier_wait(global_barrier);
			mppa_async_read_wait_portal(read_portal);
		}
	}
  
	/* Slaves -> Master */
	for (int i = MIN_BUFFER_SIZE; i <= MAX_BUFFER_SIZE; i *= 2)
	{
		for (int nb_exec = 1; nb_exec <= NITERATIONS; nb_exec++)
		{
			mppa_barrier_wait(global_barrier);
			mppa_async_write_portal(write_portal, buffer, i, clusterid * MAX_BUFFER_SIZE);
			mppa_async_write_wait_portal(write_portal);
		}
	}

	/* House keeping.  */
	mppa_close_barrier(global_barrier);
	mppa_close_portal(write_portal);
	mppa_close_portal(read_portal);

	return (0);
}
