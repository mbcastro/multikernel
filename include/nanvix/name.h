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
   * @brief name request types
   */
  #define NAME_QUERY 0
  #define NAME_ADD 1
  #define NAME_REMOVE 2

  /**
   * @brief name query message.
   */
  struct name_message
  {
    uint16_t source;     	/**< Source cluster.	*/
    uint16_t op;      		/**< Operation.     	*/
  	int id;     					/**< Cluster ID.  		*/
  	int dma;    					/**< DMA channel. 		*/
  	const char *name;        	  /**< Portal name. 		*/
  	const char *process_name;		/**< Process name. 		*/
  };

#endif /* _NAME_H_ */
