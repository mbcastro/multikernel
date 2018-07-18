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

#ifndef NANVIX_SPAWNER_H_
#define NANVIX_SPAWNER_H_

	struct serverinfo
	{
		int (*main) (int);
		int nodenum;
	};
	/* Forward definitions. */
	extern int main2(int, const char **);
	extern void spawners_sync(void);

	/* Forward definitions. */
	extern const char *spawner_name;
	extern const int spawner_usermode;
	extern int NR_SERVERS;
	extern struct serverinfo servers[];
	extern void (*test_kernel_fn)(const char *);
	extern void (*test_runtime_fn)(const char *);
	extern int (*main2_fn)(int, const char **);

	#define SPAWNER_NAME(x) const char *spawner_name = x;
	#define SPAWNER_MAIN2(x) int (*main2_fn)(int, const char **) = x;
	#define SPAWNER_KERNEL_TESTS(x) void (*test_kernel_fn)(const char *) = x;
	#define SPAWNER_RUNTIME_TESTS(x) void (*test_runtime_fn)(const char *) = x;

#endif /* NANVIX_SPAWNER_H_*/

