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
#include <errno.h>
#include <string.h>

#include <nanvix/limits.h>

/**
 * @brief Buffer of objects.
 */
static struct
{
	int valid;                  /**< Valid slot?        */
	char obj[HAL_MAILBOX_MSG_SIZE]; /**< Underlying object. */
} buffer[HAL_NR_NOC_NODES];

/*============================================================================*
 * buffer_put()                                                               *
 *============================================================================*/

/**
 * @brief Puts an object in the buffer.
 *
 * @param id  Object ID.
 * @param obj Target object.
 *
 * @return Upon successful completion zero is returned.  Upon failure,
 * a negative error code is returned instead.
 */
int buffer_put(int id, const void *obj)
{
	/* Invalid object ID . */
	if ((id < 0) || (id >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Bad slot. */
	if (buffer[id].valid)
		return (-ENOENT);

	buffer[id].valid = 1;
	memcpy(buffer[id].obj, obj, HAL_MAILBOX_MSG_SIZE);

	return (0);
}

/*============================================================================*
 * buffer_get()                                                               *
 *============================================================================*/

/**
 * @brief Gets an object from the buffer.
 *
 * @param id  Object ID.
 * @param obj Target object.
 *
 * @return Upon successful completion zero is returned.
 * Upon failure, a negative error code is returned instead.
 */
int buffer_get(int id, void *obj)
{
	/* Invalid object ID . */
	if ((id < 0) || (id >= HAL_NR_NOC_NODES))
		return (-EINVAL);

	/* Bad slot. */
	if (!buffer[id].valid)
		return (-ENOENT);

	/* Get message. */
	buffer[id].valid = 0;
	memcpy(obj, buffer[id].obj, HAL_MAILBOX_MSG_SIZE);

	return (0);
}

/*============================================================================*
 * buffer_init()                                                              *
 *============================================================================*/

/**
 * @brief Initializes the buffer of objects.
 */
void buffer_init(void)
{
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
		buffer[i].valid = 0;
}
