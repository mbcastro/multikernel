/*
 * Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *              2017-2017 Clement Rouquier <clementrouquier@gmail.com>
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

#include <nanvix/hal.h>
#include <nanvix/klib.h>
#include <sys/types.h>
#include <stdarg.h>

/**
 * @brief Writes on the screen a formated string.
 * 
 * @param fmt Formated string.
 */
void kprintf(const char *fmt, ...)
{
	int i;                         /* Loop index.              */
	va_list args;                  /* Variable arguments list. */
	char buffer[KBUFFER_SIZE + 1]; /* Temporary buffer.        */
	
	kstrncpy(buffer, "[info] ", 7);
	
	/* Convert to raw string. */
	va_start(args, fmt);
	i = kvsprintf(buffer + 7, fmt, args) + 7;
	buffer[i++] = '\0';
	va_end(args);

	kputs(buffer);
}
