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

#include <nanvix/semaphore.h>
#include <nanvix/syscalls.h>
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
 * nanvix_sem_create()                                                        *
 *============================================================================*/

/**
 * @see nanvix_sem_create()
 */
static inline int _nanvix_sem_create(const char *name, mode_t mode, unsigned value, int excl)
{
	int ret;                             /* Return value.                */
	int inbox;                           /* Mailbox for small messages.  */
	int nodenum;                         /* NoC node number.             */
	struct sem_message msg1;             /* Semaphore message 1.         */
	struct sem_message msg2;             /* Semaphore message 2.         */
	char procname[NANVIX_PROC_NAME_MAX]; /* Name of the running process. */

	if (get_name(procname) != 0)
		return (-EAGAIN);

	if ((inbox = get_named_inbox()) < 0)
		return (-EAGAIN);

	nodenum = sys_get_node_num();

	/* Build message 1. */
	msg1.seq = ((nodenum << 4) | 0);
	strcpy(msg1.name, procname);
	msg1.op = (excl) ? SEM_CREATE_EXCL : SEM_CREATE;
	msg1.value = mode;

	/* Build operation header 2. */
	msg2.seq = ((nodenum << 4) | 1);
	strcpy(msg2.name, name);
	msg2.op = (excl) ? SEM_CREATE_EXCL : SEM_CREATE;
	msg2.value = value;

	pthread_mutex_lock(&lock);

		if ((ret = mailbox_write(server, &msg1, sizeof(struct sem_message))) != 0)
			goto error;

		if ((ret = mailbox_write(server, &msg2, sizeof(struct sem_message))) != 0)
			goto error;

		if ((ret = mailbox_read(inbox, &msg1, sizeof(struct sem_message))) != 0)
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg1.value);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/**
 * @brief Creates a named semaphore.
 *
 * @param name	Target name.
 * @param mode	User permissions.
 * @param value	Semaphore count value.
 * @param excl  Exclusive creation?
 *
 * @returns Upon successful completion, the ID of the newly created
 * semaphore is returned. Upon failure, a negative error code is
 * returned instead.
 */
int nanvix_sem_create(const char *name, mode_t mode, unsigned value, int excl)
{
	/* Invalid name. */
	if ((name == NULL) || (!strcmp(name, "")))
		return (-EINVAL);

	/* Name too long. */
	if (strlen(name) >= (NANVIX_SEM_NAME_MAX))
		return (-ENAMETOOLONG);

	/* Invalid semaphore value. */
	if (value > SEM_VALUE_MAX)
		return (-EINVAL);

	/* Initilize semaphore client. */
	if (!initialized)
		return (-EAGAIN);

	return (_nanvix_sem_create(name, mode, value, excl));
}

/*============================================================================*
 * nanvix_sem_open ()                                                         *
 *============================================================================*/

/**
 * @see nanvix_sem_open().
 */
static inline int _nanvix_sem_open(const char *name)
{
	int ret;                             /* Return value.                */
	int inbox;                           /* Mailbox for small messages.  */
	int nodenum;                         /* NoC node number.             */
	struct sem_message msg1;             /* Semaphore message 1.         */
	struct sem_message msg2;             /* Semaphore message 2.         */
	char procname[NANVIX_PROC_NAME_MAX]; /* Name of the running process. */

	if (get_name(procname) != 0)
		return (-EAGAIN);

	if ((inbox = get_named_inbox()) < 0)
		return (-EAGAIN);

	nodenum = sys_get_node_num();

	/* Build operation header 1. */
	msg1.seq = ((nodenum << 4) | 0);
	strcpy(msg1.name, procname);
	msg1.op = SEM_OPEN;
	msg1.value = -1;

	/* Build operation header 2. */
	msg2.seq = ((nodenum << 4) | 1);
	strcpy(msg2.name, name);
	msg2.op = SEM_OPEN;
	msg2.value = 0;

	pthread_mutex_lock(&lock);

		if ((ret = mailbox_write(server, &msg1, sizeof(struct sem_message))) != 0)
			goto error;

		if ((ret = mailbox_write(server, &msg2, sizeof(struct sem_message))) != 0)
			goto error;

		if ((ret = mailbox_read(inbox, &msg1, sizeof(struct sem_message))) != 0)
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg1.value);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/**
 * @brief Opens a named semaphore.
 *
 * @param name	Target name.
 * @param mode	User permissions.
 * @param value	Semaphore count value.
 * @param excl  Exclusive creation?
 *
 * @returns Upon successful completion, the ID of the target semaphore
 * is returned. Upon failure, a negative error code is returned
 * instead.
 */
int nanvix_sem_open(const char *name)
{
	/* Invalid name. */
	if ((name == NULL) || (!strcmp(name, "")))
		return (-EINVAL);

	/* Name too long. */
	if (strlen(name) >= (NANVIX_SEM_NAME_MAX))
		return (-ENAMETOOLONG);

	/* Initilize semaphore client. */
	if (!initialized)
		return (-EAGAIN);

	return (_nanvix_sem_open(name));
}

/*============================================================================*
 * nanvix_sem_post()                                                          *
 *============================================================================*/

/**
 * @see nanvix_set_post()
 */
static inline int _nanvix_sem_post(int sem)
{
	int ret;                                 /* Return value.                */
	struct sem_message msg;                  /* Semaphore message.           */
	int nodenum;                             /* NoC node number.             */
	char process_name[NANVIX_PROC_NAME_MAX]; /* Name of the running process. */
	int inbox;                               /* Mailbox for small messages.  */

	nodenum = sys_get_node_num();

	if (get_name(process_name) != 0)
		return (-EAGAIN);

	if ((inbox = get_named_inbox()) < 0)
		return (-EAGAIN);

	/* Build operation header. */
	msg.seq = ((nodenum << 4) | 0);
	strcpy(msg.name, process_name);
	msg.op = SEM_POST;
	msg.value = sem;

	pthread_mutex_lock(&lock);

		if ((ret = mailbox_write(server, &msg, sizeof(struct sem_message))) != 0)
			goto error;

		if ((ret = mailbox_read(inbox, &msg, sizeof(struct sem_message))) != 0)
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg.op);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/**
 * @brief Post on a named semaphore.
 *
 * @param sem Target semaphore.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int nanvix_sem_post(int sem)
{
	/* Invalid semaphore. */
	if (!sem_is_valid(sem))
		return (-EINVAL);

	/* Initilize semaphore client. */
	if (!initialized)
		return (-EAGAIN);
	
	return (_nanvix_sem_post(sem));
}

/*============================================================================*
 * nanvix_sem_wait()                                                          *
 *============================================================================*/

/**
 * @see nanvix_sem_wait()
 */
static inline int _nanvix_sem_wait(int sem)
{
	int ret;                                 /* Return value.                */
	struct sem_message msg;                  /* Semaphore message.           */
	int nodenum;                             /* NoC node number.             */
	char process_name[NANVIX_PROC_NAME_MAX]; /* Name of the running process. */
	int inbox;                               /* Mailbox for small messages.  */

	nodenum = sys_get_node_num();

	if (get_name(process_name) != 0)
		return (-EAGAIN);

	if ((inbox = get_named_inbox()) < 0)
		return (-EAGAIN);

	/* Build operation header. */
	msg.seq = ((nodenum << 4) | 0);
	strcpy(msg.name, process_name);
	msg.op = SEM_WAIT;
	msg.value = sem;

	pthread_mutex_lock(&lock);

		if ((ret = mailbox_write(server, &msg, sizeof(struct sem_message))) != 0)
			goto error;

		do
		{
			if (mailbox_read(inbox, &msg, sizeof(struct sem_message)) != 0)
				goto error;
		} while (msg.op == SEM_WAIT);

	pthread_mutex_unlock(&lock);

	return (msg.op);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/**
 * @brief Wait a named semaphore.
 *
 * @param sem Target semaphore.
 *
 * @returns Upon successful completion, zero is returned.  Upon
 * failure, a negative error code is returned instead.
 */
int nanvix_sem_wait(int sem)
{
	/* Invalid semaphore. */
	if (!sem_is_valid(sem))
		return (-EINVAL);

	/* Initilize semaphore client. */
	if (!initialized)
		return (-EAGAIN);

	return (nanvix_sem_wait(sem));
}

/*============================================================================*
 * nanvix_sem_close()                                                         *
 *============================================================================*/

/**
 * @see nanvix_sem_close()
 */
static inline int _nanvix_sem_close(int sem)
{
	int ret;                                 /* Return value.                */
	struct sem_message msg;                  /* Semaphore message.           */
	int nodenum;                             /* NoC node number.             */
	char process_name[NANVIX_PROC_NAME_MAX]; /* Name of the running process. */
	int inbox;                               /* Mailbox for small messages.  */

	nodenum = sys_get_node_num();

	if (get_name(process_name) != 0)
		return (-EAGAIN);

	if ((inbox = get_named_inbox()) < 0)
		return (-EAGAIN);

	/* Build operation header. */
	msg.seq = ((nodenum << 4) | 0);
	strcpy(msg.name, process_name);
	msg.op = SEM_CLOSE;
	msg.value = sem;

	pthread_mutex_lock(&lock);

		if ((ret = mailbox_write(server, &msg, sizeof(struct sem_message))) != 0)
			goto error;

		if ((ret = mailbox_read(inbox, &msg, sizeof(struct sem_message))) != 0)
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg.op);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/**
 * @brief Close a named semaphore.
 *
 * @param sem	Target semaphore.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
int nanvix_sem_close(int sem)
{
	/* Invalid semaphore. */
	if (!sem_is_valid(sem))
		return (-EINVAL);

	/* Initilize semaphore client. */
	if (!initialized)
		return (-EAGAIN);

	return (_nanvix_sem_close(sem));
}

/*============================================================================*
 * nanvix_sem_unlink()                                                        *
 *============================================================================*/
/**
 * @see nanvix_sem_unlink()
 */
static inline int _nanvix_sem_unlink(const char *name)
{
	int ret;                                 /* Return value.                */
	struct sem_message msg1;                 /* Semaphore message 1.         */
	struct sem_message msg2;                 /* Semaphore message 2.         */
	int nodenum;                             /* NoC node number.             */
	char process_name[NANVIX_PROC_NAME_MAX]; /* Name of the running process. */
	int inbox;                               /* Mailbox for small messages.  */

	nodenum = sys_get_node_num();

	if (get_name(process_name) != 0)
		return (-EAGAIN);

	if ((inbox = get_named_inbox()) < 0)
		return (-EAGAIN);

	/* Build operation header 1. */
	msg1.seq = ((nodenum << 4) | 0);
	strcpy(msg1.name, process_name);
	msg1.op = SEM_UNLINK;
	msg1.value = -1;

	/* Build operation header 2. */
	msg2.seq = ((nodenum << 4) | 1);
	strcpy(msg2.name, name);
	msg2.op = SEM_UNLINK;
	msg2.value = -1;

	pthread_mutex_lock(&lock);

		if ((ret = mailbox_write(server, &msg1, sizeof(struct sem_message)) != 0))
			goto error;

		if ((ret = mailbox_write(server, &msg2, sizeof(struct sem_message)) != 0))
			goto error;

		if ((ret = mailbox_read(inbox, &msg1, sizeof(struct sem_message)) != 0))
			goto error;

	pthread_mutex_unlock(&lock);

	return (msg1.value);

error:
	pthread_mutex_unlock(&lock);
	return (ret);
}

/**
 * @brief Unlinks a named semaphore.
 *
 * @param name	Target semaphore name.
 *
 * @returns Upon successful completion, 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
int nanvix_sem_unlink(const char *name)
{
	/* Invalid name. */
	if ((name == NULL) || (!strcmp(name, "")))
		return (-ENOENT);

	/* Name too long. */
	if (strlen(name) >= (NANVIX_SEM_NAME_MAX))
		return (-ENAMETOOLONG);

	/* Initilize semaphore client. */
	if (!initialized)
		return (-EAGAIN);

	return (_nanvix_sem_unlink(name));
}
