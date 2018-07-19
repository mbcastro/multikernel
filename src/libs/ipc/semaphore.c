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

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>

#define __NEED_HAL_NOC_
#define __NEED_HAL_CORE_

#include <nanvix/hal.h>
#include <nanvix/semaphore.h>
#include <nanvix/pm.h>

/**
 * @brief Mailboxe for small messages.
 */
static int server;

/**
 * @brief Is the semaphore service initialized ?
 */
static int initialized = 0;

/**
 * @brief Mailbox module lock.
 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*============================================================================*
 * sem_init()                                                                 *
 *============================================================================*/

/**
 * @brief Initializes the semaphore client.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int sem_init(void)
{
	char server_name[NANVIX_PROC_NAME_MAX];

	/* Nothing to do. */
	if (initialized)
		return (0);

	sprintf(server_name, "/sem-server");

	server = mailbox_open(server_name);

	if (server >= 0)
	{
		initialized = 1;
		return (0);
	}

	return (-1);
}

/*============================================================================*
 * sem_finalize()                                                             *
 *============================================================================*/

/**
 * @brief Closes the semaphore client.
 */
void sem_finalize(void)
{
	/* Nothing to do. */
	if (!initialized)
		return;

	mailbox_close(server);

	initialized = 0;
}

/*=======================================================================*
 * sem_name_is_valid()                                                   *
 *=======================================================================*/

/**
 * @brief Asserts whether or not a name is valid.
 *
 * @param name	Target name.
 *
 * @returns One if the target semaphore name is valid, and false
 * otherwise.
 */
static int sem_name_is_valid(const char *name)
{
	return ((name != NULL) && (strlen(name) < (NANVIX_SEM_NAME_MAX - 1)) &&
													   (strcmp(name, "")));
}

/*=======================================================================*
 * sem_is_valid()                                                        *
 *=======================================================================*/

/**
 * @brief Asserts whether or not a semaphore is valid.
 *
 * @param name	Target semaphore.
 *
 * @returns One if the target semaphore is valid, and false
 * otherwise.
 */
static int sem_is_valid(int sem)
{
	return ((sem >= 0) && (sem < SEM_MAX));
}

/*============================================================================*
 * nanvix_sem_open()                                                          *
 *============================================================================*/

/**
 * @brief Open a semaphore.
 *
 * @param name	Target name.
 * @param oflag	Creation flags.
 * @param mode	User permissions.
 * @param value	Semaphore count value.
 *
 * @returns Upon successful completion, the semaphore is
 * returned. Upon failure, a negative error code is returned instead.
 */
int nanvix_sem_open(const char *name, int oflag, ...)
{
	int mode;                                /* Creation mode.               */
	int value;                               /* semaphore value.             */
	int nodeid;                              /* NoC node ID.                 */
	va_list ap;                              /* Arguments pointer.           */
	struct sem_message msg1;                 /* Semaphore message 1.         */
	struct sem_message msg2;                 /* Semaphore message 2.         */
	char process_name[NANVIX_PROC_NAME_MAX]; /* Name of the running process. */
	int inbox;                               /* Mailbox for small messages.  */

	if (!sem_name_is_valid(name))
		return (-EINVAL);

	/* Initilize semaphore client. */
	if (!initialized)
		return (-EAGAIN);

	nodeid = hal_get_node_id();

	if (get_name(process_name) != 0)
		return (-EAGAIN);

	if ((inbox = get_named_inbox()) < 0)
		return (-EAGAIN);

	/* Semaphore creation operation. */
	if (oflag & O_CREAT)
	{
		/* Retrieve arguments. */
		va_start(ap, oflag);
		mode = va_arg(ap, int);
		value = va_arg(ap, int);
		va_end(ap);

		/* Invalid semaphore value. */
		if (value > SEM_VALUE_MAX)
			return (-EINVAL);

		/* Build operation header 1. */
		msg1.seq = ((nodeid << 4) | 0);
		strcpy(msg1.name, process_name);

		/* Is the creation exclusive ? */
		if (oflag & O_EXCL)
			msg1.op = SEM_CREATE_EXCL;
		else
			msg1.op = SEM_CREATE;

		msg1.value = mode;

		/* Build operation header 2. */
		msg2.seq = ((nodeid << 4) | 1);
		strcpy(msg2.name, name);

		/* Is the creation exclusive ? */
		if (oflag & O_EXCL)
			msg2.op = SEM_CREATE_EXCL;
		else
			msg2.op = SEM_CREATE;

		msg2.value = value;
	}
	/* Semaphore opening operation. */
	else
	{
		/* Build operation header 1. */
		msg1.seq = ((nodeid << 4) | 0);
		strcpy(msg1.name, process_name);
		msg1.op = SEM_OPEN;
		msg1.value = -1;

		/* Build operation header 2. */
		msg2.seq = ((nodeid << 4) | 1);
		strcpy(msg2.name, name);
		msg2.op = SEM_OPEN;
		msg2.value = 0;
	}

	pthread_mutex_lock(&lock);

	if (mailbox_write(server, &msg1, sizeof(struct sem_message)) != 0)
		goto error;

	if (mailbox_write(server, &msg2, sizeof(struct sem_message)) != 0)
		goto error;

	if (mailbox_read(inbox, &msg1, sizeof(struct sem_message)) != 0)
		goto error;

	pthread_mutex_unlock(&lock);

	if (msg1.value < 0)
	{
		errno = msg1.value;
		return (SEM_FAILURE);
	}

	return (msg1.value);

error:

	pthread_mutex_unlock(&lock);

	return (-EAGAIN);
}

/*============================================================================*
 * nanvix_sem_post()                                                          *
 *============================================================================*/

/**
 * @brief Post a semaphore.
 *
 * @param sem	Target semaphore.
 *
 * @returns Upon successful completion, 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
int nanvix_sem_post(int sem)
{
	struct sem_message msg;                  /* Semaphore message.           */
	int nodeid;                              /* NoC node ID.                 */
	char process_name[NANVIX_PROC_NAME_MAX]; /* Name of the running process. */
	int inbox;                               /* Mailbox for small messages.  */

	if (!sem_is_valid(sem))
		return (-EINVAL);

	/* Initilize semaphore client. */
	if (!initialized)
		return (-EAGAIN);

	nodeid = hal_get_node_id();

	if (get_name(process_name) != 0)
		return (-EAGAIN);

	if ((inbox = get_named_inbox()) < 0)
		return (-EAGAIN);

	/* Build operation header. */
	msg.seq = ((nodeid << 4) | 0);
	strcpy(msg.name, process_name);
	msg.op = SEM_POST;
	msg.value = sem;

	pthread_mutex_lock(&lock);

	if (mailbox_write(server, &msg, sizeof(struct sem_message)) != 0)
		goto error;

	if (mailbox_read(inbox, &msg, sizeof(struct sem_message)) != 0)
		goto error;

	pthread_mutex_unlock(&lock);

	if (msg.op < 0)
	{
		errno = msg.op;
		return (SEM_FAILURE);
	}

	return (msg.op);

error:

	pthread_mutex_unlock(&lock);

	return (-EAGAIN);
}

/*============================================================================*
 * nanvix_sem_wait()                                                          *
 *============================================================================*/

/**
 * @brief Wait a semaphore.
 *
 * @param sem	Target semaphore.
 *
 * @returns Upon successful completion, 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
int nanvix_sem_wait(int sem)
{
	struct sem_message msg;                  /* Semaphore message.           */
	int nodeid;                              /* NoC node ID.                 */
	char process_name[NANVIX_PROC_NAME_MAX]; /* Name of the running process. */
	int inbox;                               /* Mailbox for small messages.  */

	if (!sem_is_valid(sem))
		return (-EINVAL);

	/* Initilize semaphore client. */
	if (!initialized)
		return (-EAGAIN);

	nodeid = hal_get_node_id();

	if (get_name(process_name) != 0)
		return (-EAGAIN);

	if ((inbox = get_named_inbox()) < 0)
		return (-EAGAIN);

	/* Build operation header. */
	msg.seq = ((nodeid << 4) | 0);
	strcpy(msg.name, process_name);
	msg.op = SEM_WAIT;
	msg.value = sem;

	pthread_mutex_lock(&lock);

	if (mailbox_write(server, &msg, sizeof(struct sem_message)) != 0)
		goto error;

	if (mailbox_read(inbox, &msg, sizeof(struct sem_message)) != 0)
		goto error;

	/* Wait for a semaphore ressource. */
	if (msg.op == SEM_WAIT)
	{
		while (msg.op != SEM_SUCCESS)
			if (mailbox_read(get_named_inbox(), &msg, sizeof(struct sem_message)) != 0)
				goto error;

	}
	else if (msg.op == SEM_SUCCESS)
	{
		pthread_mutex_unlock(&lock);
	}

	if (msg.op < 0)
	{
		errno = msg.op;
		return (SEM_FAILURE);
	}

	return (msg.op);

error:

	pthread_mutex_unlock(&lock);

	return (-EAGAIN);
}

/*============================================================================*
 * nanvix_sem_close()                                                         *
 *============================================================================*/

/**
 * @brief Close a semaphore.
 *
 * @param sem	Target semaphore.
 *
 * @returns Upon successful completion, 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
int nanvix_sem_close(int sem)
{
	struct sem_message msg;                  /* Semaphore message.           */
	int nodeid;                              /* NoC node ID.                 */
	char process_name[NANVIX_PROC_NAME_MAX]; /* Name of the running process. */
	int inbox;                               /* Mailbox for small messages.  */

	if (!sem_is_valid(sem))
		return (-EINVAL);

	/* Initilize semaphore client. */
	if (!initialized)
		return (-EAGAIN);

	nodeid = hal_get_node_id();

	if (get_name(process_name) != 0)
		return (-EAGAIN);

	if ((inbox = get_named_inbox()) < 0)
		return (-EAGAIN);

	/* Build operation header. */
	msg.seq = ((nodeid << 4) | 0);
	strcpy(msg.name, process_name);
	msg.op = SEM_CLOSE;
	msg.value = sem;

	pthread_mutex_lock(&lock);

	if (mailbox_write(server, &msg, sizeof(struct sem_message)) != 0)
		goto error;

	if (mailbox_read(inbox, &msg, sizeof(struct sem_message)) != 0)
		goto error;

	pthread_mutex_unlock(&lock);

	if (msg.op < 0)
	{
		errno = msg.op;
		return (SEM_FAILURE);
	}

	return (msg.op);

error:

	pthread_mutex_unlock(&lock);

	return (-EAGAIN);
}

/*============================================================================*
 * nanvix_sem_unlink()                                                        *
 *============================================================================*/

/**
 * @brief Unlink a semaphore.
 *
 * @param name	Target semaphore name.
 *
 * @returns Upon successful completion, 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
int nanvix_sem_unlink(const char *name)
{
	struct sem_message msg1;                 /* Semaphore message 1.         */
	struct sem_message msg2;                 /* Semaphore message 2.         */
	int nodeid;                              /* NoC node ID.                 */
	char process_name[NANVIX_PROC_NAME_MAX]; /* Name of the running process. */
	int inbox;                               /* Mailbox for small messages.  */

	if (!sem_name_is_valid(name))
		return (-EINVAL);

	/* Initilize semaphore client. */
	if (!initialized)
		return (-EAGAIN);

	nodeid = hal_get_node_id();

	if (get_name(process_name) != 0)
		return (-EAGAIN);

	if ((inbox = get_named_inbox()) < 0)
		return (-EAGAIN);

	/* Build operation header 1. */
	msg1.seq = ((nodeid << 4) | 0);
	strcpy(msg1.name, process_name);
	msg1.op = SEM_UNLINK;
	msg1.value = -1;

	/* Build operation header 2. */
	msg2.seq = ((nodeid << 4) | 1);
	strcpy(msg2.name, name);
	msg2.op = SEM_UNLINK;
	msg2.value = -1;

	pthread_mutex_lock(&lock);

	if (mailbox_write(server, &msg1, sizeof(struct sem_message)) != 0)
		goto error;

	if (mailbox_write(server, &msg2, sizeof(struct sem_message)) != 0)
		goto error;

	if (mailbox_read(inbox, &msg1, sizeof(struct sem_message)) != 0)
		goto error;

	pthread_mutex_unlock(&lock);

	if (msg1.value < 0)
	{
		errno = msg1.value;
		return (SEM_FAILURE);
	}

	return (msg1.value);

error:

	pthread_mutex_unlock(&lock);

	return (-EAGAIN);
}
