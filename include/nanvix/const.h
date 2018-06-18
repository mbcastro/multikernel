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

#ifndef CONST_H_
#define CONST_H_

	/**
	 * @name System Information
	 */
	/**@{*/
	#define SYSNAME  "Nanvix"      /**< Operating system name. */
	#define NODENAME "bluedragon"  /**< Network node name.     */
	#define RELEASE  "beta"        /**< Kernel release.        */
	#define VERSION  "2.0"         /**< Kernel version.        */
	/**@}*/

	/**
	 * @name Scope Constants
	 */
	/**@{*/
	#define PUBLIC         /**< Global scope       */
	#define PRIVATE static /**< File scope.        */
	#define EXTERN extern  /**< Defined elsewhere. */
	/**@}*/

	/**
	 * @name Logical Constants
	 */
	#define FALSE 0 /**< False. */
	#define TRUE  1 /**< True.  */

#endif /* CONST_H_ */

