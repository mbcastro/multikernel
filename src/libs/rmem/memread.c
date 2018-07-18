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

#include <nanvix/syscalls.h>
#include <nanvix/mm.h>
#include <stdio.h>
#include <string.h>
#include "mem.h"

/**
 * @brief Reads from a remote memory.
 *
 * @param addr Remote address.
 * @param bug  Location where the data should be written to.
 * @param n    Number of bytes to read.
 */
void memread(uint64_t addr, void *buf, size_t n)
{
	int clusterid;           /* Cluster ID of the calling process. */
	struct rmem_message msg; /* Remote memory operation.           */

	clusterid = sys_get_cluster_id();

	meminit();

	/* Build operation header. */
	msg.source = clusterid;
	msg.op = RMEM_READ;
	msg.blknum = addr;
	msg.size = n;

	/* Send operation header. */
	mailbox_write(_mem_outbox, &msg);

	/* Send data. */
	portal_allow(_mem_inportal, IOCLUSTER1);
	portal_read(_mem_inportal, buf, n);
}

