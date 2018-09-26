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

#ifndef MQUEUE_H_
#define MQUEUE_H_

	#include <fcntl.h>
	#include <signal.h>
	#include <sys/types.h>
	#include <time.h>

	/**
	 * @brief Maximum priority for a message (not included).
	 */
	#define MQ_PRIO_MAX 256

	/**
	 * @brief Attributes of a message queue.
	 */
	struct mq_attr
	{
		long mq_flags;    /**< Message queue flags.                 */
		long mq_maxmsg;   /**< Maximum number of messages.          */
		long mq_msgsize;  /**< Maximum message size.                */
		long mq_curmsgs;  /**< Number of messages currently queued. */
	};

	/**
	 * @brief Descriptor of a message queue.
	 */
	typedef int mqd_t;

	/* Forward definitions. */
	extern mqd_t mq_open(const char *, int, ...);
	extern int mq_unlink(const char *);
	extern int mq_close(mqd_t);
	extern ssize_t mq_receive(mqd_t, char *, size_t, unsigned *);
	extern int mq_send(mqd_t, const char *, size_t, unsigned);

#endif /* MQUEUE_H_ */

