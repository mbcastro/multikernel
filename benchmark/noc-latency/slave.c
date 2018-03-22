#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <string.h>
#include "common.h"
#include <limits.h>
#include <mppa/osconfig.h>

int clusterid;

static char buffer[MAX_BUFFER_SIZE];

int sync_slaves;
int sync_master;

static void _barrier_create(void)
{
	char pathname[128];

	/* Create sync connector. */
	sprintf(pathname,
			"/mppa/sync/[0..15]:%d",
			4
	);
	sync_slaves = mppa_open(pathname, O_RDONLY);
	assert(sync_slaves != -1);

	/* Create sync connector. */
	sprintf(pathname,
			"/mppa/sync/128:%d",
			12
	);
	sync_master = mppa_open(pathname, O_WRONLY);
	assert(sync_master != -1);
}

static void _barrier_wait(void)
{
	uint64_t mask;

	/* Unblock master. */
	mask = (1 << clusterid);
	assert(mppa_write(sync_master, &mask, sizeof(uint64_t)) == sizeof(uint64_t));

	/* Wait for master. */
	mask = 0;
	assert(mppa_ioctl(sync_slaves, MPPA_RX_SET_MATCH, mask) != -1);
	assert(mppa_read(sync_slaves, &mask, sizeof(uint64_t)) != -1);
}

int main(int argc,char **argv)
{
	int portal_fd;
	char pathname[128];
	int size = MAX_BUFFER_SIZE;

	((void) argc);
	((void) argv);

	clusterid = arch_get_cluster_id();

	/* Open portal connector. */
	sprintf(pathname,
			"/mppa/portal/%d:8",
			128 + (clusterid%NR_DMA)
	);
	portal_fd = mppa_open(pathname, O_WRONLY);
	assert(portal_fd != -1);
	assert(mppa_ioctl(portal_fd, MPPA_TX_WAIT_RESOURCE_ON) != -1);
	assert(mppa_ioctl(portal_fd, MPPA_TX_NOTIFY_ON) != -1);

	timer_init();

	_barrier_create();

	/* Benchmark. */
	long min = LONG_MAX;
	for (int i = 0; i < NITERATIONS; i++)
	{
		long start_time, exec_time;

		_barrier_wait();

		start_time = timer_get();
		assert(mppa_pwrite(portal_fd, buffer, size, (clusterid%NR_DMA)*size) == size);

		_barrier_wait();

		exec_time = timer_diff(start_time, timer_get());

		if (exec_time < min)
			min = exec_time;
	}

	printf("%s;%d;%ld\n",
		"ccluster-iocluster",
		size,
		min
	);

	/* House keeping. */
	mppa_close(sync_master);
	mppa_close(sync_slaves);
	mppa_close(portal_fd);

	return (EXIT_SUCCESS);
}
