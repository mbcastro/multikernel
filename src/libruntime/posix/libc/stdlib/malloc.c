/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#define __NEED_MM_MANAGER

#include <nanvix/runtime/rmem.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>
#include <posix/stddef.h>

/**
 * @brief Size of block structure.
 */
#define SIZEOF_BLOCK (sizeof(struct block))

/**
 * @brief Memory block.
 */
struct block
{
	struct block *nextp; /* Next free block. */
	unsigned size;       /* Size (in bytes). */
};

/**
 * @brief Free list of blocks.
 */
static struct block freep = { 0, 0 };

/**
 * @brief Frees allocated memory.
 *
 * @param ptr Memory area to free.
 */
void nanvix_free(void *ptr)
{
	struct block* block = (struct block *)(((char *)ptr) - sizeof(size_t));

	block->nextp = freep.nextp;
	freep.nextp = block;
}

/**
 * @brief Allocates memory.
 *
 * @param size Number of bytes to allocate.
 *
 * @returns Upon successful completion with size not equal to 0,
 * nanvix_malloc() returns a pointer to the allocated space. If size is
 * 0, either a null pointer or a unique pointer that can be successfully
 * passed to nanvix_free() is returned. Otherwise, it returns a null
 * pointer and set errno to indicate the error.
 */
void *nanvix_malloc(size_t size)
{
	struct block *block = freep.nextp;
	struct block **head = &freep.nextp;

	size = TRUNCATE(size, 2*SIZEOF_BLOCK);

	/* Find a free block. */
	while (block != 0)
	{
		if (block->size >= size)
		{
			*head = block->nextp;

			return ((char*)block) + sizeof(size_t);
		}

		head = &(block->nextp);

		block = block->nextp;
	}

	size = TRUNCATE(size*SIZEOF_BLOCK, PAGE_SIZE)/PAGE_SIZE;

	if ((block = nanvix_vmem_alloc(size)) == NULL)
		return (NULL);

	block->size = size;

	return ((char*)block) + sizeof(size_t);
}
