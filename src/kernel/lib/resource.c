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

#include "resource.h"

/**
 * @brief Resource flags.
 */
/**@{*/
#define RESOURCE_FLAGS_USED  (1 << 0) /**< Used synchronization point? */
#define RESOURCE_FLAGS_BUSY  (1 << 1) /**< Busy?                       */
#define RESOURCE_FLAGS_WRITE (1 << 2) /**< Writable?                   */
#define RESOURCE_FLAGS_READ  (1 << 3) /**< Readable?                   */
#define RESOURCE_FLAGS_ASYNC (1 << 4) /**< Asynchronous?               */
/**@}*/

/*============================================================================*
 * resource_set_used()                                                        *
 *============================================================================*/

/**
 * @brief Sets a resource as used.
 *
 * @param resource Target resource.
 */
void resource_set_used(struct resource *rsrc)
{
	rsrc->flags |= RESOURCE_FLAGS_USED;
}

/*============================================================================*
 * resource_set_unused()                                                      *
 *============================================================================*/

/**
 * @brief Sets a resource as not used.
 *
 * @param resource Target resource.
 */
void resource_set_unused(struct resource *rsrc)
{
	rsrc->flags &= ~RESOURCE_FLAGS_USED;
}

/*============================================================================*
 * resource_set_busy()                                                        *
 *============================================================================*/

/**
 * @brief Sets a resource as busy.
 *
 * @param resource Target resource.
 */
void resource_set_busy(struct resource *rsrc)
{
	rsrc->flags |= RESOURCE_FLAGS_BUSY;
}

/*============================================================================*
 * resource_set_notbusy()                                                     *
 *============================================================================*/

/**
 * @brief Sets a resource as not busy.
 *
 * @param resource Target resource.
 */
void resource_set_notbusy(struct resource *rsrc)
{
	rsrc->flags &= ~RESOURCE_FLAGS_BUSY;
}

/*============================================================================*
 * resource_set_wronly()                                                      *
 *============================================================================*/

/**
 * @brief Sets a resource as write-only.
 *
 * @param resource Target resource.
 */
void resource_set_wronly(struct resource *rsrc)
{
	rsrc->flags |= RESOURCE_FLAGS_WRITE;
	rsrc->flags &= ~RESOURCE_FLAGS_READ;
}

/*============================================================================*
 * resource_set_rdonly()                                                      *
 *============================================================================*/

/**
 * @brief Sets a resource as read-only.
 *
 * @param resource Target resource.
 */
void resource_set_rdonly(struct resource *rsrc)
{
	rsrc->flags |= RESOURCE_FLAGS_READ;
	rsrc->flags &= ~RESOURCE_FLAGS_WRITE;
}

/*============================================================================*
 * resource_set_rdwr()                                                        *
 *============================================================================*/

/**
 * @brief Sets a resource as readable and writable.
 *
 * @param resource Target resource.
 */
void resource_set_rdwr(struct resource *rsrc)
{
	rsrc->flags |= (RESOURCE_FLAGS_READ | RESOURCE_FLAGS_WRITE);
}

/*============================================================================*
 * resource_set_async()                                                       *
 *============================================================================*/

/**
 * @brief Sets a resource as asynchronous.
 *
 * @param resource Target resource.
 *
 * @note A resource cannot be synchronous and asynchronous at the same time.
 */
void resource_set_async(struct resource *rsrc)
{
	rsrc->flags |= RESOURCE_FLAGS_ASYNC;
}

/*============================================================================*
 * resource_set_sync()                                                        *
 *============================================================================*/

/**
 * @brief Sets a resource as synchronous.
 *
 * @param resource Target resource.
 *
 * @note A resource cannot be synchronous and asynchronous at the same time.
 */
void resource_set_sync(struct resource *rsrc)
{
	rsrc->flags &= ~RESOURCE_FLAGS_ASYNC;
}

/*============================================================================*
 * resource_is_used()                                                         *
 *============================================================================*/

/**
 * @brief Asserts whether or not a resource is in use.
 *
 * @param resource Target resource.
 *
 * @returns One if the target resource is in used, and zero otherwise.
 */
int resource_is_used(const struct resource *rsrc)
{
	return (rsrc->flags & RESOURCE_FLAGS_USED);
}

/*============================================================================*
 * resource_is_busy()                                                         *
 *============================================================================*/

/**
 * @brief Asserts whether or not a resource is busy.
 *
 * @param resource Target resource.
 *
 * @returns One if the target resource is busy, and zero otherwise.
 */
int resource_is_busy(const struct resource *rsrc)
{
	return (rsrc->flags & RESOURCE_FLAGS_BUSY);
}

/*============================================================================*
 * resource_is_readable()                                                     *
 *============================================================================*/

/**
 * @brief Asserts whether or not a resource is readable.
 *
 * @param resource Target resource.
 *
 * @returns One if the target resource is readable and zero otherwise.
 */
int resource_is_readable(const struct resource *rsrc)
{
	return (rsrc->flags & RESOURCE_FLAGS_READ);
}

/*============================================================================*
 * resource_is_rdonly()                                                       *
 *============================================================================*/

/**
 * @brief Asserts whether or not a resource is read-only.
 *
 * @param resource Target resource.
 *
 * @returns One if the target resource is read-inly and zero otherwise.
 */
int resource_is_rdonly(const struct resource *rsrc)
{
	return ((rsrc->flags & RESOURCE_FLAGS_READ) == RESOURCE_FLAGS_READ);
}

/*============================================================================*
 * resource_is_writable()                                                     *
 *============================================================================*/

/**
 * @brief Asserts whether or not a resource is writable.
 *
 * @param resource Target resource.
 *
 * @returns One if the target resource is writable and zero otherwise.
 */
int resource_is_writable(const struct resource *rsrc)
{
	return (rsrc->flags & RESOURCE_FLAGS_WRITE);
}

/*============================================================================*
 * resource_is_wronly()                                                       *
 *============================================================================*/

/**
 * @brief Asserts whether or not a resource is write-only.
 *
 * @param resource Target resource.
 *
 * @returns One if the target resource is write-only and zero otherwise.
 */
int resource_is_wronly(const struct resource *rsrc)
{
	return ((rsrc->flags & RESOURCE_FLAGS_WRITE) == RESOURCE_FLAGS_WRITE);
}

/*============================================================================*
 * resource_is_async()                                                        *
 *============================================================================*/

/**
 * @brief Asserts whether or not a resource is asynchronous.
 *
 * @param resource Target resource.
 *
 * @returns One if the target resource is asynchronous and zero otherwise.
 *
 * @note A resource cannot be synchronous and asynchronous at the same time.
 */
int resource_is_async(const struct resource *rsrc)
{
	return (rsrc->flags & RESOURCE_FLAGS_ASYNC);
}

/*============================================================================*
 * resource_is_sync()                                                         *
 *============================================================================*/

/**
 * @brief Asserts whether or not a resource is synchronous.
 *
 * @param resource Target resource.
 *
 * @returns One if the target resource is synchronous and zero otherwise.
 *
 * @note A resource cannot be synchronous and asynchronous at the same time.
 */
int resource_is_sync(const struct resource *rsrc)
{
	return (!(rsrc->flags & RESOURCE_FLAGS_ASYNC));
}

/*============================================================================*
 * resource_dumb_alloc()                                                      *
 *============================================================================*/

/**
 * @brief Dumb resource allocator.
 *
 * @param pool Target generic resource pool.
 *
 * @returns Upon successful completion, the ID of a newly allocated
 * resource point is returned. Upon failure, -1 is returned instead.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static int resource_dumb_alloc(const struct resource_pool *pool)
{
	char *base = (char *) pool->resources;
	int n = pool->nresources;
	size_t size = pool->resource_size;

	/* Search for a free synchronization point. */
	for (int i = 0; i < n; i++)
	{
		struct resource *resource;
			
		resource = (struct resource *)(&base[i*size]);

		/* Found. */
		if (!resource_is_used(resource))
		{
			resource_set_used(resource);
			return (i);
		}
	}

	return (-1);
}

/*============================================================================*
 * resource_dumb_free()                                                       *
 *============================================================================*/

/**
 * @brief Dumb resource releaser.
 *
 * @param pool Target generic resource pool.
 * @param id   ID of the target resource.
 *
 * @note This function is non-blocking.
 * @note This function is @b NOT thread safe.
 * @note This function is reentrant.
 */
static void resource_dumb_free(const struct resource_pool *pool, int id)
{
	char *base = (char *) pool->resources;
	size_t size = pool->resource_size;
	struct resource *resource;
			
	resource = (struct resource *)(&base[id*size]);

	resource_set_unused(resource);
}

/*============================================================================*
 * Resource Allocator                                                         *
 *============================================================================*/

/**
 * @brief Default resource allocation operation.
 */
alloc_fn resource_alloc = resource_dumb_alloc;

/**
 * @brief Default resource free operation.
 */
free_fn resource_free = resource_dumb_free;
