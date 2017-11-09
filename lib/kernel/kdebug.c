/*
 * Copyright(C) 2011-2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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
#include <unistd.h>
#include <stdarg.h>

/**
 * @brief Writes a debug message to the kernel's output device.
 * 
 * @param fmt Formatted message to be written onto kernel's output device.
 */
void kdebug(const char *fmt, ...)
{
	int i;                         /* Loop index.              */
	va_list args;                  /* Variable arguments list. */
	char buffer[KBUFFER_SIZE + 1]; /* Temporary buffer.        */
	
	kstrncpy(buffer, "[debug] ", 8);
	
	/* Convert to raw string. */
	va_start(args, fmt);
	i = kvsprintf(buffer + 8, fmt, args) + 8;
	buffer[i++] = '\0';
	va_end(args);

	kputs(buffer);
}
