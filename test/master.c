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
 * @brief NoC connectors testing unit.
 */
static void test_noc(void)
{
	mppa_pid_t server;
	mppa_pid_t client;

	const char *argv0[] = {
		"noc.test",
		"--server",
		NULL
	};

	const char *argv1[] = {
		"noc.test",
		"--client",
		NULL
	};

	server = mppa_spawn(CCLUSTER0, NULL, argv0[0], argv0, NULL);
	client = mppa_spawn(CCLUSTER1, NULL, argv1[0], argv1, NULL);

	mppa_waitpid(server, NULL, 0);
	mppa_waitpid(client, NULL, 0);
}

/**
 * @brief Mailboxes testing unit.
 */
static void test_mailbox(void)
{
	mppa_pid_t server;
	mppa_pid_t client;

	const char *argv0[] = {
		"mailbox.test",
		"--server",
		NULL
	};

	const char *argv1[] = {
		"mailbox.test",
		"--client",
		NULL
	};

	/* Spawn slaves. */
	server = mppa_spawn(CCLUSTER0, NULL, argv0[0], argv0, NULL);
	client = mppa_spawn(CCLUSTER1, NULL, argv1[0], argv1, NULL);

	mppa_waitpid(server, NULL, 0);
	mppa_waitpid(client, NULL, 0);
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
		printf("  noc     NoC connectors.");
		printf("  mailbox Mailboxes.");

		return (0);
	}

	if (!strcmp(argv[1], "noc"))
		test_noc();
	if (!strcmp(argv[1], "mailbox"))
		test_mailbox();

	return (0);
}
