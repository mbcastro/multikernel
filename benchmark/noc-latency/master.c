#include <nanvix/pm.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sched.h>
#include <unistd.h>

#include "interface_mppa.h"
#include "common.h"

#define ARGC_SLAVE 4

void 
spawn_slaves(int nclusters) 
{
  int cluster_id;
  int pid;
 
 const char *argv[] = {"noc-latency-slave", NULL};

  // Spawn slave processes
  for (cluster_id = 0; cluster_id < nclusters; cluster_id++) {
    pid = mppa_spawn(cluster_id, NULL, argv[0], argv, NULL);
    assert(pid >= 0);
  }
  
}

int
main(int argc, char **argv) 
{
  int status;
  int pid;
  int nclusters;
  char path[256];
  long start_time, exec_time;

  ((void) argc);
  
  nclusters = atoi(argv[1]);
  
  char *comm_buffer = (char *) malloc(MAX_BUFFER_SIZE * nclusters);
  assert(comm_buffer != NULL);
  init_buffer(comm_buffer, MAX_BUFFER_SIZE * nclusters);
  
 	timer_init();

  // Spawn slave processes
  spawn_slaves(nclusters);
  
  // Initialize global barrier
  barrier_t *global_barrier = mppa_create_master_barrier(BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE, nclusters);
  
  // Initialize communication portal to receive messages from clusters
  int number_dmas = nclusters < 4 ? nclusters : 4;
  portal_t **read_portals = (portal_t **) malloc (sizeof(portal_t *) * number_dmas);
  
  // Each DMA will receive at least one message
  int nb_msgs_per_dma[4] = {1, 1, 1, 1};
  
  // Adjust the number of messages according to the number of clusters
  if (nclusters > 4) {
    int remaining_messages = nclusters - 4;
    while (remaining_messages > 0) {
      for (i = 0; i < number_dmas && remaining_messages > 0; i++) {
	nb_msgs_per_dma[i]++;
	remaining_messages--;
      }
    }
  }
  
  for (i = 0; i < number_dmas; i++) {
    sprintf(path, "/mppa/portal/%d:3", 128 + i);
    read_portals[i] = mppa_create_read_portal(path, comm_buffer, MAX_BUFFER_SIZE * nclusters, nb_msgs_per_dma[i], NULL);
  }
    
  // Initialize communication portals to send messages to clusters (one portal per cluster)
  portal_t **write_portals = (portal_t **) malloc (sizeof(portal_t *) * nclusters);
  for (i = 0; i < nclusters; i++) {
    sprintf(path, "/mppa/portal/%d:%d", i, 4 + i);
    write_portals[i] = mppa_create_write_portal(path, comm_buffer, MAX_BUFFER_SIZE, i);
  }
  
  printf ("type;exec;direction;nclusters;size;time;bandwidth\n");
  
  mppa_barrier_wait(global_barrier);
  
	/* Benchmark. */
	for (int niterations = 1; niterations <= NB_EXEC; niterations++)
	{
		/* Master -> Slaves */
		for (int i = 1; i <= MAX_BUFFER_SIZE; i *= 2)
		{
			mppa_barrier_wait(global_barrier);

			start_time = timer_get();

			/* Send data. */
			for (int j = 0; j < nclusters; j++)
				mppa_async_write_portal(write_portals[j], comm_buffer, i, 0);

			/* Wait data transfers to finish. */
			for (int j = 0; j < nclusters; j++)
				mppa_async_write_wait_portal(write_portals[j]);

			exec_time = timer_diff(start_time, timer_get());
			printf("portal;%d;%s;%d;%d;%ld\n", niterations, "master-slave", nclusters, i, exec_time);
		}

		/* Slaves -> Master */
		for (int i = 1; i <= MAX_BUFFER_SIZE; i *= 2)
		{
			mppa_barrier_wait(global_barrier);

			start_time = timer_get();

			/* Send data and wait. */
			for (int j = 0; j < number_dmas; j++)
				mppa_async_read_wait_portal(read_portals[j]);

			exec_time = timer_diff(start_time, timer_get());
			printf ("portal;%d;%s;%d;%d;%ld;\n",
				niterations,
				"slave-master",
				nclusters,
				i,
				exec_time
			);
		}
	}
  
  /* Wait for slaves. */
  for (pid = 0; pid < nclusters; pid++)
	assert(mppa_waitpid(pid, NULL, 0) != -1);
  
  /* House keeping. */
  mppa_close_barrier(global_barrier);
  for (int i = 0; i < number_dmas; i++)
    mppa_close_portal(read_portals[i]);
  for (int i = 0; i < nclusters; i++)
    mppa_close_portal(write_portals[i]);
  
  return 0;
}
