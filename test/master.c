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
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <assert.h>
#include <stdlib.h>

/**
 * @brief Maximum number of arguments for a slave.
 */
#define NR_ARGS 4

/**
 * @brief Parameters.
 */
/**@{*/
static int ncclusters = -1;          /**< Number of compute clusters to spawn. */
static const char *naccesses = NULL; /**< Number of accesses.                  */
static const char *kernel;           /**< Kernel.                              */
static const char *pattern;          /**< Communication pattern.               */
static const char *workload;         /**< Workload.                            */
/**@}*/

/*=======================================================================*
 * options()                                                             *
 *=======================================================================*/

/**
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: test <kernel> [global options] [kernel options]\n");
	printf("Kernels:\n");
	printf("  rmem   Remote memory kernel.\n");
	printf("Global Options:\n");
	printf("  --ncclusters <int> Number of compute clusters\n");
	printf("\nRemote Memory Kernel Options:\n");
	printf("  --pattern <regular | irregular>\n");
	printf("  --workload <read | write | mixed>\n");
	printf("  --naccesses <int>\n");

	exit(-1);
}

/**
 * @brief Gets the kernel parameter.
 *
 * @return Kernel value.
 */
static const char *readargs_get_kernel(const char *arg)
{
	if (!strcmp(arg, "rmem"))
		return (arg);

	usage();

	/* Never gets here. */
	return (NULL);
}

/**
 * Gets the pattern parameter.
 *
 * @returns Pattern value.
 */
static const char *readargs_get_pattern(const char *arg)
{
	if (!strcmp(arg, "irregular"))
		return (arg);
	else if (!strcmp(arg, "regular"))
		return (arg);

	usage();

	/* Never gets here. */
	return (NULL);
}

/**
 * Gets the workload parameter
 *
 * @returns Workload value.
 */
static const char *readargs_get_workload(const char *arg)
{
	if (!strcmp(arg, "read"))
		return (arg);
	else if (!strcmp(arg, "write"))
		return (arg);
	else if (!strcmp(arg, "mixed"))
		return (arg);

	usage();

	/* Never gets here. */
	return (NULL);
}
/**
 * @briefProcessing states while read command line arguments.
 */
enum readargs_states
{
	READ_ARG,
	SET_NCCLUSTERS,
	SET_PATTERN,
	SET_WORKLOAD,
	SET_NACCESSES
};

/**
 * @brief Parses a command line argument.
 *
 * @param arg Target command line argument.
 */
static int readargs_parse(const char *arg)
{
	if (!strcmp(arg, "--ncclusters"))
		return (SET_NCCLUSTERS);
	else if (!strcmp(arg, "--pattern"))
		return (SET_PATTERN);
	else if (!strcmp(arg, "--workload"))
		return (SET_WORKLOAD);
	else if (!strcmp(arg, "--naccesses"))
		return (SET_NACCESSES);

	usage();

	/* Never gets here. */
	return (-1);
}

/**
 * @brief Read command line arguments.
 */
static void readargs(int argc, char **argv)
{
	enum readargs_states state;

	kernel = readargs_get_kernel(argv[1]);

	/* Read command line arguments. */
	state = READ_ARG;
	for (int i = 2; i < argc; i++)
	{
		char *arg;

		arg = argv[i];

		/* Set value. */
		if (state != READ_ARG)
		{
			switch (state)
			{
				/* Set number of compute clusters. */
				case SET_NCCLUSTERS:
					ncclusters = atoi(arg);
					break;

				/* Set pattern. */
				case SET_PATTERN:
					pattern = readargs_get_pattern(arg);
					break;

				/* Set workload. */
				case SET_WORKLOAD:
					workload = readargs_get_workload(arg);
					break;

				/* Set workload. */
				case SET_NACCESSES:
					naccesses = arg;
					break;

				/* Should not happen. */
				default:
					usage();
			}

			state = READ_ARG;
			continue;
		}

		state = readargs_parse(arg);
	}

	/* Check global parameters. */
	if (naccesses == NULL)
		usage();
	if (ncclusters < 0)
		usage();
}

/*=======================================================================*
 * main()                                                                *
 *=======================================================================*/

/**
 * @brief Remote memory unit test.
 */
int main(int argc, char **argv)
{
	int size;
	double bandwidth;
	long start, end, time;
	const char *args[NR_ARGS + 1];
	mppa_pid_t client[NR_CCLUSTER];

	/* Missing parameters. */
	if (argc < 2)
	{
		printf("error: missing parameters\n");
		usage();
	}

	readargs(argc, argv);

	/* Build slave arguments. */
	args[0] = kernel;
	args[1] = pattern;
	args[2] = workload;
	args[3] = naccesses;
	args[4] = NULL;
	
	timer_init();

	/* Wait RMEM server. */
	barrier_open(ncclusters);
	barrier_wait();

#ifdef DEBUG
	printf("[IOCLUSTER0] spawning kernels\n");
#endif

	for (int i = 0; i < ncclusters; i++)
		client[i] = mppa_spawn(i, NULL, args[0], args, NULL);

	/* Wait clients */
	barrier_wait();
	start = timer_get();

#ifdef DEBUG
	printf("[IOCLUSTER0] waiting kernels\n");
#endif

	/* Wait clients. */
	barrier_wait();
	end = timer_get();

	size = ncclusters*atoi(naccesses)*RMEM_BLOCK_SIZE;
	time = timer_diff(start, end);
	bandwidth = size/((double)time);

	printf("[IOCLUSTER0]: %d KB %ld us %.2lf GB/s\n",
			size/1024,
			time,
			(bandwidth*1000000.0)/(1024*1024*1024)
	);

	/* House keeping. */
	for (int i = 0; i < ncclusters; i++)
		mppa_waitpid(client[i], NULL, 0);
	barrier_close();

	return (EXIT_SUCCESS);
}
