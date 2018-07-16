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

#ifndef NANVIX_SYSCALLS_H_
#define NANVIX_SYSCALLS_H_

	#include <stddef.h>
	#include <inttypes.h>

	#define __NEED_HAL_CORE_
	#define __NEED_HAL_SETUP_
	#define __NEED_HAL_NOC_
	#define __NEED_HAL_SYNC_
	#define __NEED_HAL_MAILBOX_
	#include <nanvix/hal.h>

	/* Forward definitions. */
	extern void sys_setup(void);
	extern void sys_cleanup(void);
	extern uint64_t sys_timer_get(void);
	extern uint64_t sys_timer_diff(uint64_t, uint64_t);
	extern void sys_timer_init(void);
	extern int sys_get_cluster_id(void);
	extern int sys_get_core_id(void);
	extern int sys_get_core_type(void);
	extern int sys_get_num_cores(void);
	extern int sys_get_core_freq(void);
	extern int sys_get_node_id(void);
	extern int sys_mailbox_create(int);
	extern int sys_mailbox_open(int);
	extern int sys_mailbox_unlink(int);
	extern int sys_mailbox_close(int);
	extern size_t sys_mailbox_write(int, const void *, size_t);
	extern size_t sys_mailbox_read(int, void *, size_t);
	extern int sys_portal_allow(int, int);
	extern int sys_portal_create(int);
	extern int sys_portal_open(int);
	extern int sys_portal_read(int, void *, size_t);
	extern int sys_portal_write(int, const void *, size_t);
	extern int sys_portal_close(int);
	extern int sys_portal_unlink(int);
	extern int sys_sync_create(const int *, int, int);
	extern int sys_sync_open(const int *, int, int);
	extern int sys_sync_wait(int);
	extern int sys_sync_signal(int);
	extern int sys_sync_close(int);
	extern int sys_sync_unlink(int);
	extern int kernel_setup(void);
	extern int kernel_cleanup(void);
	extern int runtime_setup(void);
	extern int runtime_cleanup(void);

#endif /* NANVIX_SYSCALLS_H_ */
