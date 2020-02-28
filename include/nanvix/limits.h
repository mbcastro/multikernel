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

#ifndef NANVIX_LIMITS_H_
#define NANVIX_LIMITS_H_

	#include <nanvix/sys/portal.h>

	/**
	 * @brief Number of NoC nodes.
	 */
	#define NANVIX_NODES_NUM PROCESSOR_NOC_NODES_NUM

	/**
	 * @brief Maximum length of a process name.
	 *
	 * @note The null character is included.
	 */
	#define NANVIX_PROC_NAME_MAX 64

	/**
	 * @brief Maximum number of mailboxes that can be opened.
	 */
	#define NANVIX_MAILBOX_MAX (MAILBOX_CREATE_MAX + MAILBOX_OPEN_MAX)

	/**
	 * @brief Maximum number of portals that can be opened.
	 */
	#define NANVIX_PORTAL_MAX KPORTAL_MAX

#endif /* NANVIX_LIMITS_H_ */
