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

#ifndef _UNIX_NOC_H_
#define _UNIX_NOC_H_

	#ifndef _UNIX_
		#error "bad target"
	#endif

	/* Cluster IDs. */
	#define IOCLUSTER0   0 /**< IO cluster 0.       */
	#define IOCLUSTER1   4 /**< IO cluster 1.       */
	#define CCLUSTER0    8 /**< Compute cluster  0. */
	#define CCLUSTER1    9 /**< Compute cluster  1. */
	#define CCLUSTER2   10 /**< Compute cluster  2. */
	#define CCLUSTER3   11 /**< Compute cluster  3. */
	#define CCLUSTER4   12 /**< Compute cluster  4. */
	#define CCLUSTER5   13 /**< Compute cluster  5. */
	#define CCLUSTER6   14 /**< Compute cluster  6. */
	#define CCLUSTER7   15 /**< Compute cluster  7. */
	#define CCLUSTER8   16 /**< Compute cluster  8. */
	#define CCLUSTER9   17 /**< Compute cluster  9. */
	#define CCLUSTER10  18 /**< Compute cluster 10. */
	#define CCLUSTER11  19 /**< Compute cluster 11. */
	#define CCLUSTER12  20 /**< Compute cluster 12. */
	#define CCLUSTER13  11 /**< Compute cluster 13. */
	#define CCLUSTER14  22 /**< Compute cluster 14. */
	#define CCLUSTER15  23 /**< Compute cluster 15. */

	/* Forward definitions. */
	extern void unix_noc_setup(void);
	extern void unix_noc_cleanup(void);

#endif /* _UNIX_NOC_H_ */


