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
#include <nanvix/name.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static pthread_mutex_t lock;

/*====================================================================*
 * main                                                               *
 *====================================================================*/

/**
 * @brief Remote name unit test.
 */
int main()
{
  struct name_message msg;
  struct name_message ans;
  char pathname[16];
  int inbox;         /* Mailbox for small messages. */

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = NAME_QUERY;
	msg.id = 0;     					/**< Cluster ID.  		*/

  sprintf(pathname, "/cpu%d", k1_get_cluster_id());
  pthread_mutex_lock(&lock);
		inbox = mailbox_create(pathname);
	pthread_mutex_unlock(&lock);

	/* Send name request. */
	mailbox_write(mailbox_open("/io0"), &msg);

  ans.op = 3;
  while(1){
    mailbox_read(inbox, &ans);

    if(ans.op != 3)
      printf("Message : op = %d, name = %s\n", ans.op, ans.name);
  }

	return (EXIT_SUCCESS);
}
