/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of Nanvix.
 * 
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <string.h>
#include "kernel.h"

/**
 * @brief Cluster ID.
 */
static int clusterid;

/*===================================================================*
 * Barrier                                                           *
 *===================================================================*/

/**
 * @brief Global barrier.
 */
static struct
{
	int sync_slaves; /**< Slaves sync NoC connector. */
	int sync_master; /**< Master sync NoC connector. */
} barrier;

/**
 * @brief Opens the global barrier.
 */
static void barrier_open(void)
{
	char pathname[128];

	/* Open slave sync connector. */
	sprintf(pathname,
			"/mppa/sync/[%d..%d]:%d",
			CCLUSTER0,
			CCLUSTER15,
			BARRIER_SLAVE_CNOC
	);
	barrier.sync_slaves = mppa_open(pathname, O_RDONLY);
	assert(barrier.sync_slaves != -1);

	/* Open master sync connector. */
	sprintf(pathname,
			"/mppa/sync/%d:%d",
			IOCLUSTER0,
			BARRIER_MASTER_CNOC
	);
	barrier.sync_master = mppa_open(pathname, O_WRONLY);
	assert(barrier.sync_master != -1);
}

/**
 * @brief Waits on the global barrier.
 */
static void barrier_wait(void)
{
	uint64_t mask;

	/* Unblock master. */
	mask = (1 << clusterid);
	assert(mppa_write(barrier.sync_master, &mask, sizeof(uint64_t)) == sizeof(uint64_t));

	/* Wait for master. */
	mask = 0;
	assert(mppa_ioctl(barrier.sync_slaves, MPPA_RX_SET_MATCH, mask) != -1);
	assert(mppa_read(barrier.sync_slaves, &mask, sizeof(uint64_t)) != -1);
}

/**
 * @brief Closes the global barrier.
 */
static void barrier_close(void)
{
	mppa_close(barrier.sync_master);
	mppa_close(barrier.sync_slaves);
}

/*===================================================================*
 * Portal                                                            *
 *===================================================================*/

/**
 * @brief Portal file descriptor.
 */
static int portal_fd;

/**
 * @brief Opens output portal.
 *
 * @param dma DMA channel to use.
 */
static void portal_open(int dma)
{
	char pathname[128];

	/* Open portal connector. */
	sprintf(pathname,
			"/mppa/portal/%d:%d",
			IOCLUSTER0 + dma,
			PORTAL_DNOC
	);
	portal_fd = mppa_open(pathname, O_WRONLY);
	assert(portal_fd != -1);
}

/**
 * @brief Closes output portal.
 */
static inline void portal_close(void)
{
	assert(mppa_close(portal_fd) != -1);
}

/**
 * @brief Writes data to output portal.
 *
 * @param buffer  Target buffer.
 * @parm  size    Write size.
 * @param offset  Write offset.
 */
static inline void portal_write(const void *buffer, int size, int offset)
{
	assert(mppa_pwrite(portal_fd, buffer, size, offset) == size);
}

/*===================================================================*
 * Kernel                                                            *
 *===================================================================*/

/**
 * @brief Buffer.
 */
static char buffer[MAX_BUFFER_SIZE];

/**
 * @brief Benchmarks write operations on a portal connector.
 */
int main(int argc, char **argv)
{
	int dma;    /* DMA channel to use. */
	int size;   /* Write size.         */
	int offset; /* Write offset.       */

	assert(argc == 2);

	/* Retrieve kernel parameters. */
	assert((size = atoi(argv[1])*KB) <= MAX_BUFFER_SIZE);
	clusterid = k1_get_cluster_id();
	dma = clusterid%NR_IOCLUSTER_DMA;
	offset = dma*size;

	portal_open(dma);
	barrier_open();

	/*
	 * Touch data to initialize all pages
	 * and warmup D-cache.
	 */
	memset(buffer, clusterid, size);

	/* 
	 * Benchmark. First iteration is
	 * used to warmup resources.
	 */
	for (int i = 0; i <= NITERATIONS; i++)
	{
		/*
		 * Force cclusters to start
		 * all together.
		 */
		barrier_wait();

		portal_write(buffer, size, offset);

		/* 
		 * Wait for other cclusters to
		 * complete the write operation.
		 */
		barrier_wait();
	}

	/* House keeping. */
	barrier_close();
	portal_close();

	return (EXIT_SUCCESS);
}
