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
 * @brief Number of barriers.
 */
#define NR_BARRIER 16

/**
 * @brief Barrier flags.
 */
/**@{*/
#define BARRIER_USED       (1 << 0)
#define BARRIER_IOCLUSTERS (1 << 1)
/**@}*/

/**
 * @brief Barrier.
 */
struct barrier
{
	int local;  /**< Local cluster.  */
	int remote; /**< Remote cluster. */
	int flags;  /**< Flags.          */
};

/**
 * @brief table of barriers.
 */
static struct barrier barriers[NR_BARRIER];

/*=======================================================================*
 * barrier_alloc()                                                       *
 *=======================================================================*/

/**
 * @brief Allocates a barrier.
 *
 * @return Upon successful completion the ID of the newly allocated
 * barrier is returned. Upon failure, a negative error code is returned
 * instead.
 */
static int barrier_alloc(void)
{
	/* Search for a free barrier. */
	for (int i = 0; i < NR_BARRIER; i++)
	{
		/* Found. */
		if (!(barriers[i].flags & BARRIER_USED))
		{
			barriers[i].flags |= BARRIER_USED;
			return (i);
		}
	}

	return (-ENOENT);
}

/*=======================================================================*
 * barrier_free()                                                        *
 *=======================================================================*/

/**
 * @brief Frees a barrier.
 *
 * @param barid ID of the target barrier.
 */
static void barrier_free(int barid)
{
	/* Sanity check. */
	assert((barid >= 0) && (barid < NR_BARRIER));
	assert(barriers[barid].flags & BARRIER_USED);

	barriers[barid].flags = 0;
	mppa_close(barriers[barid].fd);
}
/*=======================================================================*
 * barrier_noctag()                                                      *
 *=======================================================================*/

/**
 * @brief Computes the barrier NoC tag for a cluster.
 *
 * @param local Id of target cluster.
 */
static int barrier_noctag(int local)
{
	if ((local >= CCLUSTER0) && (local <= CCLUSTER15))
		return (96 + local);
	else if (local == IOCLUSTER0)
		return (96 + 16 + 0);
	else if (local == IOCLUSTER1)
		return (96 + 16 + 1);

	return (0);
}

/*=======================================================================*
 * barrier_open()                                                        *
 *=======================================================================*/

/**
 * @brief Opens a barrier.
 *
 * @param name Barrier group.
 *
 * @returns Upon successful completion, the ID of the target barrier is
 * returned. Upon failure, a negative error code is returned instead.
 */
int barrier_open(int group)
{
	int local;          /* ID of local cluster.               */
	int fd;             /* File descriptor for NoC connector. */
	int barid;          /* ID of mailbix.                     */
	char pathname[128]; /* NoC connector name.                */
	int noctag;         /* NoC tag used for transfers.        */

	assert(local != arch_get_cluster_id());

	/* Invalid barrier group. */
	if ((group != BARRIER_IOCLUSTERS) || (group != BARRIER_CCLUSTERS))
		return (-EINVAL);

	/* Invalid barrier group. */

	/* Allocate a barrier. */
	barid = barrier_alloc();
	if (barid < 0)
		return (barid);

	/* Open local barrier. */
	noctag = barrier_noctag(local);
	sprintf(pathname,
			"/mppa/sync/%d:%d",
			local,
			noctag,
			remotes,
			noctag,
			BARRIER_MSG_SIZE
	);
	assert((fd = mppa_open(pathname, O_WRONLY)) != -1);

	/* Initialize barrier. */
	barriers[barid].fd = fd;
	barriers[barid].flags |= BARRIER_WRONLY;

	return (barid);
}

/*=======================================================================*
 * barrier_read()                                                        *
 *=======================================================================*/

/**
 * @brief Reads data from a barrier.
 *
 * @param barid ID of the target barrier.
 * @param buf   Location from where data should be written.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int barrier_read(int barid, void *buf)
{
	/* Invalid barrier ID.*/
	if ((barid < 0) || (barid >= NR_BARRIER))
		return (-EINVAL);

	/*  Invalid barrier. */
	if (!(barriers[barid].flags & BARRIER_USED))
		return (-EINVAL);

	/* Operation no supported. */
	if (barriers[barid].flags & BARRIER_WRONLY)
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	assert(mppa_read(barriers[barid].fd, buf, BARRIER_MSG_SIZE) == BARRIER_MSG_SIZE);

	return (0);
}

/*=======================================================================*
 * barrier_write()                                                       *
 *=======================================================================*/

/**
 * @brief Reases all processes that are blocked in a barrier.
 *
 * @param barid ID of the target barrier.
 * @param buf   Location from where data should be read.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int barrier_release(int barid, const void *buf)
{
	/* Invalid barrier ID.*/
	if ((barid < 0) || (barid >= NR_BARRIER))
		return (-EINVAL);

	/*  Invalid barrier. */
	if (!(barriers[barid].flags & BARRIER_USED))
		return (-EINVAL);

	/* Operation no supported. */
	if (!(barriers[barid].flags & BARRIER_WRONLY))
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	assert(mppa_write(barriers[barid].fd, buf, BARRIER_MSG_SIZE) == BARRIER_MSG_SIZE);

	return (0);
}

/*=======================================================================*
 * barrier_close()                                                       *
 *=======================================================================*/

/**
 * @brief Closes a barrier.
 *
 * @param barid ID of the target barrier.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int barrier_close(int barid)
{
	/* Invalid barrier ID.*/
	if ((barid < 0) || (barid >= NR_BARRIER))
		return (-EINVAL);

	/*  Invalid barrier. */
	if (!(barriers[barid].flags & BARRIER_USED))
		return (-EINVAL);

	barrier_free(barid);

	return (0);
}

/*=======================================================================*
 * barrier_unlink()                                                      *
 *=======================================================================*/

/**
 * @brief Destroys a barrier.
 *
 * @param barid ID of the target barrier.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int barrier_unlink(int barid)
{
	/* Invalid barrier ID.*/
	if ((barid < 0) || (barid >= NR_BARRIER))
		return (-EINVAL);

	/*  Invalid barrier. */
	if (!(barriers[barid].flags & BARRIER_USED))
		return (-EINVAL);

	barrier_free(barid);

	return (0);
}

barrier_t *mppa_create_master_barrier (char *path_master, char *path_slave, int clusters) {
  int status, i;
  int ranks[clusters];
  long long match;
  
  barrier_t *ret = (barrier_t*) malloc (sizeof (barrier_t));
  
  ret->sync_fd_master = mppa_open(path_master, O_RDONLY);
  assert(ret->sync_fd_master != -1);
  
  ret->sync_fd_slave = mppa_open(path_slave, O_WRONLY);
  assert(ret->sync_fd_slave != -1);

  // set all bits to 1 except the less significative "cluster" bits (those ones are set to 0).
  // when the IO receives messagens from the clusters, they will set their correspoding bit to 1.
  // the mppa_read() on the IO will return when match = 11111...1111
  match = (long long) - (1 << clusters);
  status = mppa_ioctl(ret->sync_fd_master, MPPA_RX_SET_MATCH, match);
  assert(status == 0);
  
  for (i = 0; i < clusters; i++)
    ranks[i] = i;
  
  // configure the sync connector to receive message from "ranks"
  status = mppa_ioctl(ret->sync_fd_slave, MPPA_TX_SET_RX_RANKS, clusters, ranks);
  assert(status == 0);
  
  ret->mode = BARRIER_MASTER;
  
  return ret;
}

barrier_t *mppa_create_slave_barrier (char *path_master, char *path_slave) {
  int status;
  
  barrier_t *ret = (barrier_t*) malloc (sizeof (barrier_t));
  assert(ret != NULL);
  
  ret->sync_fd_master = mppa_open(path_master, O_WRONLY);
  assert(ret->sync_fd_master != -1);
  
  ret->sync_fd_slave = mppa_open(path_slave, O_RDONLY);
  assert(ret->sync_fd_slave != -1);
  
  // set match to 0000...000.
  // the IO will send a massage containing 1111...11111, so it will allow mppa_read() to return
  uint64_t mask = 0;
  status = mppa_ioctl(ret->sync_fd_slave, MPPA_RX_SET_MATCH, mask);
  assert(status == 0);
  
  ret->mode = BARRIER_SLAVE;
  
  return ret;
}

void mppa_barrier_wait(barrier_t *barrier) {
  int status;
  long long dummy;
  
  if(barrier->mode == BARRIER_MASTER) {
    dummy = -1;
    long long match;
    
    // the IO waits for a message from each of the clusters involved in the barrier
    // each cluster will set its correspoding bit on the IO (variable match) to 1
    // when match = 11111...1111 the following mppa_read() returns
    status = mppa_read(barrier->sync_fd_master, &match, sizeof(match));
    assert(status == sizeof(match));
    
    // the IO sends a message (dummy) containing 1111...1111 to all slaves involved in the barrier
    // this will unblock their mppa_read()
    status = mppa_write(barrier->sync_fd_slave, &dummy, sizeof(long long));
    assert(status == sizeof(long long));
  }
  else {
    dummy = 0;
    long long mask;

    // the cluster sets its corresponding bit to 1
    mask = 0;
    mask |= 1 << arch_get_cluster_id();
    
    // the cluster sends the mask to the IO
    status = mppa_write(barrier->sync_fd_master, &mask, sizeof(mask));
    assert(status == sizeof(mask));
    
    // the cluster waits for a message containing 1111...111 from the IO to unblock
    status = mppa_read(barrier->sync_fd_slave, &dummy, sizeof(long long));
    assert(status == sizeof(long long));
  }
}

void mppa_close_barrier (barrier_t *barrier) {
  assert(mppa_close(barrier->sync_fd_master) != -1);
  assert(mppa_close(barrier->sync_fd_slave) != -1);
  free(barrier);
}

