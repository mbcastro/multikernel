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

#ifndef NANVIX_SERVERS_NAME_H_
#define NANVIX_SERVERS_NAME_H_

	#ifndef __NAME_SERVICE
	#error "do not include this file"
	#endif

	#include <nanvix/servers/message.h>
	#include <nanvix/limits.h>
	#include <posix/stdint.h>

	/**
	 * @brief Operation types for Name Server.
	 */
	/**@{*/
	#define NAME_EXIT    0 /**< Exit request.            */
	#define NAME_LOOKUP  1 /**< lookup a name.           */
	#define NAME_LINK    2 /**< Add a new name.          */
	#define NAME_UNLINK  3 /**< Remove a name.           */
	#define NAME_SUCCESS 4 /**< Success acknowledgement. */
	#define NAME_FAIL    5 /**< Failure acknowledgement. */
	/**@}*/

	/**
	 * @brief Name Server message.
	 */
	struct name_message
	{
		message_header header;           /**< Message header. */
		int nodenum;                     /**< NoC node.       */
		int errcode;                     /**< Error code.     */
		char name[NANVIX_PROC_NAME_MAX]; /**< Portal name.    */
	};

#endif /* NANVIX_SERVERS_NAME_H_ */
