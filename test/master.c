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

/**
 * @brief Mailboxes testing unit.
 */
static void test_mailbox(void)
{
	int sync_fd;
	uint64_t mask;
	mppa_pid_t client[NR_CCLUSTER];

	sync_fd = mppa_open("/mppa/sync/128:8", O_RDONLY);

	const char *argv1[] = {
		"rmem-client.test",
		"client",
		NULL
	};

	mask = ~(1 << 0);
	mppa_ioctl(sync_fd, MPPA_RX_SET_MATCH, mask);
	mppa_read(sync_fd, &mask, sizeof(uint64_t));
	mppa_close(sync_fd);

	printf("spawning mailbox clients\n");
	for (int i = 0; i < NR_CCLUSTER; i++)
		client[i] = mppa_spawn(i, NULL, argv1[0], argv1, NULL);

	for (int i = 0; i < NR_CCLUSTER; i++)
		mppa_waitpid(client[i], NULL, 0);
}

/**
 * @brief IPC library unit test
 */
int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("missing parameters");
		printf("usage: test <testing unit>");
		printf("  mailbox Mailboxes.");

		return (0);
	}

	if (!strcmp(argv[1], "mailbox"))
		test_mailbox();

	return (0);
}
