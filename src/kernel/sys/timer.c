/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of Nanvix.
 * 
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/time.h>

/**
 * @brief Timer error.
 */
static long timer_error = 0;

/**
 * @brief Gets the current timer value.
 *
 * @returns The current timer value;
 */
long timer_get(void)
{
	struct timeval t;

	gettimeofday(&t, 0);

	return (t.tv_sec*1000000+t.tv_usec);
}

/**
 * @brief Computes the difference between two timer values.
 *
 * @param t1 Start time.
 * @param t2 End time.
 *
 * @returns The difference between the two timers (t2 - t1).
 */
long timer_diff(long t1, long t2)
{
	return (((t2 - t1) <= timer_error) ? timer_error : t2 - t1 - timer_error);
}

/**
 * @brief Calibrates the timer.
 */
void timer_init(void)
{
	long start, end;

	start = timer_get();
	end = timer_get();

	timer_error = (end - start);
}

