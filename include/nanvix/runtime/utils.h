/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
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

#ifndef NANVIX_RUNTIME_UTILS_H_
#define NANVIX_RUNTIME_UTILS_H_

	#include <posix/errno.h>
	#include <stddef.h>
	#include <stdint.h>

/*============================================================================*
 * Bitmap                                                                     *
 *============================================================================*/

	/**
	 * @brief Shift of a bitmap word.
	 */
	#define BITMAP_WORD_SHIFT 5

	/**
	 * @brief Length of a bitmap word.
	 */
	#define BITMAP_WORD_LENGTH (1 << BITMAP_WORD_SHIFT)

	/**
	 * @brief Bitmap word.
	 */
	typedef uint32_t bitmap_t;

	/**
	 * @brief Full bitmap.
	 */
	#define BITMAP_FULL 0xffffffff

	/**
	 * @name Bitmap Operators
	 */
	#define IDX(a) ((a) >> BITMAP_WORD_SHIFT) /**< Returns the index of the bit.  */
	#define OFF(a) ((a) & 0x1F)               /**< Returns the offset of the bit. */

	/**
	 * @brief Sets a bit in a bitmap.
	 *
	 * @param bitmap Bitmap where the bit should be set.
	 * @param pos	Position of the bit that shall be set.
	 */
	#define bitmap_set(bitmap, pos) \
		(((bitmap_t *)(bitmap))[IDX(pos)] |= (0x1 << OFF(pos)))

	/**
	 * @brief Clears a bit in a bitmap.
	 *
	 * @param bitmap Bitmap where the bit should be cleared.
	 * @param pos	Position of the bit that shall be cleared.
	 */
	#define bitmap_clear(bitmap, pos) \
		(((bitmap_t *)(bitmap))[IDX(pos)] &= ~(0x1 << OFF(pos)))

	/**
	 * @brief Returns the number of bits that are set in a bitmap.
	 *
	 * @details Counts the number of bits that are set in a bitmap using a
	 *		  bit-hacking algorithm from Stanford.
	 *
	 * @param bitmap Bitmap to be searched.
	 * @param size   Size (in bytes) of the bitmap.
	 *
	 * @returns The number of bits that are set in the bitmap.
	 */
	extern bitmap_t bitmap_nset(bitmap_t *, size_t);

	/**
	 * @brief Returns the number of bits that are cleared in a bitmap.
	 *
	 * @details Counts the number of bits that are cleared in a bitmap using a
	 *		  bit-hacking algorithm from Stanford.
	 *
	 * @param bitmap Bitmap to be searched.
	 * @param size   Size (in bytes) of the bitmap.
	 *
	 * @returns The number of bits that are cleared in the bitmap.
	 */
	extern bitmap_t bitmap_nclear(bitmap_t *, size_t);

	/**
	 * @brief Searches for the first free bit in a bitmap.
	 *
	 * @details Searches for the first free bit in a bitmap. In order to speedup
	 *		  computation, bits are checked in chunks of 4 bytes.
	 *
	 * @param bitmap Bitmap to be searched.
	 * @param size   Size (in bytes) of the bitmap.
	 *
	 * @returns If a free bit is found, the number of that bit is returned. However,
	 *		  if no free bit is found #BITMAP_FULL is returned instead.
	 */
	extern bitmap_t bitmap_first_free(bitmap_t *, size_t);

	/**
	 * @brief Checks what is the value of the nth bit.
	 *
	 * @details Return the value of the bit on the nth position.
	 *
	 * @param bitmap Bitmap to be checked.
	 * @param idx Index of the bitmap to be checked.
	 *
	 * @returns The value of the bit in the idx position.
	 */
	extern bitmap_t bitmap_check_bit(bitmap_t *, bitmap_t);

/*============================================================================*
 * Debug                                                                      *
 *============================================================================*/

	/**
	 * @brief Dumps debug information.
	 *
	 * @param modulename Name of the calling module.
	 * @param fmt        Formatted string.
	 */
	extern void debug(const char *, const char *, ...);

#endif /* NANVIX_RUNTIME_UTILS_H_ */
