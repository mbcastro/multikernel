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

#ifndef NANVIX_NAME_H_
#define NANVIX_NAME_H_

	/**
	 * @brief Maximum length of a process name (including the
	 * terminating null character).
	 */
	#define PROC_NAME_MAX 50

	/**
	 * @brief Types of requests for name server.
	 */
	#define NAME_QUERY  1 /**< Query a name.   */
	#define NAME_ADD    2 /**< Add a new name. */
	#define NAME_REMOVE 3 /**< Remove a name.  */

  /**
   * @brief Name server message.
   */
  struct name_message
  {
    uint16_t source;                  /**< Source cluster. */
    uint16_t op;                      /**< Operation.      */
  	int id;                           /**< Cluster ID.     */
  	int dma;                          /**< DMA channel.    */
  	char name[50];                    /**< Portal name.    */
  	char process_name[PROC_NAME_MAX]; /**< Process name.   */
  };

#endif /* _NAME_H_ */
