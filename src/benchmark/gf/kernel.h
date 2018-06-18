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

#ifndef _KERNEL_H_
#define _KERNEL_H_

	#define MICRO (1.0/1000000)

	#define CHUNK_SIZE (512)     /* Maximum chunk size. */
	#define MASK_SIZE   15
    #define NCHUNKS    1024     /* Maximum number of chunks */

	#define PI 3.14159265359    /* pi */
	#define E 2.71828182845904  /* e */
	#define SD 0.8              /* Standard deviation. */

	/* Type of messages. */
	#define MSG_CHUNK 1
	#define MSG_DIE   0

	/* RMEM definitions. */
	#define SIZE_MASKSIZE    (sizeof(int))
	#define SIZE_IMGSIZE     (sizeof(int))

	#define SIZE_MASK        (masksize * masksize * sizeof(double))
	#define SIZE_IMAGE       (imgsize * imgsize * sizeof(unsigned char))
	#define SIZE_NEWIMAGE    SIZE_IMAGE
    #define SIZE_CHUNKS      NCHUNKS * (CHUNK_SIZE+masksize-1) * (CHUNK_SIZE+masksize-1)

	#define OFF_MASKSIZE   (0)
	#define OFF_IMGSIZE    (OFF_MASKSIZE  + SIZE_MASKSIZE)
	#define OFF_MASK       (OFF_IMGSIZE   + SIZE_IMGSIZE) 
	#define OFF_IMAGE      (OFF_MASK      + SIZE_MASK)
    #define OFF_NEWIMAGE   (OFF_IMAGE     + SIZE_IMAGE)
	#define OFF_CHUNKS     (OFF_NEWIMAGE  + SIZE_NEWIMAGE)
    #define OFF_CHUNKSIZE  (OFF_CHUNKS    + SIZE_CHUNKS)

#endif /* _KERNEL_H_ */
