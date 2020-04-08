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

#ifndef NANVIX_LIMITS_SHM_H_
#define NANVIX_LIMITS_SHM_H_

	/**
	 * @name Limits on Shared Memory Regions
	 */
	/**@{*/
	#define SHM_OPEN_MAX             8  /**< Maximum number of opened shared memory regions. */
	#define SHM_MAX                128  /**< Maximum number of shared memory regions.        */
	#define SHM_NAME_MAX            64  /**< Maximum length for a shared memory region name. */
	#define SHM_MAP_SIZE_MAX  (64*1024) /**< Maximum mapping size (in bytes).                */
	/**@}*/

#endif /* NANVIX_LIMITS_SHM_H_ */
