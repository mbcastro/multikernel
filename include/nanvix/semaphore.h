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

#ifndef NANVIX_SEM_H_
#define NANVIX_SEM_H_

	#include <stdint.h>
	#include <fcntl.h>

	#include <nanvix/limits.h>

	/**
	 * @brief Maximal number of sempahores in the system.
	 */
	#define SEM_MAX 50

	/**
	 * @brief Maximal semaphore count value.
	 */
	#define SEM_VALUE_MAX 50

	/**
	 * @brief Operation types for semaphore server.
	 */
	/**@{*/
	#define SEM_OPEN        1 /* Open a semaphore.                        */
	#define SEM_POST        2 /* Post a semaphore.                        */
	#define SEM_WAIT        3 /* Wait a semaphore.                        */
	#define SEM_CLOSE       4 /* Close a semaphore.                       */
	#define SEM_UNLINK      5 /* Unlink a semaphore.                      */
	#define SEM_CREATE      6 /* Create a semaphore.                      */
	#define SEM_CREATE_EXCL 7 /* Create a semaphore with existence check. */
	/**@}*/

	/**
	 * @brief Semaphore message.
	 */
	struct sem_message
	{
		uint16_t seq;                   /**< Sequence number.          */
		char name[NANVIX_SEM_NAME_MAX]; /**< Client or semaphore name. */
		int16_t op;                     /**< Semaphore operation.      */
		int value;                      /**< Value.                    */
	};

	/* Forward definitions. */
	extern int nanvix_sem_create(const char *, mode_t, unsigned, int);
	extern int nanvix_sem_open(const char *name);
	extern int nanvix_sem_post(int);
	extern int nanvix_sem_wait(int);
	extern int nanvix_sem_close(int);
	extern int nanvix_sem_unlink(const char *);

#endif /* _SEM_H_ */
