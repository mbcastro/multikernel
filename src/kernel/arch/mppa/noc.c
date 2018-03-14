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
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Portal NoC connectors.
 */
static int portals[NR_CCLUSTER];

/**
 * @brief Cluster ID of current process.
 */
static int myrank;

/**
 * @brief Initializes NoC connectors.
 *
 * @parm ncclusters Number of compute clusters to initialize.
 */
void nanvix_noc_init(int ncclusters)
{
	myrank = mppa_getpid();

	/* Open NoC Connectors. */
	for (int i = 0; i < ncclusters; i++)
	{
		char pathname[128];

		sprintf(pathname, "/mppa/portal/%d:%d", i, 16 + i);

		portals[i] = mppa_open(pathname, (i == myrank) ? O_RDONLY : O_WRONLY);
		assert(portals[i] != -1);
	}
}

#define N 1024

int first = 0;
int last = 0;
int buffer[N];

/**
 * @brief Reads data from the local NoC connector.
 *
 * @param buf  Location where data should be written to.
 * @param size Number of bytes to read.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_noc_receive(void *buf, size_t size)
{
	int counter;

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid size. */
	if (size < 1)
		return (-EINVAL);

	mppa_ioctl(portals[myrank], MPPA_RX_GET_COUNTER, &counter);


	printf("counter=%d first=%d last=%d\n", counter, first, last);
	if ((counter == 0) && (first==last))
	{
			mppa_aiocb_t aiocb = MPPA_AIOCB_INITIALIZER(portals[myrank], &buffer[last], sizeof(int));
			mppa_aiocb_set_trigger(&aiocb, 1);
			printf("+ before %d: %d %d %d\n", myrank, counter, first, last);
			mppa_aio_read(&aiocb);
			mppa_aio_wait(&aiocb);
			last = (last+1)%N;
			mppa_ioctl(portals[myrank], MPPA_RX_GET_COUNTER, &counter);
			printf("+ after %d: %d %d %d\n", myrank, counter, first, last);
	}
	else
	{
		do
		{
			mppa_aiocb_t aiocb = MPPA_AIOCB_INITIALIZER(portals[myrank], &buffer[last], sizeof(int));
			mppa_aiocb_set_trigger(&aiocb, 0);
			printf("before %d: %d %d %d\n", myrank, counter, first, last);
			mppa_aio_read(&aiocb);
			int n = mppa_aio_wait(&aiocb);
			last = (last+counter)%N;
			mppa_ioctl(portals[myrank], MPPA_RX_GET_COUNTER, &counter);
			printf("after %d: %d %d %d %d\n", myrank, counter, first, last, n);
		} while (counter > 0);
	}

	memcpy(buf, &buffer[first], sizeof(int));
	first = (first+1)%N;

	return (0);
}

/**
 * @brief Writes data to a remote NoC connector.
 *
 * @param id   ID of the target NoC connector.
 * @param buf  Location where data should be read from.
 * @param size Number of bytes to write.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int nanvix_noc_send(int id, const void *buf, size_t size)
{
	/* Invalid cluster ID. */
	if (id == myrank)
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid size. */
	if (size < 1)
		return (-EINVAL);

	mppa_pwrite(portals[id], buf, size, 0);

	return (0);
}

