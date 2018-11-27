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

#ifndef NANVIX_MQUEUE_H_
#define NANVIX_MQUEUE_H_

	#include <sys/types.h>
	#include <stdint.h>

	#include "limits.h"

	/**
	 * @brief Maximum number of opened message queues.
	 */
	#define MQUEUE_OPEN_MAX 8

	/**
	 * @brief Default size for a message.
	 */
	#define MQUEUE_MESSAGE_SIZE 256

	/**
	 * @brief Maximum number of stored messages.
	 */
	#define MQUEUE_MESSAGE_MAX 10

	/**
	 * @brief Maximum priority for a message.
	 */
	#define MQUEUE_PRIO_MAX 256

	/**
	 * @brief Operation types for message queue server.
	 */
	/**@{*/
	#define MQUEUE_EXIT         0 /**< Exit request.                      */
	#define MQUEUE_OPEN         1 /**< Open a message queue.              */
	#define MQUEUE_CLOSE        4 /**< Close a message queue.             */
	#define MQUEUE_UNLINK       5 /**< Unlink a message queue.            */
	#define MQUEUE_CREATE       6 /**< Create a message queue.            */
	#define MQUEUE_CREATE_EXCL  7 /**< Create an exclusive message queue. */
	#define MQUEUE_SEND         8 /**< Send a message.                    */
	#define MQUEUE_RECEIVE      9 /**< Receive a message.                 */
	#define MQUEUE_SUCCESS     10 /**< Success.                           */
	#define MQUEUE_FAILURE     11 /**< Failure.                           */
	/**@}*/

	/**
	 * @brief Message queue.
	 */
	struct mqueue_message
	{
		uint16_t source; /**< Source cluster.  */
		int16_t opcode;  /**< Operation.       */
		uint16_t seq;    /**< Sequence number. */

		/* Operation-specific fields. */
		union 
		{
			/* Create message 1. */
			struct {
				mode_t mode;  /**< Access permissions. */
				int readable; /**< May read?           */
				int writable; /**< May writable?       */
			} create1;

			/* Create message 2. */
			struct {
				char name[NANVIX_MQUEUE_NAME_MAX]; /**< Message queue name. */
			} create2;

			/* Open message 1. */
			struct {
				int readable; /**< May read?     */
				int writable; /**< May writable? */
			} open1;

			/* Open message 2. */
			struct {
				char name[NANVIX_MQUEUE_NAME_MAX]; /**< Message queue name. */
			} open2;

			/* Close message. */
			struct {
				int mqueueid; /**< ID of target message queue. */
			} close;

			/* Unlink message. */
			struct {
				char name[NANVIX_MQUEUE_NAME_MAX]; /**< Message queue name. */
			} unlink;

			/* Send message. */
			struct {
				int mqueueid;  /**< ID of the target message queue.   */
				size_t len;    /**< Length of the message (in bytes). */
				unsigned prio; /**< Priority of the message.          */
			} send;

			/* Receive message. */
			struct {
				int mqueueid; /**< ID of the target message queue.   */
				size_t len;   /**< Length of the message (in bytes). */
			} receive;

			/* Return message. */
			union
			{
				unsigned prio; /**< Message priority.            */
				int status;    /**< Status code.                 */
				int mqueueid;  /**< Newly created message queue. */
			} ret;
		} op;
	};

	/* Forward definitions. */
	extern int nanvix_mqueue_init(void);
	extern void nanvix_mqueue_finalize(void);
	extern int nanvix_mqueue_create(const char *, int, int, mode_t);
	extern int nanvix_mqueue_create_excl(const char *, int, int, mode_t);
	extern int nanvix_mqueue_open(const char *, int, int);
	extern int nanvix_mqueue_unlink(const char *);
	extern int nanvix_mqueue_close(int);
	extern int nanvix_mqueue_send(int, const char *, size_t, unsigned);
	extern ssize_t nanvix_mqueue_receive(int, char *, size_t, unsigned *);

#endif /* NANVIX_MQUEUE_H_ */
