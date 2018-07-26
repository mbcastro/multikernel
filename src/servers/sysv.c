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
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <nanvix/spawner.h>
#include <nanvix/syscalls.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>
#include <nanvix/semaphore.h>

/**
 * @brief Table of named semaphores.
 */
static struct semaphore semaphores[SEM_MAX];

/**
 * @brief Messages queue.
 */
static struct msg_element messages[NANVIX_PROC_MAX];

/**
 * @brief Tail of the messages queue.
 */
static int tail = 0;

/**
 * @brief Head of the messages queue.
 */
static int head = 0;

/*===================================================================*
 * _sem_init()                                                       *
 *===================================================================*/

/**
 * @brief Initializes the name server.
 */
static void _sem_init(void)
{
	/* Initialize semaphores table. */
	for (int i = 0; i < SEM_MAX; i++)
	{
		semaphores[i].nr_proc = 0;
		semaphores[i].flags = 0;
		semaphores[i].count = 0;
		strcpy(semaphores[i].name, "");

		for (int j = 0; j < NANVIX_PROC_MAX; j++)
			semaphores[i].queue[j].used = 0;

		semaphores[i].head = -1;
		semaphores[i].tail = -1;
	}

	/* Initilize message queue. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		messages[i].used = 0;

	head = -1;
	tail = -1;
}

/*=======================================================================*
 * _sem_proc_name_is_valid()                                             *
 *=======================================================================*/

/**
 * @brief Asserts whether or not a process name is valid.
 *
 * @param name Target name.
 *
 * @returns One if the target semaphore name is valid, and zero
 * otherwise.
 */
static int _sem_proc_name_is_valid(const char *name)
{
	return ((name != NULL) && (strlen(name) < (NANVIX_PROC_NAME_MAX - 1)) &&
														(strcmp(name, "")));
}

/*=======================================================================*
 * _sem_name_is_valid()                                                  *
 *=======================================================================*/

/**
 * @brief Asserts whether or not a semaphore name is valid.
 *
 * @param name Target name.
 *
 * @returns One if the target semaphore name is valid, and zero
 * otherwise.
 */
static int _sem_name_is_valid(const char *name)
{
	return ((name != NULL) && (strlen(name) < (NANVIX_SEM_NAME_MAX - 1)) &&
													   (strcmp(name, "")));
}

/*=======================================================================*
 * _sem_is_valid()                                                       *
 *=======================================================================*/

/**
 * @brief Asserts whether or not a semaphore Id is valid.
 *
 * @param semid Target semaphore Id.
 *
 * @returns One if the semaphore Id is valid, and zero otherwise.
 */
static int _sem_is_valid(int semid)
{
	return ((semid >= 0) && (semid < SEM_MAX));
}

/*=======================================================================*
 * _sem_is_used()                                                        *
 *=======================================================================*/

 /**
  * @brief Asserts whether or not a semaphore name is used.
  *
  * @param semid Target semaphore id.
  *
  * @returns One if the target semaphore name is used, and zero
  * otherwise.
  */
static int _sem_is_used(int semid)
{
	return (semaphores[semid].flags & SEM_USED);
}

/*=======================================================================*
 * _sem_set_used()                                                       *
 *=======================================================================*/

/**
 * @brief Set a semaphore as used.
 *
 * @param semid Target semaphore Id.
 */
static void _sem_set_used(int semid)
{
	semaphores[semid].flags |= SEM_USED;
}

/*=======================================================================*
 * _sem_clear_frags()                                                       *
 *=======================================================================*/

/**
 * @brief Clear semaphore flags.
 *
 * @param semid Target semaphore Id.
 */
static void _sem_clear_flags(int semid)
{
	semaphores[semid].flags = 0;
}

/*=======================================================================*
 * _sem_set_permission()                                                 *
 *=======================================================================*/

/**
 * @brief Set semaphore permission.
 *
 * @param semid Target semaphore Id.
 * @param mode Permission mode.
 */
static void _sem_set_permission(int semid, int mode)
{
	mode &= 0xf0000000;

	semaphores[semid].flags &= 0x0fffffff;

	/* Write the four lower bits of mode on the semaphore flags. */
	semaphores[semid].flags |= mode;
}

/*=======================================================================*
 * _sem_alloc()                                                          *
 *=======================================================================*/

/**
 * @brief Allocates a semaphore.
 *
 * @return Upon successful completion the ID of the newly allocated
 * semaphore is returned. Upon failure, a negative error code is returned
 * instead.
 */
static int _sem_alloc(void)
{
	/* Search for a free semaphore. */
	for (int i = 0; i < SEM_MAX; i++)
	{
		/* Found. */
		if (!_sem_is_used(i))
		{
			_sem_set_used(i);
			return (i);
		}
	}

	return (-ENOENT);
}

/*=======================================================================*
 * _sem_free()                                                           *
 *=======================================================================*/

/**
 * @brief Free a semaphore.
 *
 * @param semid Targeted semaphore Id
 */
static int _sem_free(int semid)
{
	/* Sanity check. */
	if (!_sem_is_valid(semid))
		return (-EINVAL);

	if (!_sem_is_used(semid))
		return (-EINVAL);

	_sem_clear_flags(semid);

	return (0);
}

/*=======================================================================*
 * _sem_put_message()                                                    *
 *=======================================================================*/

/**
 * @brief Enqueue a message.
 *
 * @param message Targeted message
 *
 * @return Upon successful completion zero is returned.
 * Upon failure, a negative error code is returned instead.
 */
static int _sem_put_message(struct sem_message *message)
{
	int i;   /* Place in the queue. */

	/* Invalid message. */
	if (message == NULL)
		return (-EINVAL);

	/* Search for a place in the queue. */
	for (i = 0; i < NANVIX_PROC_MAX; i++)
		if (messages[i].used == 0)
			goto found;

	/* The queue is full. */
	return (-EAGAIN);

found:

	messages[i].message.seq = message->seq;
	strcpy(messages[i].message.name, message->name);
	messages[i].message.op = message->op;
	messages[i].message.value = message->value;

	messages[i].next = head;
	head = i;

	messages[i].used = 1;

	return (0);
}

/*=======================================================================*
 * _sem_get_message()                                                    *
 *=======================================================================*/

/**
 * @brief Get a message.
 *
 * @param message Address where the message will be stored.
 * @param seq Sequence number of the targeted message.
 *
 * @return Upon successful completion zero is returned.
 * Upon failure, a negative error code is returned instead.
 */
static int _sem_get_message(struct sem_message *message, uint16_t seq)
{
	int pred; /* Predecessor. */
	int i;

	/* Invalid message. */
	if (message == NULL)
		return (-EINVAL);

	/* Invalid sequence number. */
	if ((seq & 1) == 1)
		return (-EINVAL);

	/* The queue should not be empty. */
	if (head == tail)
		return (-EAGAIN);

	/* Look for the message. */
	pred = head;
	i = head;
	while ((i != (-1)) && (messages[i].message.seq != seq))
	{
		pred = i;
		i = messages[i].next;
	}

	/* Message not found. */
	if (i == (-1))
		return (-EINVAL);

	/* Get message. */
	message->seq = messages[i].message.seq;
	strcpy(message->name, messages[i].message.name);
	message->op = messages[i].message.op;
	message->value = messages[i].message.value;

	/* Update chain. */
	messages[i].used = 0;
	messages[pred].next = messages[i].next;

	return (0);
}

/*=======================================================================*
 * _sem_enqueue()                                                        *
 *=======================================================================*/

/**
 * @brief Enqueue a process name.
 *
 * @param name Process name
 * @param semid Targeted semaphore Id
 *
 * @return Upon successful completion zero is returned.
 * Upon failure, a negative error code is returned instead.
 */
static int _sem_enqueue(char *name, int semid)
{
	int i;   /* Place in the queue. */

	/* Invalid name. */
	if (!_sem_proc_name_is_valid(name))
		return (-EINVAL);

	/* Invalid semaphore Id. */
	if (!_sem_is_valid(semid))
		return (-EINVAL);

	/* Search for a place in the queue. */
	for (i = 0; i < NANVIX_PROC_MAX; i++)
		if (semaphores[semid].queue[i].used == 0)
			goto found;

	/* The queue is full. */
	return (-EAGAIN);

found:

	strcpy(semaphores[semid].queue[i].process, name);
	semaphores[semid].queue[i].next = semaphores[semid].head;
	semaphores[semid].head = i;
	semaphores[semid].queue[i].used = 1;

	/* Set the tail. */
	if (semaphores[semid].tail == (-1))
		semaphores[semid].tail = i;

	return (0);
}

/*=======================================================================*
 * _sem_dequeue()                                                        *
 *=======================================================================*/

/**
 * @brief Dequeue a process name.
 *
 * @param name Address where the process name will be stored.
 * @param semid Targeted semaphore Id
 *
 * @return Upon successful completion zero is returned.
 * Upon failure, a negative error code is returned instead.
 */
static int _sem_dequeue(char *name, int semid)
{
	int pred;       /* Predecessor.       */
	int t;          /* Tail of the queue. */

	/* Invalid name. */
	if (!_sem_proc_name_is_valid(name))
		return (-EINVAL);

	/* Invalid semaphore Id. */
	if (!_sem_is_valid(semid))
		return (-EINVAL);

	t = semaphores[semid].tail;

	/* The queue should not be empty. */
	if (t == (-1))
		return (-EAGAIN);

	/* One element left. */
	if (semaphores[semid].head == semaphores[semid].tail)
	{
		strcpy(name, semaphores[semid].queue[t].process);

		semaphores[semid].head = (-1);
		semaphores[semid].tail = (-1);

		return (0);
	}

	/* Search for the predecessor of the tail. */
	pred = semaphores[semid].head;

	while ((pred >= 0) && (semaphores[semid].queue[pred].next != t))
		pred = semaphores[semid].queue[pred].next;

	semaphores[semid].queue[t].used = 0;

	strcpy(name, semaphores[semid].queue[t].process);

	semaphores[semid].tail = pred;

	return (0);
}

/*===================================================================*
 * _sem_open()                                                       *
 *===================================================================*/

/**
 * @brief Open a semaphore
 *
 * @param source Name of the client.
 * @param name Name of the targeted semaphore.
 *
 * @returns Upon successful completion, the semaphore Id is
 * returned. Upon failure, a negative error code is returned instead.
 */
static int _sem_open(char *source, char *name)
{
	int i;       /* Semaphore Id.      */
	int nr_proc; /* Number of process. */

	/* Invalid name. */
	if ((!_sem_proc_name_is_valid(source)) || (!_sem_name_is_valid(name)))
		return (-EINVAL);

	/* The semaphore should exist. */
	for (i = 0; i < SEM_MAX; i++)
		if ((_sem_is_used(i)) && (!strcmp(semaphores[i].name, name)))
			goto found;

	return (-EINVAL);

found:

	/* Is the semaphore already opened ? */
	for (int j = 0; j < NANVIX_PROC_MAX; j++)
		if (!strcmp(semaphores[i].processes[j].name, source))
			return (-EINVAL);

	nr_proc = semaphores[i].nr_proc;

	/* Add the process to the list. */
	if (nr_proc >= (NANVIX_PROC_MAX - 1))
		return (-EAGAIN);

	strcpy(semaphores[i].processes[nr_proc].name, source);

	semaphores[i].processes[nr_proc].use = 0;

	semaphores[i].nr_proc++;

	return (i);
}

/*===================================================================*
 * _sem_create()                                                     *
 *===================================================================*/

/**
 * @brief Create a semaphore
 *
 * @param source Name of the client.
 * @param name Name of the targeted semaphore.
 * @param mode	User permissions.
 * @param value	Semaphore count value.
 *
 * @returns Upon successful completion, the newly created semaphore Id is
 * returned. Upon failure, a negative error code is returned instead.
 */
static int _sem_create(char *source, char *name, int mode, int value)
{
	int i;       /* Semaphore Id.      */
	int nr_proc; /* Number of process. */

	/* Invalid name. */
	if ((!_sem_proc_name_is_valid(source)) || (!_sem_name_is_valid(name)))
		return (-EINVAL);

	/* Check if the semaphore already exist. */
	for (i = 0; i < SEM_MAX; i++)
		if ((_sem_is_used(i)) && (!strcmp(semaphores[i].name, name)))
			goto open;

	/* Invalid semaphore value. */
	if (value > SEM_VALUE_MAX)
		return (-EINVAL);

	/* Allocate a new semaphore. */
	i = _sem_alloc();

	/* Initialize the semaphore. */
	nr_proc = semaphores[i].nr_proc;

	semaphores[i].count = value;
	strcpy(semaphores[i].processes[nr_proc].name, source);
	semaphores[i].processes[nr_proc].use = 0;
	strcpy(semaphores[i].name, name);
	semaphores[i].nr_proc++;

	_sem_set_permission(i, mode);

	return (i);

open:

	return (_sem_open(source, name));
}

/*===================================================================*
 * _sem_create_exclusive()                                           *
 *===================================================================*/

/**
 * @brief Open a semaphore with existence check
 *
 * @param source Name of the client.
 * @param name Name of the targeted semaphore.
 * @param mode	User permissions.
 * @param value	Semaphore count value.
 *
 * @returns Upon successful completion, the newly created semaphore Id is
 * returned. Upon failure, a negative error code is returned instead.
 */
static int _sem_create_exclusive(char *source, char *name, int mode, int value)
{
	/* Invalid name. */
	if ((!_sem_proc_name_is_valid(source)) || (!_sem_name_is_valid(name)))
		return (-EINVAL);

	/* The semaphore should not already exist. */
	for (int i = 0; i < SEM_MAX; i++)
		if (!strcmp(semaphores[i].name, name))
			return (-1);

	return (_sem_create(source, name, mode, value));
}

/*===================================================================*
 * _sem_close()                                                      *
 *===================================================================*/

/**
 * @brief Close a semaphore
 *
 * @param semid Targeted semaphore.
 * @param source Name of the client.
 *
 * @returns Upon successful completion, 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
static int _sem_close(int semid, char *source)
{
	int i;
	int nr_proc;

	/* Sanity check. */
	if ((!_sem_is_valid(semid)) || (!_sem_proc_name_is_valid(source)))
		return (-EINVAL);

	/* The semaphore should be used. */
	if (!_sem_is_used(semid))
		return (-EINVAL);

	nr_proc = semaphores[semid].nr_proc;

	/* The process should have opened the semaphore. */
	for (i = 0; i < nr_proc; i++)
		if (!strcmp(semaphores[semid].processes[i].name, source))
			goto found;

	return (-EINVAL);

found:

	/* Remove the process from the list. */
	if (i != (NANVIX_PROC_MAX - 1))
		for (int j = i; j < nr_proc; j++)
			strcpy(semaphores[semid].processes[j].name,
			  semaphores[semid].processes[j + 1].name);

	semaphores[semid].nr_proc--;

	return (0);
}

/*===================================================================*
 * _sem_unlink()                                                     *
 *===================================================================*/

/**
 * @brief Unlink a semaphore
 *
 * @param name Targeted semaphore name.
 * @param source Name of the client.
 *
 * @returns Upon successful completion, 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
static int _sem_unlink(char *name, char *source)
{
	int r;     /* Return value. */
	int semid; /* Semaphore Id. */
	int i;

	/* Sanity check. */
	if ((!_sem_name_is_valid(name)) || (!_sem_proc_name_is_valid(source)))
		return (-EINVAL);

	/* Search for semaphore Id. */
	for (semid = 0; semid < SEM_MAX; semid++)
		if (!strcmp(semaphores[semid].name, name))
			goto found;

	return (-EAGAIN);

found:

	/* The semaphore should be used. */
	if (!_sem_is_used(semid))
		return (-EINVAL);

	/* Is the semaphore opened ? */
	for (i = 0; i < SEM_MAX; i++)
		if (!strcmp(semaphores[semid].processes[i].name, source))
			goto found2;

	goto notfound;

found2:

	/* Close the semaphore for the process. */
	if ((r = _sem_close(semid, source)) < 0)
		return (r);

notfound:

	/* Unlink semaphore if no process use it anymore. */
	if (semaphores[semid].nr_proc == 0)
		_sem_free(semid);

	return (0);
}

/*===================================================================*
 * _sem_wait()                                                       *
 *===================================================================*/

/**
 * @brief Wait a semaphore
 *
 * @param semid Targeted semaphore Id.
 * @param source Name of the client.
 *
 * @returns Upon successful completion, 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
static int _sem_wait(int semid, char *source)
{
	int i;

	/* Invalid name. */
	if (!_sem_proc_name_is_valid(source))
		return (-EINVAL);

	/* Invalid semaphore Id. */
	if (!_sem_is_valid(semid))
		return (-EINVAL);

	/* The process should have opened the semaphore. */
	for (i = 0; i < NANVIX_PROC_MAX; i++)
		if (!strcmp(semaphores[semid].processes[i].name, source))
			goto found;

	return (-EINVAL);

found:

	semaphores[semid].count--;

	/* Is there a ressource available ? */
	if (semaphores[semid].count >= 0)
	{
		semaphores[semid].processes[i].use++;

		return (SEM_SUCCESS);
	}
	else
	{
		/* Add the process in the queue. */
		if (_sem_enqueue(source, semid) != 0)
			return (-EAGAIN);

		/* Send wait signal to the process. */
		return (SEM_WAIT);
	}
}

/*===================================================================*
 * _sem_post()                                                       *
 *===================================================================*/

/**
 * @brief Post a semaphore
 *
 * @param semid Targeted semaphore Id.
 * @param source Name of the client.
 *
 * @returns Upon successful completion, 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
static int _sem_post(int semid, char *source)
{
	char name[NANVIX_PROC_NAME_MAX]; /* Name of the process to wake up. */
	int outbox;                      /* Mailbox for small messages.     */
	struct sem_message msg;          /* Semaphore message.              */
	int i;
	int j;

	/* Invalid name. */
	if (!_sem_proc_name_is_valid(source))
		return (-EINVAL);

	/* Invalid semaphore Id. */
	if (!_sem_is_valid(semid))
		return (-EINVAL);
	/* The process should have opened the semaphore. */
	for (i = 0; i < NANVIX_PROC_MAX; i++)
		if (!strcmp(semaphores[semid].processes[i].name, source))
			goto found;

	return (-EINVAL);

found:

	semaphores[semid].processes[i].use--;

	semaphores[semid].count++;

	/* Wake up a process. */
	if (semaphores[semid].count <= 0)
		goto wakeup;

	return (0);

wakeup:

	/* Get the process to wakeup. */
	if (_sem_dequeue(name, semid) != 0)
		return (-EAGAIN);

	for (j = 0; j < NANVIX_PROC_MAX; j++)
		if (!strcmp(semaphores[semid].processes[j].name, name))
			goto found2;

	return (-EAGAIN);

found2:

	/* Increment the number of ressource used by the woken up process. */
	semaphores[semid].processes[j].use++;

	/* Send wake up signal. */
	assert((outbox = mailbox_open(name)) >= 0);

	strcpy(msg.name, "/sem-server");
	msg.op = SEM_SUCCESS;

	assert(mailbox_write(outbox, &msg, sizeof(struct sem_message)) == 0);

	assert(mailbox_close(outbox) == 0);

	return (0);
}

/*===================================================================*
 * semaphore_server()                                                *
 *===================================================================*/

/**
 * @brief Handles remote semaphore requests.
 *
 * @param inbox    Input mailbox.
 * @param inportal Input portal.
 *
 * @returns Always returns NULL.
 */
int semaphore_server(int inbox, int inportal)
{
	int outbox;                            /* Mailbox for small messages. */
	int semid;                             /* Semaphore ID.               */
	struct sem_message msg1;               /* Semaphore message 1.        */
	struct sem_message msg2;               /* Semaphore message 2.        */

	((void) inportal);

	printf("[nanvix][semaphore] booting up server\n");

	/* Create input mailbox. */
	inbox = mailbox_create("/sem-server");

	_sem_init();

	printf("[nanvix][semaphore] server alive\n");

	spawner_ack();

	while(1)
	{
		assert(mailbox_read(inbox, &msg1, sizeof(struct sem_message)) == 0);

		/* Handle semaphore requests. */
		switch (msg1.op)
		{
			/* Create a semaphore. */
			case SEM_CREATE:
#ifdef DEBUG
				printf("SEM_CREATE name: %s.\n", msg1.name);
#endif
				/* First message. */
				if (!(msg1.seq & 1))
				{
					assert(_sem_put_message(&msg1) == 0);
				}
				/* Second message. */
				else
				{
					assert(_sem_get_message(&msg2, (msg1.seq & 0xfffe)) == 0);

					/* Sequence number check. */
					assert(msg1.seq == (msg2.seq | 1));

					semid = _sem_create(msg2.name, msg1.name, msg2.value, msg1.value);

					if ((msg1.value = semid) >= 0)
						msg1.op = SEM_SUCCESS;
					else
						msg1.op = SEM_FAILURE;

					/* Send response. */
					assert((outbox = sys_mailbox_open(atoi(msg2.name))) >= 0);

					assert(sys_mailbox_write(outbox, &msg1, sizeof(struct sem_message))
														== sizeof(struct sem_message));

					assert(sys_mailbox_close(outbox) == 0);
				}

				break;

			/* Create a semaphore with existence check. */
			case SEM_CREATE_EXCL:
#ifdef DEBUG
				printf("SEM_CREATE_EXCL name: %s.\n", msg1.name);
#endif
				/* First message. */
				if (!(msg1.seq & 1))
				{
					assert(_sem_put_message(&msg1) == 0);
				}
				/* Second message. */
				else
				{
					assert(_sem_get_message(&msg2, (msg1.seq & 0xfffe)) == 0);

					/* Sequence number check. */
					assert(msg1.seq == (msg2.seq | 1));

					semid = _sem_create_exclusive(msg2.name, msg1.name, msg2.value, msg1.value);

					if ((msg1.value = semid) >= 0)
						msg1.op = SEM_SUCCESS;
					else
						msg1.op = SEM_FAILURE;

					/* Send response. */
					assert((outbox = sys_mailbox_open(atoi(msg2.name))) >= 0);

					assert(sys_mailbox_write(outbox, &msg1, sizeof(struct sem_message))
														== sizeof(struct sem_message));

					assert(sys_mailbox_close(outbox) == 0);
				}

				break;

			/* Open a semaphore. */
			case SEM_OPEN:
#ifdef DEBUG
				printf("SEM_OPEN name: %s.\n", msg1.name);
#endif
				/* First message. */
				if (!(msg1.seq & 1))
				{
					assert(_sem_put_message(&msg1) == 0);
				}
				/* Second message. */
				else
				{
					assert(_sem_get_message(&msg2, (msg1.seq & 0xfffe)) == 0);

					/* Sequence number check. */
					assert(msg1.seq == (msg2.seq | 1));

					semid = _sem_open(msg2.name, msg1.name);

					if ((msg1.value = semid) >= 0)
						msg1.op = SEM_SUCCESS;
					else
						msg1.op = SEM_FAILURE;

					/* Send response. */
					assert((outbox = sys_mailbox_open(atoi(msg2.name))) >= 0);

					assert(sys_mailbox_write(outbox, &msg1, sizeof(struct sem_message))
														== sizeof(struct sem_message));

					assert(sys_mailbox_close(outbox) == 0);
				}

				break;

			/* Close a semaphore. */
			case SEM_CLOSE:
#ifdef DEBUG
				printf("SEM_CLOSE name: %s.\n", msg1.name);
#endif

				if ((msg1.value = _sem_close(msg1.value, msg1.name)) >= 0)
					msg1.op = SEM_SUCCESS;
				else
					msg1.op = SEM_FAILURE;

				/* Send response. */
				assert((outbox = sys_mailbox_open(atoi(msg1.name))) >= 0);

				assert(sys_mailbox_write(outbox, &msg1, sizeof(struct sem_message))
													== sizeof(struct sem_message));

				assert(sys_mailbox_close(outbox) == 0);

				break;

			/* Unlink a semaphore. */
			case SEM_UNLINK:
#ifdef DEBUG
				printf("SEM_UNLINK name: %s.\n", msg1.name);
#endif
				/* First message. */
				if (!(msg1.seq & 1))
				{
					assert(_sem_put_message(&msg1) == 0);
				}
				/* Second message. */
				else
				{
					assert(_sem_get_message(&msg2, (msg1.seq & 0xfffe)) == 0);

					/* Sequence number check. */
					assert(msg1.seq == (msg2.seq | 1));

					if ((msg1.value = _sem_unlink(msg1.name, msg2.name)) >= 0)
						msg1.op = SEM_SUCCESS;
					else
						msg1.op = SEM_FAILURE;

					/* Send response. */
					assert((outbox = sys_mailbox_open(atoi(msg2.name))) >= 0);

					assert(sys_mailbox_write(outbox, &msg1, sizeof(struct sem_message))
														== sizeof(struct sem_message));

					assert(sys_mailbox_close(outbox) == 0);
				}

				break;

			/* Wait a semaphore. */
			case SEM_WAIT:
#ifdef DEBUG
				printf("SEM_WAIT name: %s.\n", msg1.name);
#endif

				msg1.op = _sem_wait(msg1.value, msg1.name);

				/* Send response. */
				assert((outbox = sys_mailbox_open(atoi(msg1.name))) >= 0);

				assert(sys_mailbox_write(outbox, &msg1, sizeof(struct sem_message))
													== sizeof(struct sem_message));

				assert(sys_mailbox_close(outbox) == 0);

				break;

			/* Post a semaphore. */
			case SEM_POST:
#ifdef DEBUG
				printf("SEM_POST name: %s.\n", msg1.name);
#endif

				msg1.op = _sem_post(msg1.value, msg1.name);

				/* Send response. */
				assert((outbox = sys_mailbox_open(atoi(msg1.name))) >= 0);

				assert(sys_mailbox_write(outbox, &msg1, sizeof(struct sem_message))
													== sizeof(struct sem_message));

				assert(sys_mailbox_close(outbox) == 0);

				break;

			/* Should not happen. */
			default:
				break;
		}
	}

	/* House keeping. */
	mailbox_unlink(inbox);

	return (EXIT_SUCCESS);
}
