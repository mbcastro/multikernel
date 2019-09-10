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

#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <mqueue.h>

#include <nanvix/mqueues.h>

/**
 * @brief Receives a message from a message queue.
 *
 * @param mqdes Descriptor of the received message queue.
 * @param msg   Location to store the received message.
 * @param len   Length of the received message (in bytes).
 * @param prio  Location to store the priority of the received message.
 *
 * @param Upon successful completion, the length of the received
 * message in bytes is returned, and the message is removed from the
 * queue. Upon failure, no message is removed from the queue, -1 is
 * returned and errno is set to indicate the error.
 */
ssize_t mq_receive(mqd_t mqdes, char *msg, size_t len, unsigned *prio)
{
	/* Invalid descriptor. */
	if (mqdes < 0)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Invalid message. */
	if (msg == NULL)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Invalid length. */
	if (len == 0)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Invalid priority pointer. */
	if (prio == NULL)
	{
		errno = EINVAL;
		return (-1);
	}

	return (nanvix_mqueue_receive(mqdes, msg, len, prio));
}

