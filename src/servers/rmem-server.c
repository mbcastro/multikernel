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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <nanvix/spawner.h>
#include <nanvix/utils.h>
#include <nanvix/syscalls.h>
#include <nanvix/mm.h>
#include <nanvix/name.h>
#include <nanvix/pm.h>

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
	double tstart;    /**< Start time.              */
	double tshutdown; /**< Shutdown time.           */
	double tnetwork;  /**< Network time.            */
	double tcpu;      /**< CPU Time.                */
} stats = { 0, 0, 0, 0, 0, 0, 0, 0 };

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
static int blocks[RMEM_NUM_BLOCKS];

/*============================================================================*
 * rmem_alloc()
 *============================================================================*/

/**
 * @brief Initializes blocks array.
 */
static inline void blocks_init(void)
{
    memset(blocks, 0, (RMEM_NUM_BLOCKS)*sizeof(int));
}

/*============================================================================*
 * rmem_malloc()                                                              *
 *============================================================================*/

/**
 * @brief Handles remote memory allocation.
 */
static inline uint64_t rmem_malloc(void)
{
	/* Search for a free block. */
    for (int i = 0; i < RMEM_NUM_BLOCKS; i++)
	{
		/* Found. */
        if (blocks[i] == 0)
		{
			int blknum;

            blknum = i;
            blocks[i] = 1;

            return (blknum);
        }
    }
    return (-ENOMEM);
}

/*============================================================================*
 * rmem_memfree()                                                             *
 *============================================================================*/

/**
 * @brief Handles remote memory free.
 *
 * @param blknum Target block.
 */
static inline void rmem_memfree(uint64_t blknum)
{
	/* Invalid block number. */
    if (blknum > RMEM_SIZE)
    {
        printf("[nanvix][rmem] invalid blocks number\n");
        return;
    }

	/* Bad block number. */
    if (blocks[blknum] != 1)
    {
        printf("[nanvix][rmem] double blocks free\n");
        return;
    }

    blocks[blknum] = 0;

}
/*============================================================================*
 * rmem_write()                                                               *
 *============================================================================*/

/**
 * @brief Handles a write request.
 *
 * @param inportal Input portal for data transfer.
 * @param remote   Remote client.
 * @param blknum   RMEM block.
 * @param size     Number of bytes to write.
 */
static inline void rmem_write(int remote, uint64_t blknum, int size)
{
	uint64_t t;

	rmem_debug("write nodenum=%d blknum=%d size=%d",
		remote,
		blknum,
		size
	);

	/* Invalid write. */
	if ((blknum >= RMEM_SIZE) || (blknum + size > RMEM_SIZE))
	{
		printf("[nanvix][rmem] invalid write\n");
		return;
	}

	/* Invalid write size. */
	if (size > RMEM_BLOCK_SIZE)
	{
		printf("[nanvix][rmem] invalid write size\n");
		return;
	}

	sys_portal_allow(inportal, remote);
	sys_portal_read(inportal, &rmem[blknum], size);

	sys_portal_ioctl(inportal, PORTAL_IOCTL_GET_LATENCY, &t);

	stats.tnetwork += t/((double) sys_get_core_freq());
}

/*============================================================================*
 * rmem_read()                                                                *
 *============================================================================*/

/**
 * @brief Handles a read request.
 *
 * @param remote Remote client.
 * @param blknum RMEM block.
 * @param size   Number of bytes to write.
 */
static inline void rmem_read(int remote, uint64_t blknum, int size)
{
	uint64_t t;
	int outportal;

	rmem_debug("read nodenum=%d blknum=%d size=%d",
		remote,
		blknum,
		size
	);

	/* Invalid read. */
	if ((blknum >= RMEM_SIZE) || (blknum + size > RMEM_SIZE))
	{
		printf("[nanvix][rmem] invalid read\n");
		return;
	}

	/* Invalid read size. */
	if (size > RMEM_BLOCK_SIZE)
	{
		printf("[nanvix][rmem] invalid read size\n");
		return;
	}

	outportal = sys_portal_open(remote);
	sys_portal_write(outportal, &rmem[blknum], size);
	sys_portal_ioctl(outportal, PORTAL_IOCTL_GET_LATENCY, &t);
	sys_portal_close(outportal);

	stats.tnetwork += t/((double) sys_get_core_freq());
}

/*============================================================================*
 * rmem_loop()                                                                *
 *============================================================================*/

/**
 * @brief Handles remote memory requests.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int rmem_loop(void)
{
	int shutdown = 0;
	uint64_t t0, t1;
	double t2, t3;

	stats.tstart = sys_timer_get()/((double) sys_get_core_freq());

	while(!shutdown)
	{
		struct rmem_message msg;

		sys_mailbox_read(inbox, &msg, MAILBOX_MSG_SIZE);

		t2 = stats.tnetwork;
		t0 = sys_timer_get();

		/* handle write operation. */
		switch (msg.header.opcode)
		{
			/* Write to RMEM. */
			case RMEM_WRITE:
				stats.nwrites++;
				stats.written += msg.size;
				rmem_write(msg.header.source, msg.blknum, msg.size);
				break;

			/* Read from RMEM. */
			case RMEM_READ:
				stats.nreads++;
				stats.read += msg.size;
				rmem_read(msg.header.source, msg.blknum, msg.size);
				break;

            /* Allocate RMEM. */
            case RMEM_MEMALLOC:
                blk = rmem_malloc();
                source = sys_mailbox_open(msg.header.source);
                mailbox_write(source, &blk, msg.size);
                mailbox_close(source);
                break;

            /* Free  RMEM. */
            case RMEM_MEMFREE:
                rmem_memfree(msg.blknum);
                break;

			case RMEM_EXIT:
				stats.tshutdown = sys_timer_get()/((double) sys_get_core_freq());
				shutdown = 1;
				break;

			/* Should not happen. */
			default:
				break;
		}

		t1 = sys_timer_get();
		t3 = stats.tnetwork;

		stats.tcpu += ((t1 - t0)/((double) sys_get_core_freq())) - (t3 - t2);
	}

	/* Dump statistics. */
	printf("[nanvix][rmem] uptime=%lf cpu=%lf network=%lf read=%zu nreads=%d written=%zu nwrites=%d\n",
			(stats.tshutdown - stats.tstart),
			stats.tcpu,
			stats.tnetwork,
			stats.read, stats.nreads, stats.written, stats.nwrites
	);

	return (0);
}

/*============================================================================*
 * rmem_startup()                                                             *
 *============================================================================*/

/**
 * @brief Initializes the remote memory server.
 *
 * @param _inbox    Input mailbox.
 * @param _inportal Input portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int rmem_startup(int _inbox, int _inportal)
{
	int ret;
	char pathname[NANVIX_PROC_NAME_MAX];

    blocks_init();

	nodenum = sys_get_node_num();

	/* Assign input mailbox. */
	inbox = _inbox;

	/* Assign input portal. */
	inportal = _inportal;

	/* Link name. */
	sprintf(pathname, "/rmem");
	if ((ret = name_link(nodenum, pathname)) < 0)
		return (ret);

	return (0);
}

/*============================================================================*
 * rmem_shutdown()                                                            *
 *============================================================================*/

/**
 * @brief Shutdowns the remote memory server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int rmem_shutdown(void)
{
	return (0);
}

/*============================================================================*
 * rmem_server()                                                              *
 *============================================================================*/

/**
 * @brief Remote memory server.
 *
 * @param _inbox    Input mailbox.
 * @param _inportal Input portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int rmem_server(int _inbox, int _inportal)
{
	int ret;

	printf("[nanvix][rmem] booting up server\n");

	if ((ret = rmem_startup(_inbox, _inportal)) < 0)
		goto error;

	spawner_ack();

	printf("[nanvix][rmem] server alive\n");

	if ((ret = rmem_loop()) < 0)
		goto error;

	printf("[nanvix][rmem] shutting down server\n");

	if ((ret = rmem_shutdown()) < 0)
		goto error;

	return (0);

error:
	return (ret);
}
