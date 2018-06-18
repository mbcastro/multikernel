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

#include <nanvix/hal.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>
#include <stdio.h>
#include <assert.h>

/**
 * @brief Underlying IPC connectors.
 */
/**@{*/
int _mem_outbox = -1;    /* Mailbox used for small transfers. */
int _mem_inportal = -1;  /* Portal used for large transfers.  */
int _mem_outportal = -1; /* Portal used for large transfers.  */
/**@}*/

/**
 * @brief Initializes the RMA engine.
 */
void meminit(void)
{
	int clusterid;                   /* Cluster ID of the calling process.   */
	static int initialized = 0;      /* IS RMA Engine initialized?           */

	/* Already initialized.  */
	if (initialized)
		return;

	/* Retrieve cluster information. */
	clusterid = hal_get_cluster_id();

	/* Open underlying IPC connectors. */
	_mem_inportal = _portal_create(clusterid);
	_mem_outbox =hal_mailbox_open(IOCLUSTER1 + clusterid%NR_IOCLUSTER_DMA);
	_mem_outportal = _portal_open(clusterid%NR_IOCLUSTER_DMA);

	initialized = 1;
}
