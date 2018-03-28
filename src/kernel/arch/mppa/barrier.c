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

#include <nanvix/arch/mppa.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Barrier.
 */
struct
{
	int local;  /**< Local cluster sync.   */
	int remote; /**< Remote cluster sync. */
} barrier;

/*=======================================================================*
 * barrier_open()                                                        *
 *=======================================================================*/

/**
 * @brief Opens the global barrier.
 *
 * @param ncclusters Number of compute clusters in the barrier.
 */
void barrier_open(int ncclusters)
{
	int local;
	uint64_t mask;
	char pathname[128];

	local = k1_get_cluster_id();

	/* I0 1 cluster barrier. */
	if (local == IOCLUSTER1)
	{
		sprintf(pathname, "/mppa/sync/%d:4", IOCLUSTER0);
		assert((barrier.remote = mppa_open(pathname, O_WRONLY)) != -1);
	}

	/* IO 0 cluster barrier. */
	else if (local == IOCLUSTER0)
	{
		int cclusters[NR_CCLUSTER];

		sprintf(pathname, "/mppa/sync/%d:4", IOCLUSTER0);
		assert((barrier.local = mppa_open(pathname, O_RDONLY)) != -1);
		sprintf(pathname, "/mppa/sync/[%d..%d]:4", CCLUSTER0, CCLUSTER15);
		assert((barrier.remote = mppa_open(pathname, O_WRONLY)) != -1);

		mask = ((uint64_t) -1) & ~(1 << (ncclusters - 1));
		assert(mppa_ioctl(barrier.local, MPPA_RX_SET_MATCH, mask) == 0);

		for (int i = 0; i < ncclusters; i++)
			cclusters[i] = i;

		assert(mppa_ioctl(barrier.remote, MPPA_TX_SET_RX_RANKS, ncclusters, cclusters) == 0);
	}

	else
	{
		sprintf(pathname, "/mppa/sync/%d:4", local);
		assert((barrier.local = mppa_open(pathname, O_RDONLY)) != -1);
		sprintf(pathname, "/mppa/sync/%d:4", IOCLUSTER0);
		assert((barrier.remote = mppa_open(pathname, O_WRONLY)) != -1);

		mask = 0;
		assert(mppa_ioctl(barrier.local, MPPA_RX_SET_MATCH, mask) == 0);
	}
}

/*=======================================================================*
 * barrier_wait()                                                        *
 *=======================================================================*/

int barrier_wait(void)
{
	int local;
	uint64_t mask;

	local = k1_get_cluster_id();

	/* Invalid cluster. */
	if (local == IOCLUSTER1)
		return (-EINVAL);

	/* IO 0 cluster barrier. */
	if (local == IOCLUSTER0)
	{
		assert(mppa_read(barrier.local, &mask, sizeof(uint64_t)) == sizeof(uint64_t));
		mask = ~0;	
		assert(mppa_write(barrier.remote, &mask, sizeof(uint64_t)) == sizeof(uint64_t));
	}
	/* Compute cluster barrier. */
	else
	{
		mask = 1 << k1_get_cluster_id();
		assert(mppa_write(barrier.remote, &mask, sizeof(uint64_t)) == sizeof(uint64_t));
		assert(mppa_read(barrier.local, &mask, sizeof(uint64_t)) == sizeof(uint64_t));
	}
	
	return (0);
}

/*=======================================================================*
 * barrier_release()                                                     *
 *=======================================================================*/

int barrier_release(void)
{
	int local;
	uint64_t mask;

	local = k1_get_cluster_id();

	/* Invalid cluster. */
	if (local != IOCLUSTER1)
		return (-EINVAL);

	mask = ~0;	
	assert(mppa_write(barrier.remote, &mask, sizeof(uint64_t)) == sizeof(uint64_t));

	return (0);
}

/*=======================================================================*
 * barrier_close()                                                       *
 *=======================================================================*/

/**
 * @brief Closes the global barrier.
 */
void barrier_close(void)
{
	int local;

	local = k1_get_cluster_id();

	/* House keeping. */
	if (local != IOCLUSTER1)
		assert(mppa_close(barrier.local) != -1);
	assert(mppa_close(barrier.remote) != -1);
}

