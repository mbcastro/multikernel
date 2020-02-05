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

#define __NEED_RMEM_CACHE

#include <nanvix/runtime/rmem.h>
#include <nanvix/const.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Length of remote memory table.
 */
#define RMEM_TABLE_LENGTH 1024

/**
 * @brief Remote memory table
 */
static rpage_t rmem_table[RMEM_TABLE_LENGTH] = {
	[0 ... (RMEM_TABLE_LENGTH - 1)] = RMEM_NULL
};

/*============================================================================*
 * nanvix_rlookup()                                                           *
 *============================================================================*/

/**
 * @brief Looks up a remote memory address.
 *
 * @param base   Store location for base address.
 * @param offset Store location for offset address.
 * @param ptr    Remote memory address.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int nanvix_rlookup(raddr_t *base, raddr_t *offset, const void *ptr)
{
	raddr_t _base;
	raddr_t _offset;

	/* Invalid remote address. */
	if (ptr == RMEM_NULL)
		return (-EFAULT);

	_base = ((raddr_t) ptr) >> RMEM_BLOCK_SHIFT;

	/* Invalid remote memory area. */
	if (_base >= RMEM_TABLE_LENGTH)
		return (-EINVAL);

	/* Bad remote memory address. */
	if (rmem_table[_base] == RMEM_NULL)
		return (-EFAULT);

	_offset = ((raddr_t) ptr) & (RMEM_BLOCK_SIZE - 1);

	if (base != NULL)
		*base = _base;
	if (offset != NULL)
		*offset = _offset;

	return (0);
}

/*============================================================================*
 * nanvix_ralloc()                                                            *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 * @todo TODO: Enable arbitrary remote memory allocation.
 */
void *nanvix_ralloc(size_t n)
{
	raddr_t base;
	rpage_t pgnum;

	/* Invalid allocation size */
	if (n == 0)
		return (NULL);

	n = TRUNCATE(n, RMEM_BLOCK_SIZE);

	/* Bad allocation size. */
	if (n > RMEM_BLOCK_SIZE)
		return (NULL);

	/*
	 * Find an empty slot in the
	 * remote memory table.
	 */
	for (int i = 1; i < RMEM_TABLE_LENGTH; i++)
	{
		/* Found. */
		if (rmem_table[i] == RMEM_NULL)
		{
			base = i;
			goto found;
		}
	}

	return (NULL);

found:

	/* Allocate page. */
	if ((pgnum = nanvix_rcache_alloc()) == RMEM_NULL)
		return (NULL);

	rmem_table[base] = pgnum;

	return ((void *) (base << RMEM_BLOCK_SHIFT));
}

/*============================================================================*
 * nanvix_rfree()                                                             *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int nanvix_rfree(void *ptr)
{
	int err;      /* Error code.   */
	raddr_t base; /* Base address. */

	/* Invalid remote address. */
	if (ptr == RMEM_NULL)
		return (-EFAULT);

	/* Lookup remote address. */
	if ((err = nanvix_rlookup(&base, NULL, ptr)) < 0)
		return (err);

	/* Free underlying remote page. */
	if ((err = nanvix_rcache_free(rmem_table[base])) < 0)
		return (err);

	/* Update remote memory table. */
	rmem_table[base] = RMEM_NULL;

	return (0);
}

/*============================================================================*
 * nanvix_rread()                                                             *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 * @todo TODO: Enable arbitrary read offset.
 * @todo TODO: Enable arbitrary read sizes.
 */
size_t nanvix_rread(void *buf, const void *ptr, size_t n)
{
	char *rptr;     /* Cached remote page. */
	int err;        /* Error code.         */
	raddr_t base;   /* Base address.       */
	raddr_t offset; /* Offset address.     */

	/* Nothing to do. */
	if (n == 0)
		return (0);

	/* Invalid remote address. */
	if (ptr == RMEM_NULL)
	{
		errno = EFAULT;
		return (0);
	}

	/* Lookup remote address. */
	if ((err = nanvix_rlookup(&base, &offset, ptr)) < 0)
	{
		errno = -err;
		return (0);
	}

	/* Invalid buffer. */
	if (buf == NULL)
	{
		errno = EINVAL;
		return (0);
	}

	/* Invalid read size. */
	if ((n > RMEM_BLOCK_SIZE) || ((offset + n) > RMEM_BLOCK_SIZE))
	{
		errno = EINVAL;
		return (0);
	}

	/* Get cached remote page. */
	if ((rptr = nanvix_rcache_get(rmem_table[base])) == NULL)
		return (0);

	umemcpy(buf, &rptr[offset], n);

	return (n);
}

/*============================================================================*
 * nanvix_rwrite()                                                            *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 * @todo TODO: Enable arbitrary write offset.
 * @todo TODO: Enable arbitrary write sizes.
 */
size_t nanvix_rwrite(void *ptr, const void *buf, size_t n)
{
	char *rptr;     /* Cached remote page. */
	int err;        /* Error code.         */
	raddr_t base;   /* Base address.       */
	raddr_t offset; /* Offset address.     */

	/* Nothing to do. */
	if (n == 0)
		return (0);

	/* Invalid remote address. */
	if (ptr == RMEM_NULL)
	{
		errno = EFAULT;
		return (0);
	}

	/* Lookup remote address. */
	if ((err = nanvix_rlookup(&base, &offset, ptr)) < 0)
	{
		errno = -err;
		return (0);
	}

	/* Invalid buffer. */
	if (buf == NULL)
	{
		errno = EINVAL;
		return (0);
	}

	/* Invalid write size. */
	if ((n > RMEM_BLOCK_SIZE) || ((offset + n) > RMEM_BLOCK_SIZE))
	{
		errno = EINVAL;
		return (0);
	}

	/* Get cached remote page. */
	if ((rptr = nanvix_rcache_get(rmem_table[base])) == NULL)
		return (0);

	umemcpy(&rptr[offset], buf, n);

	return (n);
}
