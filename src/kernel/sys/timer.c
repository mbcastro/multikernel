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

#include <omp.h>

/**
 * @brief Timer error.
 */
static double timer_error = 0;

/**
 * @brief Gets the current timer value.
 *
 * @returns The current timer value;
 */
double timer_get(void)
{
	return (omp_get_wtime());
}

/**
 * @brief Computes the difference between two timer values.
 *
 * @param t1 Start time.
 * @param t2 End time.
 *
 * @returns The difference between the two timers (t2 - t1).
 */
double timer_diff(double t1, double t2)
{
	return (t2 - t1 - timer_error);
}

/**
 * @brief Calibrates the timer.
 */
void timer_init(void)
{
	double start, end;

	start = timer_get();
	end = timer_get();

	timer_error = (end - start);
}


