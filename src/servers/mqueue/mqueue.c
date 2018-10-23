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
#include <sys/stat.h>
#include <string.h>

#include <nanvix/mqueue.h>

/**
 * @brief Maximum number of message mqueues.
 */
#define MQUEUE_MAX 128

/**
 * @brief Message queue flags.
 */
/**@{*/
#define MQUEUE_USED   (1 << 0) /**< Used?   */
#define MQUEUE_REMOVE (1 << 1) /**< Remove? */
/**@}*/

/**
 * @brief Table of message mqueues.
 */
static struct
 {
	char name[NANVIX_MQUEUE_NAME_MAX]; /**< Name.                    */
	int flags;                         /**< Flags.                   */
	int owner;                         /**< ID of owner process.     */
	int refcount;                      /**< Number of references.    */
	mode_t mode;                       /**< Access permissions.      */
	size_t size;                       /**< Message size (in bytes). */

	int nstored;                       /**< Number of stored msgs.   */

	/**
	 * Table of stored messages.
	 */
	struct {
		unsigned prio;                 /**< Priority.                */
		char msg[MQUEUE_MESSAGE_SIZE]; /**< Message.                 */
	} messages[MQUEUE_MESSAGE_MAX];
} mqueues[MQUEUE_MAX];

/*============================================================================*
 * mqueue_is_valid()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a message queue ID is valid.
 *
 * @param mqueueid ID of the target message queue.
 *
 * @returns Non-zero if the message queue valid, and zero otherwise.
 */
static inline int mqueue_is_valid(int mqueueid)
{
	return ((mqueueid >= 0) && (mqueueid < MQUEUE_MAX));
}

/*============================================================================*
 * mqueue_is_used()                                                           *
 *============================================================================*/

/**
 * @brief Asserts whether or not a message queue is used.
 *
 * @param mqueueid ID of the target message queue.
 *
 * @returns Non-zero if the message queue is marked as used,
 * and zero otherwise.
 */
int mqueue_is_used(int mqueueid)
{
	return (mqueue_is_valid(mqueueid) && (mqueues[mqueueid].flags & MQUEUE_USED));
}

/*============================================================================*
 * mqueue_is_remove()                                                         *
 *============================================================================*/

/**
 * @brief Asserts whether or not a message queue is remove.
 *
 * @param mqueueid ID of the target message queue.
 *
 * @returns Non-zero if the message queue is marked to be
 * removed, and zero otherwise.
 */
int mqueue_is_remove(int mqueueid)
{
	return (mqueues[mqueueid].flags & MQUEUE_REMOVE);
}

/*============================================================================*
 * mqueue_is_owner()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a given node owns a given shared
 * memory region.
 *
 * @param mqueueid Target message queue.
 * @param node  Target node.
 *
 * @returns Non-zero if the target message queue is owned by
 * the target node, and zero otherwise.
 */
int mqueue_is_owner(int mqueueid, int node)
{
	return (mqueues[mqueueid].owner == node);
}

/*============================================================================*
 * mqueue_is_readable()                                                       *
 *============================================================================*/

/**
 * @brief Asserts whether or not a given message queue has read
 * permission.
 *
 * @param mqueueid Target message queue.
 *
 * @returns Non-zero if the target message queue is readable,
 * and zero otherwise.
 */
int mqueue_is_readable(int mqueueid)
{
	return (mqueues[mqueueid].mode & S_IRUSR);
}

/*============================================================================*
 * mqueue_is_writable()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a given message queue has
 * write permission.
 *
 * @param mqueueid Target message queue.
 *
 * @returns Non-zero if the target message queue is writable,
 * and zero otherwise.
 */
int mqueue_is_writable(int mqueueid)
{
	return (mqueues[mqueueid].mode & S_IWUSR);
}

/*============================================================================*
 * mqueue_set_used()                                                          *
 *============================================================================*/

/**
 * @brief Sets a message queue as used.
 *
 * @param mqueueid ID of the target message queue.
 */
static inline void mqueue_set_used(int mqueueid)
{
	mqueues[mqueueid].flags |= MQUEUE_USED;
}

/*============================================================================*
 * mqueue_set_remove()                                                        *
 *============================================================================*/

/**
 * @brief Marks a message queue to be removed.
 *
 * @param mqueueid ID of the target message queue.
 */
void mqueue_set_remove(int mqueueid)
{
	mqueues[mqueueid].flags |= MQUEUE_REMOVE;
}

/*============================================================================*
 * mqueue_set_perm()                                                          *
 *============================================================================*/

/**
 * @brief Sets the access permissions of a message queue.
 *
 * @param mqueueid ID of the target message queue.
 */
void mqueue_set_perm(int mqueueid, int owner, mode_t mode)
{
	mqueues[mqueueid].owner = owner;
	mqueues[mqueueid].mode = mode;
}

/*============================================================================*
 * mqueue_set_name()                                                          *
 *============================================================================*/

/**
 * @brief Sets the name of a message queue.
 *
 * @param mqueueid ID of the target message queue.
 * @param name  Name for the target message queue.
 */
void mqueue_set_name(int mqueueid, const char *name)
{
	strcpy(mqueues[mqueueid].name, name);
}

/*============================================================================*
 * mqueue_set_size()                                                          *
 *============================================================================*/

/**
 * @brief Sets the size (in bytes) of a message queue.
 *
  @param mqueueid ID of the target message queue.
 * @param size  Size of the target message queue.
 */
void mqueue_set_size(int mqueueid, size_t size)
{
	mqueues[mqueueid].size = size;
}

/*============================================================================*
 * mqueue_clear_flags()                                                       *
 *============================================================================*/

/**
 * @brief Clears the flags of a message queue.
 *
 * @param mqueueid ID of the target message queue.
 */
static inline void mqueue_clear_flags(int mqueueid)
{
	mqueues[mqueueid].flags = 0;
}

/*============================================================================*
 * mqueue_alloc()                                                             *
 *============================================================================*/

/**
 * @brief Allocates a message queue.
 *
 * @return Upon successful completion, the ID of the newly allocated
 * message queue is returned. Upon failure, -1 is returned instead.
 */
int mqueue_alloc(void)
{
	/* Search for a free message queue. */
	for (int i = 0; i < MQUEUE_MAX; i++)
	{
		/* Found. */
		if (!mqueue_is_used(i))
		{
			mqueues[i].refcount = 1;
			mqueue_set_used(i);
			return (i);
		}
	}

	return (-1);
}

/*============================================================================*
 * mqueue_free()                                                              *
 *============================================================================*/

/**
 * @brief Free a message queue.
 */
static void mqueue_free(int mqueueid)
{
	mqueue_clear_flags(mqueueid);
}

/*============================================================================*
 * mqueue_get_size()                                                          *
 *============================================================================*/

/**
 * @brief Gets the size (in bytes) of a message queue.
 *
 * @param mqueueid ID of the target message queue.
 *
 * @returns The size (in bytes) of the target message queue.
 */
size_t mqueue_get_size(int mqueueid)
{
	return (mqueues[mqueueid].size);
}

/*============================================================================*
 * mqueue_is_full()                                                           *
 *============================================================================*/

/**
 * @brief Asserts whether or not a message queue is full.
 *
 * @param mqueueid ID of the target message queue.
 *
 * @returns One if the message queue is full, and zero otherwise.
 */
int mqueue_is_full(int mqueueid)
{
	return (mqueues[mqueueid].nstored == MQUEUE_MESSAGE_MAX);
}

/*============================================================================*
 * mqueue_is_empty()                                                          *
 *============================================================================*/

/**
 * @brief Asserts whether or not a message queue is emtpy.
 *
 * @param mqueueid ID of the target message queue.
 *
 * @returns One if the message queue is empty, and zero otherwise.
 */
int mqueue_is_empty(int mqueueid)
{
	return (mqueues[mqueueid].nstored == 0);
}

/*============================================================================*
 * mqueue_slot_alloc()                                                        *
 *============================================================================*/

/**
 * @brief Gets the last slot in the message queue.
 *
 * @param mqueueid ID of the target message queue.
 * @param prio     Priority.
 *
 * @return Upon successful completion, A pointer to the sorted location 
 * in the message queue. Upon failure, NULL pointer is returned instead.
 */
void *mqueue_slot_alloc(int mqueueid, unsigned prio)
{
	int i;
	
	/* Is the message queue full? */
	if (mqueue_is_full(mqueueid))
		return NULL;

	for (i = 0; i < mqueues[mqueueid].nstored; i++)
		if (mqueues[mqueueid].messages[i].prio < prio)
			break;
	
	for (int j = mqueues[mqueueid].nstored; j > i; j--)
	{
		mqueues[mqueueid].messages[j].prio = mqueues[mqueueid].messages[j - 1].prio;

		memcpy(
			mqueues[mqueueid].messages[j].msg,
			mqueues[mqueueid].messages[j - 1].msg,
			MQUEUE_MESSAGE_SIZE
		);
 	}

	mqueues[mqueueid].nstored++;
	mqueues[mqueueid].messages[i].prio = prio;

	return (mqueues[mqueueid].messages[i].msg);
}

/*============================================================================*
 * mqueue_slot_free()                                                         *
 *============================================================================*/

/**
 * @brief Put back a slot in the message queue.
 *
 * @param mqueueid ID of the target message queue.
 * @param slot     Slot ID.
 */
void mqueue_slot_free(int mqueueid, void * slot)
{
	int i;

	for (i = 0; i < mqueues[mqueueid].nstored; i++)
		if (mqueues[mqueueid].messages[i].msg == slot)
			break;

	/* Is the message queue not empty? */
	if (mqueues[mqueueid].nstored < i)
	{
		mqueues[mqueueid].nstored--;

		for (int j = i; j > mqueues[mqueueid].nstored; j++)
		{
			mqueues[mqueueid].messages[j].prio = mqueues[mqueueid].messages[j + 1].prio;

			memcpy(
				mqueues[mqueueid].messages[j].msg,
				mqueues[mqueueid].messages[j + 1].msg,
				MQUEUE_MESSAGE_SIZE
			);
		}
	}
}

/*============================================================================*
 * mqueue_get_first_slot()                                                    *
 *============================================================================*/

/**
 * @brief Gets the first slot in the message queue.
 *
 * @param mqueueid ID of the target message queue.
 * @param prio     Location to store the priority of the message.
 *
 * @return Upon successful completion, A pointer to the first slot message
 * in the message queue. Upon failure, NULL pointer is returned instead.
 */
void *mqueue_get_first_slot(int mqueueid, unsigned * prio)
{
	/* Is the message queue empty? */
	if (mqueue_is_empty(mqueueid))
		return NULL;

	*prio = mqueues[mqueueid].messages[0].prio;

	return (mqueues[mqueueid].messages[0].msg);
}

/*============================================================================*
 * mqueue_remove_first_slot()                                                 *
 *============================================================================*/

/**
 * @brief Removes the first slot in the message queue.
 *
 * @param mqueueid ID of the target message queue.
 */
void mqueue_remove_first_slot(int mqueueid)
{
	if (!mqueue_is_empty(mqueueid))
	{
		--mqueues[mqueueid].nstored;

		for (int i = 0; i < mqueues[mqueueid].nstored; i++)
		{
			mqueues[mqueueid].messages[i].prio = mqueues[mqueueid].messages[i + 1].prio;
			memcpy(mqueues[mqueueid].messages[i].msg,
					mqueues[mqueueid].messages[i + 1].msg,
					MQUEUE_MESSAGE_SIZE);
		}
	}
}

/*============================================================================*
 * mqueue_get()                                                               *
 *============================================================================*/

/**
 * @brief Gets a message queue.
 *
 * @param name Name of the target message queue.
 *
 * @return If the target message queue is found, its ID is
 * returned. Otherwise, -1 is returned instead.
 */
int mqueue_get(const char *name)
{
	for (int i = 0; i < MQUEUE_MAX; i++)
	{
		/* Message queue not in use. */
		if (!mqueue_is_used(i))
			continue;

		/* Found.*/
		if (!strcmp(mqueues[i].name, name))
		{
			mqueues[i].refcount++;
			return (i);
		}
	}

	return (-1);
}

/*============================================================================*
 * mqueue_put()                                                               *
 *============================================================================*/

/**
 * @brief Releases a message queue.
 *
 * @param mqueueid ID of the target message queue.
 */
void mqueue_put(int mqueueid)
{
	mqueues[mqueueid].refcount--;

	/* Unlink the message queue if no process is using it anymore. */
	if ((mqueues[mqueueid].refcount == 0) && (mqueue_is_remove(mqueueid)))
		mqueue_free(mqueueid);
}

/*============================================================================*
 * mqueue_init()                                                              *
 *============================================================================*/

/**
 * @brief Initializes the table of message queues.
 */
void mqueue_init(void)
{
	for (int i = 0; i < MQUEUE_MAX; i++)
	{
		mqueues[i].refcount = 0;
		mqueues[i].nstored = 0;
		mqueue_clear_flags(i);
	}
}

