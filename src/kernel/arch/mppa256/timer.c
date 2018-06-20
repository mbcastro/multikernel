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

#include <nanvix/arch/mppa.h>

#include "mppa.h" 

/**
 * @brief Timer error.
 */
static uint64_t timer_error = 0;

/**
 * @brief Gets the current timer value.
 *
 * @returns The current timer value;
 */
uint64_t hal_timer_get(void)
{
	return (__k1_read_dsu_timestamp());
}

/**
 * @brief Computes the difference between two timer values.
 *
 * @param t1 Start time.
 * @param t2 End time.
 *
 * @returns The difference between the two timers (t2 - t1).
 */
uint64_t hal_timer_diff(uint64_t t1, uint64_t t2)
{
	return (((t2 - t1) <= timer_error) ? timer_error : t2 - t1 - timer_error);
}

/**
 * @brief Calibrates the timer.
 */
void hal_timer_init(void)
{
	uint64_t start, end;

	start = hal_timer_get();
	end = hal_timer_get();

	timer_error = (end - start);
}
