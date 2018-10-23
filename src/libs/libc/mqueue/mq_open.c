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

#include <mqueue.h>
#include <stdarg.h>

#include <nanvix/mqueue.h>

/**
 * @brief Opens and initializes a message queue.
 *
 * @param name  Name of the target message queue.
 * @param oflag Opening flags.
 *
 * @param Upon successful completion, a descriptor of the target
 * message queue is returned. Upon failure, (mqd_t)-1 is returned
 * instead and errno is set to indicate the error.
 */
mqd_t mq_open(const char *name, int oflag, ...)
{
	int ret;
	int create;
	int writable;
	int readable;
	int exclusive;

	create = (oflag & O_CREAT);
	exclusive = (oflag & O_EXCL);
	readable = ((oflag & O_RDONLY) == O_RDONLY) | ((oflag & O_RDWR) == O_RDWR);
	writable = ((oflag & O_WRONLY) == O_WRONLY) | ((oflag & O_RDWR) == O_RDWR);

	/* Create. */
	if (create)
	{
		va_list ap;           /* Arguments pointer.        */
		mode_t mode;          /* Creation mode.            */
		struct mq_attr *attr; /* Message queue attributes. */

		/*
		 * Create function.
		 */
		int (*func)(const char *, int, int, mode_t);

		/* Retrieve additional arguments. */
		va_start(ap, oflag);
		mode = va_arg(ap, mode_t);
		attr = va_arg(ap, struct mq_attr *);
		va_end(ap);

		((void) attr);

		func = (exclusive) ?
			nanvix_mqueue_create_excl :
			nanvix_mqueue_create;

		ret = func(
			name,
			readable,
			writable,
			mode
		);
	}

	/* Open. */
	else
	{
		ret = nanvix_mqueue_open(
			name,
			readable,
			writable
		);
	}

	return (ret);
}
