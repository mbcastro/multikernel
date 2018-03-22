#include <nanvix/pm.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sched.h>
#include <unistd.h>
#include <mppa/osconfig.h>

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
	int size = MAX_BUFFER_SIZE;
	mppa_aiocb_t aiocb;
	uint64_t mask;
	int sync_slaves;
	int sync_master;
	int portal_fd;
	int nclusters;
	int clusters[MAX_CLUSTERS];
	char pathname[128];

	assert(argc >= 2);

	nclusters = atoi(argv[1]);

	spawn_slaves(nclusters);

	timer_init();

	/* Open input portal. */
	portal_fd = mppa_open("/mppa/portal/128:8", O_RDONLY);
	assert(portal_fd != -1);

	/* Open sync connector. */
	sprintf(pathname,
			"/mppa/sync/[0..15]:%d",
			4	
	);
	sync_slaves = mppa_open(pathname, O_WRONLY);
	assert(sync_slaves != -1);

	/* Create sync connector. */
	sprintf(pathname,
			"/mppa/sync/128:%d",
			12
	);
	sync_master = mppa_open(pathname, O_RDONLY);
	assert(sync_master != -1);

	/* Setup read operation. */
	mppa_aiocb_ctor(&aiocb, portal_fd, buffer, nclusters*size);
	mppa_aiocb_set_trigger(&aiocb, nclusters);
	assert(mppa_aio_read(&aiocb) != -1);

	/* Benchmark. */
	for (int i = 0; i < NITERATIONS; i++)
	{
		long exec_time;
		long start_time;

		memset(buffer, 0, nclusters*size);

		for (int j = 0; j < nclusters; j++)
			clusters[j] = j;

		/* Wait for slaves. */
		mask = ~((1 << nclusters) - 1);
		assert(mppa_ioctl(sync_master, MPPA_RX_SET_MATCH, mask) == 0);
		assert(mppa_read(sync_master, &mask, sizeof(uint64_t)) != -1);

		/* Unblock slaves. */
		mask = -1;
		assert(mppa_ioctl(sync_slaves, MPPA_TX_SET_RX_RANKS, nclusters, clusters) == 0);
		assert(mppa_write(sync_slaves, &mask, sizeof(uint64_t)) != -1);

		start_time = timer_get();
		assert(mppa_aio_rearm(&aiocb) == nclusters*size);
		exec_time = timer_get();
	
		printf("%d;%s;%d;%d;%ld\n",
			i,
			"ccluster-iocluster",
			nclusters,
			size,
			timer_diff(start_time, exec_time)
		);
	}

	/* House keeping. */
	mppa_close(sync_slaves);
	mppa_close(sync_master);
	mppa_close(portal_fd);

	join_slaves(nclusters);

	return (EXIT_SUCCESS);
}
