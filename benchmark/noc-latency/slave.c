#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <string.h>
#include "common.h"
#include "interface_mppa.h"
#include <mppa/osconfig.h>

static char buffer[MAX_BUFFER_SIZE];

int main(int argc,char **argv)
{
	uint64_t mask;
	int sync_slaves;
	int sync_master;
	int portal_fd;
	int clusterid;
	char pathname[128];
	int size = MAX_BUFFER_SIZE;

	((void) argc);
	((void) argv);

	clusterid = arch_get_cluster_id();

	/* Open portal connector. */
	portal_fd = mppa_open("/mppa/portal/128:8", O_WRONLY);
	assert(portal_fd != -1);
	assert(mppa_ioctl(portal_fd, MPPA_TX_WAIT_RESOURCE_ON) != -1);

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

	timer_init();

	/* Benchmark. */
	for (int i = 0; i < NITERATIONS; i++)
	{
		long exec_time;
		long start_time;

		memset(buffer, i*clusterid, size);

		/* Unblock master. */
		mask = (1 << clusterid);
		assert(mppa_write(sync_master, &mask, sizeof(uint64_t)) == sizeof(uint64_t));

		/* Wait for master. */
		mask = 0;
		assert(mppa_ioctl(sync_slaves, MPPA_RX_SET_MATCH, mask) != -1);
		assert(mppa_read(sync_slaves, &mask, sizeof(uint64_t)) != -1);

		start_time = timer_get();
		assert(mppa_pwrite(portal_fd, buffer, size, clusterid*size) == size);
		exec_time = timer_diff(start_time, timer_get());

		printf("slave %d: %ld\n", clusterid, exec_time);
	}

	/* House keeping. */
	mppa_close(sync_master);
	mppa_close(sync_slaves);
	mppa_close(portal_fd);

	return (EXIT_SUCCESS);
}
