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
#include <stdlib.h>

const char *args[3][3] = {
	{"rmem-regular.kernel", "random", NULL},
	{"rmem-regular.kernel", "read", NULL},
	{"rmem-regular.kernel", "write", NULL}
};

/**
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("missing parameters\n");
	printf("Usage: test rmem <kernel>\n");
	printf("kernel:\n");
	printf("  regular-random Regular writes and reads\n");
	printf("  regular-read   Regular reads\n");
	printf("  regular-write  Regular writes\n");

	exit(-1);
}

/**
 * @brief Parse arguments.
 */
static const char **parse(const char *arg)
{
	if (!strcmp(arg, "regular-random"))
		return (args[0]);
	else if (!strcmp(arg, "regular-read"))
		return (args[1]);

	return (args[2]);
}

/**
 * @brief Remote memory unit test.
 */
int main(int argc, char **argv)
{
	int sync_fd;
	uint64_t mask;
	int ncclusters;
	mppa_pid_t client[NR_CCLUSTER];
	const char **arg;

	/* Missing parameters. */
	if (argc < 3)
		usage();

	/* Sync with remote memory server. */
	sync_fd = mppa_open("/mppa/sync/128:8", O_RDONLY);
	mask = ~(1 << 0);
	mppa_ioctl(sync_fd, MPPA_RX_SET_MATCH, mask);
	mppa_read(sync_fd, &mask, sizeof(uint64_t));
	mppa_close(sync_fd);

	ncclusters = atoi(argv[2]);
	arg = parse(argv[1]);

	printf("[IODDR0] spawning kernels\n");
	for (int i = 0; i < ncclusters; i++)
		client[i] = mppa_spawn(i, NULL, arg[0], arg, NULL);

	for (int i = 0; i < ncclusters; i++)
		mppa_waitpid(client[i], NULL, 0);

	return (EXIT_SUCCESS);
}
