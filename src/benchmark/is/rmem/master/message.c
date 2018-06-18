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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
//#include "util.c"
#include "master.h"
//#include "ipc.c"
/*
 *  * Creates a message.
 *   */
struct message *message_create(int type, ...)
{
	va_list ap;          /* Arguments pointer. */
	struct message *msg; /* Message.           */

	va_start(ap, type);

	msg = smalloc(sizeof(struct message));

	/* Parse type of message. */
	switch (type)
	{
		/* SORTWORK. */
		case SORTWORK :
			msg->u.sortwork.id = va_arg(ap, int);
			msg->u.sortwork.size = va_arg(ap, int);
			msg->type = SORTWORK;
			break;

			/* SORTRESULT. */
		case SORTRESULT :
			msg->u.sortresult.id = va_arg(ap, int);
			msg->u.sortresult.size = va_arg(ap, int);
			msg->type = SORTRESULT;
			break;

			/* FINDWORK. */
		case FINDWORK :
			msg->u.findwork.i0 = va_arg(ap, int);
			msg->u.findwork.j0 = va_arg(ap, int);
			msg->u.findwork.height = va_arg(ap, int);
			msg->u.findwork.width = va_arg(ap, int);
			msg->type = FINDWORK;
			break;

			/* FINDRESULT. */
		case FINDRESULT :
			msg->u.findresult.i0 = va_arg(ap, int);
			msg->u.findresult.j0 = va_arg(ap, int);
			msg->u.findresult.ipvt = va_arg(ap, int);
			msg->u.findresult.jpvt = va_arg(ap, int);
			msg->type = FINDRESULT;
			break;

			/* REDUCTWORK. */
		case REDUCTWORK :
			msg->u.reductwork.ipvt = va_arg(ap, int);
			msg->u.reductwork.i0 = va_arg(ap, int);
			msg->u.reductwork.j0 = va_arg(ap, int);
			msg->u.reductwork.height = va_arg(ap, int);
			msg->u.reductwork.width = va_arg(ap, int);
			msg->type = REDUCTWORK;
			break;

			/* REDUCTRESULT. */
		case REDUCTRESULT :
			msg->u.reductresult.i0 = va_arg(ap, int);
			msg->u.reductresult.j0 = va_arg(ap, int);
			msg->u.reductresult.height = va_arg(ap, int);
			msg->u.reductresult.width = va_arg(ap, int);
			msg->type = REDUCTRESULT;
			break;

			/* DIE. */
		default :
			msg->type = DIE;
			break;
	}

	msg->next = NULL;
	va_end(ap);

	return (msg);
}

/*
 *  * Destroys a message.
 *   */
void message_destroy(struct message *msg)
{
	free(msg);
}

/*
 *  * Sends a message.
 *   */
/*void message_send(int m_outfd, struct message *msg)
{
	data_send(m_outfd, msg, sizeof(struct message));
}
*/
/*
 *  * Receives a message.
 *   */
/*struct message *message_receive(int m_infd, int remote)
{
	struct message *msg;

	msg = message_create(DIE);

	data_receive(m_infd, remote, msg, sizeof(struct message));

	return (msg);
}
*/
