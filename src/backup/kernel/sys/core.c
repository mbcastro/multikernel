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

#define __NEED_HAL_CORE_
#include <hal.h>

/**
 * @brief Gets the ID of the underlying cluster.
 *
 * @returns The ID of the underlying cluster
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int sys_get_cluster_id(void)
{
	return (hal_get_cluster_id());
}

/**
 * @brief Gets the ID of the underlying core.
 *
 * @returns The ID of the underlying core.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 */
int sys_get_core_id(void)
{
	return (hal_get_core_id());
}

/**
 * @brief Gets the type of the underlying core.
 *
 * @returns The type of the underlying core.
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int sys_get_core_type(void)
{
	return (hal_get_core_type());
}

/**
 * @brief Gets the number of cores in the processor.
 *
 * @returns The number of cores in the processor.
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int sys_get_num_cores(void)
{
	return (hal_get_num_cores());
}

/**
 * @brief Gets the frequency of the underlying core.
 *
 * @returns The frequency of the underlying core.
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int sys_get_core_freq(void)
{
	return (hal_get_core_freq());
}

