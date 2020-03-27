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

#ifndef NANVIX_SERVERS_MESSAGE_H_
#define NANVIX_SERVERS_MESSAGE_H_

    #include <posix/stdint.h>

	/**
	 * @brief Polymorphic message header.
	 */
	typedef struct
	{
		uint16_t source;      /**< Source cluster. */
		uint8_t opcode;       /**< Operation.      */
		uint8_t mailbox_port; /**< Port Number     */
		uint8_t portal_port;  /**< Port Number     */
	} message_header;

	/**
	 * @brief Prints a message header in a string.
	 *
	 * @param str Target string.
	 * @param h   Target message header.
	 */
	extern void message_header_sprint(char *str, message_header *h);

	/**
	 * @brief Builds a message header.
	 *
	 * @param h      Target message header.
	 * @param opcode Opcode of the message.
	 */
	extern void message_header_build(message_header *h, uint8_t opcode);

	/**
	 * @brief Builds a message header.
	 *
	 * @param h           Target message header.
	 * @param opcode      Opcode of the message.
	 * @param portal_port Port number for portal.
	 */
	extern void message_header_build2(
		message_header *h,
		uint8_t opcode,
		uint8_t portal_port
	);

#endif /* NANVIX_SERVERS_MESSAGE_H_ */
