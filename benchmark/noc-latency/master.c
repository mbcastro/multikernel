#include <nanvix/pm.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sched.h>
#include <unistd.h>

#include "interface_mppa.h"
#include "common.h"

static char buffer[MAX_CLUSTERS*MAX_BUFFER_SIZE];

int pids[MAX_CLUSTERS];

void spawn_slaves(int nclusters) 
{
	const char *argv[] = {"noc-latency-slave", NULL};

	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);
}

void join_slaves(int nclusters) 
{
	for (int i = 0; i < nclusters; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);
}

int main(int argc, char **argv) 
{
  int nclusters;
  char path[256];
  long start_time, exec_time;

  assert(argc >= 2);
  
  nclusters = atoi(argv[1]);
  
 	timer_init();
  
  init_buffer(buffer, MAX_BUFFER_SIZE * nclusters);

  spawn_slaves(nclusters);
  
  // Initialize global barrier
  barrier_t *global_barrier = mppa_create_master_barrier(BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE, nclusters);
  
  // Initialize communication portal to receive messages from clusters
  int ndmas = nclusters < 4 ? nclusters : 4;
  portal_t **read_portals = (portal_t **) malloc (sizeof(portal_t *) * ndmas);
  
  // Each DMA will receive at least one message
  int nb_msgs_per_dma[4] = {1, 1, 1, 1};
  
  // Adjust the number of messages according to the number of clusters
  if (nclusters > 4) {
    int remaining_messages = nclusters - 4;
    while (remaining_messages > 0) {
      for (int i = 0; i < ndmas && remaining_messages > 0; i++) {
	nb_msgs_per_dma[i]++;
	remaining_messages--;
      }
    }
  }
  
  for (int i = 0; i < ndmas; i++) {
    sprintf(path, "/mppa/portal/%d:3", 128 + i);
    read_portals[i] = mppa_create_read_portal(path, buffer, MAX_BUFFER_SIZE * nclusters, nb_msgs_per_dma[i]);
  }
    
  portal_t **write_portals = (portal_t **) malloc (sizeof(portal_t *) * nclusters);
  for (int i = 0; i < nclusters; i++) {
    sprintf(path, "/mppa/portal/%d:%d", i, 4 + i);
    write_portals[i] = mppa_create_write_portal(path, buffer, MAX_BUFFER_SIZE, i);
  }
  
  printf ("direction;nclusters;size;time\n");
  
  mppa_barrier_wait(global_barrier);
  
	/* Master -> Slaves */
	for (int i = MIN_BUFFER_SIZE; i <= MAX_BUFFER_SIZE; i *= 2)
	{
		for (int niterations = 1; niterations <= NITERATIONS; niterations++)
		{
			mppa_barrier_wait(global_barrier);

			start_time = timer_get();

			/* Send data. */
			for (int j = 0; j < nclusters; j++)
				mppa_async_write_portal(write_portals[j], buffer, i, 0);

			/* Wait data transfers to finish. */
			for (int j = 0; j < nclusters; j++)
				mppa_async_write_wait_portal(write_portals[j]);

			exec_time = timer_diff(start_time, timer_get());

			printf("%s;%d;%d;%ld\n",
					"master-slaves",
					nclusters,
					i,
					exec_time
			);
		}
	}

	/* Slaves -> Master */
	for (int i = MIN_BUFFER_SIZE; i <= MAX_BUFFER_SIZE; i *= 2)
	{
		for (int niterations = 1; niterations <= NITERATIONS; niterations++)
		{
			mppa_barrier_wait(global_barrier);

			start_time = timer_get();

			/* Send data and wait. */
			for (int j = 0; j < ndmas; j++)
				mppa_async_read_wait_portal(read_portals[j]);

			exec_time = timer_diff(start_time, timer_get());

			printf("%s;%d;%d;%ld\n",
				"slaves-master",
				nclusters,
				i,
				exec_time
			);
		}
	}
  
	join_slaves(nclusters);
  
	/* House keeping. */
	mppa_close_barrier(global_barrier);
	for (int i = 0; i < ndmas; i++)
		mppa_close_portal(read_portals[i]);
	for (int i = 0; i < nclusters; i++)
		mppa_close_portal(write_portals[i]);

	return (0);
}
