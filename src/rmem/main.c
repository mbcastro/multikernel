/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
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

#define __NEED_NAME_CLIENT

#include <nanvix/servers/message.h>
#include <nanvix/servers/name.h>
#include <nanvix/servers/rmem.h>
#include <nanvix/servers/spawn.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/utils.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/portal.h>
#include <nanvix/limits.h>
#include <ulibc/assert.h>
#include <ulibc/stdio.h>
#include <ulibc/stdlib.h>
#include <ulibc/string.h>
#include <posix/errno.h>
#include <stdint.h>

#ifdef DEBUG_RMEM
	#define rmem_debug(fmt, ...) debug("rmem", fmt, __VA_ARGS__)
#else
	#define rmem_debug(fmt, ...) { }
#endif

/**
 * @brief Number of remote memory blocks.
 */
#define RMEM_NUM_BLOCKS (RMEM_SIZE/RMEM_BLOCK_SIZE)

/**
 * @brief Server statistics.
 */
static struct
{
	int nreads;       /**< Number of reads.         */
	size_t read;      /**< Number of bytes read.    */
	int nwrites;      /**< Number of writes.        */
	size_t written;   /**< Number of bytes written. */
} stats = { 0, 0, 0, 0 };

/**
 * @brief Node number.
 */
static int nodenum;

/**
 * @brief Input mailbox for small messages.
 */
static int inbox;

/**
 * @brief Input portal for receiving data.
 */
static int inportal;

/**
 * @brief Remote memory.
 */
static char rmem[RMEM_SIZE];

/**
 * @brief Map of blocks.
 */
/* static int blocks[RMEM_NUM_BLOCKS]; */
static bit_t blocks[RMEM_NUM_BLOCKS/32];

/*============================================================================*
 * rmem_alloc()                                                               *
 *============================================================================*/

/**
 * @brief Initializes blocks array.
 */
static inline void blocks_init(void)
{
    nanvix_memset(blocks, 0, (RMEM_NUM_BLOCKS/32)*sizeof(uint32_t));
}

/*============================================================================*
 * do_rmem_malloc()                                                           *
 *============================================================================*/

/**
 * @brief Handles remote memory allocation.
 */
static inline uint32_t do_rmem_malloc(void)
{
	uint32_t bit = bitmap_first_free(blocks, RMEM_NUM_BLOCKS/8);

	rmem_debug("memalloc block=%x", bit);

	if (bit == BITMAP_FULL)
		return -ENOMEM;

    bitmap_set(blocks, bit);

	return bit;
}

/*============================================================================*
 * do_rmem_free()                                                             *
 *============================================================================*/

/**
 * @brief Handles remote memory free.
 *
 * @param blknum Target block.
 */
static inline void do_rmem_free(uint32_t blknum)
{
	/* Invalid block number. */
    if (blknum > RMEM_SIZE)
    {
        nanvix_printf("[nanvix][rmem] invalid block number\n");
        return;
    }

	/* Bad block number. */
    if (bitmap_check_bit(blocks, blknum) != 1)
    {
        nanvix_printf("[nanvix][rmem] double block free\n");
        return;
    }

	bitmap_clear(blocks, blknum);
}
/*============================================================================*
 * do_rmem_write()                                                            *
 *============================================================================*/

/**
 * @brief Handles a write request.
 *
 * @param inportal Input portal for data transfer.
 * @param remote   Remote client.
 * @param blknum   RMEM block.
 * @param size     Number of bytes to write.
 */
static inline void do_rmem_write(int remote, uint64_t blknum, int size)
{
	rmem_debug("write nodenum=%d blknum=%d size=%d",
		remote,
		blknum,
		size
	);

	/* Invalid write. */
	if ((blknum >= RMEM_SIZE) || (blknum + size > RMEM_SIZE))
	{
		nanvix_printf("[nanvix][rmem] invalid write\n");
		return;
	}

    /* Bad blknum read */
    if ((blknum % RMEM_BLOCK_SIZE) != 0)
	{
        nanvix_printf("[nanvix][rmem] bad read\n");
        return;
    }

    /* Bad size read. */
    if ((size % RMEM_BLOCK_SIZE) != 0)
	{
        nanvix_printf("[nanvix][rmem] bad read size\n");
        return;
    }

    /* Block not allocated. */
    if (bitmap_check_bit(blocks, blknum/RMEM_BLOCK_SIZE) == 0)
	{
        nanvix_printf("[nanvix][rmem] block not allocated\n");
        return;
    }

	/* Invalid write size. */
	if (size > RMEM_SIZE || size < 0)
	{
		nanvix_printf("[nanvix][rmem] invalid write size\n");
		return;
	}

	kportal_allow(inportal, remote);
	kportal_read(inportal, &rmem[blknum], size);
}

/*============================================================================*
 * do_rmem_read()                                                             *
 *============================================================================*/

/**
 * @brief Handles a read request.
 *
 * @param remote Remote client.
 * @param blknum RMEM block.
 * @param size   Number of bytes to write.
 */
static inline void do_rmem_read(int remote, uint64_t blknum, int size)
{
	int outportal;

	rmem_debug("read nodenum=%d blknum=%d size=%d",
		remote,
		blknum,
		size
	);

	/* Invalid read. */
	if ((blknum >= RMEM_SIZE) || (blknum + size > RMEM_SIZE))
	{
		nanvix_printf("[nanvix][rmem] invalid read\n");
		return;
	}

    /* Bad blknum read */
    if (blknum % RMEM_BLOCK_SIZE != 0)
	{
        nanvix_printf("[nanvix][rmem] bad read\n");
        return;
    }

    /* Bad size read. */
    if (size % RMEM_BLOCK_SIZE != 0)
	{
        nanvix_printf("[nanvix][rmem] bad read size\n");
        return;
    }

    /* Block not allocated. */
    if (bitmap_check_bit(blocks, blknum/RMEM_BLOCK_SIZE) == 0)
	{
        nanvix_printf("[nanvix][rmem] block not allocated\n");
        return;
    }

	/* Invalid write size. */
	if (size > RMEM_SIZE || size < 0)
	{
		nanvix_printf("[nanvix][rmem] invalid write size\n");
		return;
	}

	outportal = kportal_open(processor_node_get_num(core_get_id()), remote);
	kportal_write(outportal, &rmem[blknum], size);
	kportal_close(outportal);
}

/*============================================================================*
 * do_rmem_loop()                                                             *
 *============================================================================*/

/**
 * @brief Handles remote memory requests.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_rmem_loop(void)
{
	int shutdown = 0;
    int source;

	while(!shutdown)
	{
		struct rmem_message msg;

		kmailbox_read(inbox, &msg, sizeof(struct rmem_message));

		/* handle write operation. */
		switch (msg.header.opcode)
		{
			/* Write to RMEM. */
			case RMEM_WRITE:
				stats.nwrites++;
				stats.written += msg.size;
				do_rmem_write(msg.header.source, msg.blknum, msg.size);
				break;

			/* Read from RMEM. */
			case RMEM_READ:
				stats.nreads++;
				stats.read += msg.size;
				do_rmem_read(msg.header.source, msg.blknum, msg.size);
				break;

            /* Allocate RMEM. */
            case RMEM_MEMALLOC:
                msg.blknum = do_rmem_malloc();
                source = kmailbox_open(msg.header.source);
                nanvix_assert(kmailbox_write(source, &msg, sizeof(struct rmem_message)) == sizeof(struct rmem_message));
                nanvix_assert(kmailbox_close(source) == 0);
                break;

            /* Free  RMEM. */
            case RMEM_MEMFREE:
                do_rmem_free(msg.blknum);
                break;

			case RMEM_EXIT:
				shutdown = 1;
				break;

			/* Should not happen. */
			default:
				break;
		}
	}

	return (0);
}

/*============================================================================*
 * do_rmem_startup()                                                          *
 *============================================================================*/

/**
 * @brief Initializes the remote memory server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_rmem_startup(void)
{
	int ret;
	char pathname[NANVIX_PROC_NAME_MAX];

    blocks_init();

	nodenum = processor_node_get_num(core_get_id());

	/* Assign input mailbox. */
	inbox = stdinbox_get();

	/* Assign input portal. */
	inportal = stdinportal_get();

	/* Link name. */
	nanvix_strcpy(pathname, "/rmem");
	if ((ret = name_link(nodenum, pathname)) < 0)
		return (ret);

	return (0);
}

/*============================================================================*
 * do_rmem_shutdown()                                                         *
 *============================================================================*/

/**
 * @brief Shutdowns the remote memory server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_rmem_shutdown(void)
{
	return (0);
}

/*============================================================================*
 * do_rmem_server()                                                           *
 *============================================================================*/

/**
 * @brief Remote memory server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int do_rmem_server(void)
{
	int ret;

	nanvix_printf("[nanvix][rmem] booting up server\n");

	if ((ret = do_rmem_startup()) < 0)
		goto error;

	/* Unblock spawner. */
	nanvix_assert(stdsync_fence() == 0);
	nanvix_printf("[nanvix][rmem] server alive\n");

	if ((ret = do_rmem_loop()) < 0)
		goto error;

	nanvix_printf("[nanvix][rmem] shutting down server\n");

	if ((ret = do_rmem_shutdown()) < 0)
		goto error;

	return (0);

error:
	return (ret);
}

/*============================================================================*
 * __main2()                                                                  *
 *============================================================================*/

/**
 * @brief Handles remote memory requests.
 *
 * @param argc Argument count (unused).
 * @param argv Argument list (unused).
 *
 * @returns Always returns zero.
 */
int __main2(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	__runtime_setup(1);

	do_rmem_server();

	__runtime_cleanup();

	return (0);
}
