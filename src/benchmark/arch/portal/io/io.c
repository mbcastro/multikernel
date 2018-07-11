/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include <mppaipc.h>
#include <mppa/osconfig.h>
#include <HAL/hal/core/mp.h>
#include <HAL/hal/core/diagnostic.h>

#include "../kernel.h"

/**
 * @brief Benchmark parameters.
 */
/**@{*/
static int nclusters = 0;         /**< Number of remotes processes.    */
static int niterations = 0;       /**< Number of benchmark parameters. */
static int bufsize = 0;           /**< Buffer size.                    */
static const char *kernel = NULL; /**< Benchmark kernel.               */
/**@}*/

/**
 * @brief Input portal.
 */
static int inportal;

/**
 * @brief ID of slave processes.
 */
static int pids[NR_CCLUSTER];

/**
 * @brief Buffer.
 */
static char buffer[BUFFER_SIZE_MAX];

/*============================================================================*
 * Utility                                                                    *
 *============================================================================*/

/**
 * @brief Spawns remote processes.
 */
static void spawn_remotes(void)
{
	char niterations_str[4];
	char bufsize_str[10];
	const char *argv[] = {
		"/benchmark/mppa256-portal-slave",
		niterations_str,
		bufsize_str,
		kernel,
		NULL
	};

	/* Spawn remotes. */
	sprintf(niterations_str, "%d", niterations);
	sprintf(bufsize_str, "%d", bufsize);
	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);
}

/**
 * @brief Wait for remote processes.
 */
static void join_remotes(void)
{
	for (int i = 0; i < nclusters; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);
}

/*============================================================================*
 * Kernels                                                                    *
 *============================================================================*/

/**
 * @brief Gather kernel.
 */
static void kernel_gather(void)
{
	int sync_fd;

	/* Open sync. */
	assert((sync_fd = mppa_open("/mppa/sync/[0..15]:48", O_WRONLY)) != -1);

	/* Benchmark. */
	for (int k = 0; k <= niterations; k++)
	{
		double total;
		uint64_t t1, t2;

		t1 = __k1_read_dsu_timestamp();
		for (int i = 0; i < nclusters; i++)
		{
			uint64_t mask;
			mppa_aiocb_t aiocb;

			/* Setup read operation. */
			mppa_aiocb_ctor(&aiocb, inportal, buffer, bufsize);
			assert(mppa_aio_read(&aiocb) != -1);

			/* Unblock remote. */
			mask = 1 << i;
			assert(mppa_ioctl(sync_fd, MPPA_TX_SET_RX_RANK, i) != -1);
			assert(mppa_write(sync_fd, &mask, sizeof(uint64_t)) != -1);

			/* Wait read operation to complete. */
			assert(mppa_aio_wait(&aiocb) == bufsize);
		}
		t2 = __k1_read_dsu_timestamp();

		total = (t2 - t1)/((double) MPPA256_FREQ);

		/* Warmup. */
		if (k == 0)
			continue;

		printf("%s;%d;%d;%.2lf;%.2lf\n", 
			kernel,
			bufsize,
			nclusters,
			(total*MEGA)/nclusters,
			(nclusters*bufsize)/total
		);
	}

	/* House keeping. */
	assert(mppa_close(sync_fd) != -1);
}

/*============================================================================*
 * MPPA-256 Portal Microbenchmark Driver                                      *
 *============================================================================*/

/**
 * @brief Portal microbenchmark.
 */
static void benchmark(void)
{
	/* Initialization. */
	spawn_remotes();
	assert((inportal = mppa_open("/mppa/portal/128:48", O_RDONLY)) != -1);

	if (!strcmp(kernel, "gather"))
		kernel_gather();
	
	/* House keeping. */
	assert(mppa_close(inportal) != -1);
	join_remotes();
}

/**
 * @brief Portal Microbenchmark Driver
 */
int main(int argc, const char **argv)
{
	assert(argc == 5);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);
	niterations = atoi(argv[2]);
	bufsize = atoi(argv[3]);
	kernel = argv[4];

	/* Parameter checking. */
	assert((nclusters > 0) && (nclusters <= NR_CCLUSTER));
	assert(niterations > 0);
	assert((bufsize > 0) && (bufsize <= (BUFFER_SIZE_MAX)));
	assert((bufsize%2) == 0);

	benchmark();

	return (EXIT_SUCCESS);
}
