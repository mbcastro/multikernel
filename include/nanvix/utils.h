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

#ifndef NANVIX_UTILS_H_
#define NANVIX_UTILS_H_

	#include <errno.h>
	#include <stdio.h>
	#include <string.h>
	#include <stdint.h>

	/*========================================================================*
	 *                                Bitmap                                  *
	 *========================================================================*/

	/**
	 * @brief Bit number.
	 */
	typedef uint32_t bit_t;

	/**
	 * @brief Full bitmap.
	 */
	#define BITMAP_FULL 0xffffffff

	/**
	 * @name Bitmap Operators
	 */
	#define IDX(a) ((a) >> 5)   /**< Returns the index of the bit.  */
	#define OFF(a) ((a) & 0x1F) /**< Returns the offset of the bit. */

	/**
	 * @brief Sets a bit in a bitmap.
	 *
	 * @param bitmap Bitmap where the bit should be set.
	 * @param pos    Position of the bit that shall be set.
	 */
	#define bitmap_set(bitmap, pos) \
		(((uint32_t *)(bitmap))[IDX(pos)] |= (0x1 << OFF(pos)))

	/**
	 * @brief Clears a bit in a bitmap.
	 *
	 * @param bitmap Bitmap where the bit should be cleared.
	 * @param pos    Position of the bit that shall be cleared.
	 */
	#define bitmap_clear(bitmap, pos) \
		(((uint32_t *)(bitmap))[IDX(pos)] &= ~(0x1 << OFF(pos)))

	/* Forward definitions. */
	extern void debug(const char *, const char *, ...);
    extern unsigned bitmap_nset(uint32_t *, size_t);
    extern unsigned bitmap_nclear(uint32_t *, size_t);
    extern bit_t bitmap_first_free(uint32_t *, size_t);
    extern bit_t bitmap_check_bit(uint32_t *, uint32_t);

#endif /* NANVIX_UTILS_H_ */
