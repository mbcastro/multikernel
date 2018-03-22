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

int sync_slaves;
int sync_master;

int pids[MAX_CLUSTERS];

void spawn_slaves(int nclusters, const char *size) 
{
	const char *argv[] = {"noc-latency-slave", size, NULL};

	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);
}

void join_slaves(int nclusters) 
{
	for (int i = 0; i < nclusters; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);
}

static void _barrier_create(void)
{
	char pathname[128];

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
}

static void _barrier_wait(int nclusters)
{
	uint64_t mask;
	int clusters[MAX_CLUSTERS];

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
}


int main(int argc, char **argv)
{
	int size = MAX_BUFFER_SIZE;
	mppa_aiocb_t aiocb[NR_DMA];
	int portal_fd[NR_DMA];
	int trigger[NR_DMA];
	int nclusters;
	char pathname[128];

	assert(argc >= 2);

	nclusters = atoi(argv[1]);
	size = atoi(argv[2])*KB;

	spawn_slaves(nclusters, argv[2]);

	for (int i = 0; i < NR_DMA; i++)
		trigger[i] = nclusters/NR_DMA;
	for (int i = 0; i < (nclusters%NR_DMA); i++)
		trigger[i]++;

	/* Open input portal. */
	for (int i = 0; i < NR_DMA; i++)
	{
		sprintf(pathname,
				"/mppa/portal/%d:8",
				128 + i 
		);
		portal_fd[i] = mppa_open(pathname, O_RDONLY);
		assert(portal_fd[i] != -1);

		/* Setup read operation. */
		mppa_aiocb_ctor(&aiocb[i], portal_fd[i], &buffer[i*NR_DMA*size], NR_DMA*size);
		mppa_aiocb_set_trigger(&aiocb[i], trigger[i]);
		assert(mppa_aio_read(&aiocb[i]) != -1);
	}

	_barrier_create();

	timer_init();

	/* Benchmark. */
	for (int i = 0; i <= NITERATIONS; i++)
	{
		long start_time, exec_time;

		memset(buffer, 0, MAX_CLUSTERS*size);

		_barrier_wait(nclusters);

		start_time = timer_get();
		for (int i = 0; i < NR_DMA; i++)
			assert(mppa_aio_rearm(&aiocb[i]) == NR_DMA*size);
		exec_time = timer_diff(start_time, timer_get());

		/* Warmup. */
		if (i == 0)
			continue;

		printf("%s;%d;%d;%ld\n",
			"pwrite",
			nclusters,
			size,
			exec_time
		);
	}

	/* House keeping. */
	mppa_close(sync_slaves);
	mppa_close(sync_master);
	for (int i = 0; i < NR_DMA; i++)
		mppa_close(portal_fd[i]);

	join_slaves(nclusters);

	return (EXIT_SUCCESS);
}
