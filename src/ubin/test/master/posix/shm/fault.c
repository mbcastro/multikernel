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

#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define __NEED_HAL_BARRIER_
#include <nanvix/mm.h>

#include "test.h"

/*============================================================================*
 * Fault Injection Test: Invalid Create                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Create
 */
static void test_posix_shm_invalid_create(void)
{
	char buf[SHM_NAME_MAX + 1];

	memset(buf, 'a', SHM_NAME_MAX + 1);
	buf[SHM_NAME_MAX] = '\0';

	/* Create invalid shms. */
	TEST_ASSERT(shm_open(NULL, O_CREAT, 0) < 0);
	TEST_ASSERT(shm_open(buf, O_CREAT, 0) < 0);
	TEST_ASSERT(shm_open(NULL, (O_CREAT | O_EXCL), 0) < 0);
	TEST_ASSERT(shm_open(buf, (O_CREAT | O_EXCL), 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Create                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Create 
 */
static void test_posix_shm_bad_create(void)
{
	TEST_ASSERT(shm_open("", O_CREAT, 0) < 0);
	TEST_ASSERT(shm_open("", (O_CREAT | O_EXCL), 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Double Create                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Create 
 */
static void test_posix_shm_double_create(void)
{
	int shm;

	TEST_ASSERT((shm = shm_open("cool-name", O_CREAT, 0)) >= 0);
	TEST_ASSERT(shm_open("cool-name", (O_CREAT | O_EXCL), 0) < 0);
	TEST_ASSERT(shm_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Open                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Open
 */
static void test_posix_shm_invalid_open(void)
{
	char buf[SHM_NAME_MAX + 1];

	memset(buf, 'a', SHM_NAME_MAX + 1);
	buf[SHM_NAME_MAX] = '\0';

	/* Open invalid shms. */
	TEST_ASSERT(shm_open(NULL, 0, 0) < 0);
	TEST_ASSERT(shm_open(buf, 0, 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Open                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Open 
 */
static void test_posix_shm_bad_open(void)
{
	TEST_ASSERT(shm_open("", 0, 0) < 0);
	TEST_ASSERT(shm_open("cool-name", 0, 0) < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Unlink                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Unlink
 */
static void test_posix_shm_invalid_unlink(void)
{
	char buf[SHM_NAME_MAX + 1];

	memset(buf, 'a', SHM_NAME_MAX + 1);
	buf[SHM_NAME_MAX] = '\0';

	/* Unlink invalid shms. */
	TEST_ASSERT(shm_unlink(NULL) < 0);
	TEST_ASSERT(shm_unlink(buf) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Unlink                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Unlink 
 */
static void test_posix_shm_bad_unlink(void)
{
	TEST_ASSERT(shm_unlink("") < 0);
	TEST_ASSERT(shm_unlink("missing-name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Double Unlink                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Unlink 
 */
static void test_posix_shm_double_unlink(void)
{
	int shm;

	TEST_ASSERT((shm = shm_open("cool-name", O_CREAT, 0)) >= 0);
	TEST_ASSERT(shm_unlink("cool-name") == 0);
	TEST_ASSERT(shm_unlink("cool-name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Truncate                                     *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Truncate
 */
static void test_posix_shm_invalid_truncate(void)
{
	int shm;

	TEST_ASSERT((shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(ftruncate(-1, REGION_SIZE) < 0);
	TEST_ASSERT(ftruncate(1000000, REGION_SIZE) < 0);
	TEST_ASSERT(ftruncate(shm, RMEM_SIZE + 1) < 0);
	TEST_ASSERT(shm_unlink("/shm") == 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Truncate                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Truncate
 */
static void test_posix_shm_bad_truncate(void)
{
	int shm;
	void *map;

	TEST_ASSERT((shm = shm_open("/shm", O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(ftruncate(shm, REGION_SIZE) < 0);
	TEST_ASSERT(ftruncate(1, REGION_SIZE) < 0);
	TEST_ASSERT(shm_unlink("/shm") == 0);

	TEST_ASSERT((shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(ftruncate(shm, REGION_SIZE) == 0);
	TEST_ASSERT((map = mmap(NULL, REGION_SIZE, PROT_READ, MAP_PRIVATE, shm, 0)) != MAP_FAILED);
	TEST_ASSERT(ftruncate(shm, 2*REGION_SIZE) < 0);
	TEST_ASSERT(munmap(map, REGION_SIZE) == 0);
	TEST_ASSERT(shm_unlink("/shm") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Map                                          *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Map
 */
static void test_posix_shm_invalid_map(void)
{
	int shm;
	void *map;

	TEST_ASSERT((shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT((map = mmap(NULL, 0, PROT_READ, MAP_PRIVATE, shm, 0)) == MAP_FAILED);
	TEST_ASSERT(ftruncate(shm, REGION_SIZE) == 0);
	TEST_ASSERT((map = mmap(NULL, REGION_SIZE, PROT_READ, MAP_PRIVATE, -1, 0)) == MAP_FAILED);
	TEST_ASSERT((map = mmap(NULL, REGION_SIZE, PROT_READ, MAP_PRIVATE, 1000000, 0)) == MAP_FAILED);
	TEST_ASSERT((map = mmap(NULL, RMEM_SIZE + 1, PROT_READ, MAP_PRIVATE, shm, 0)) == MAP_FAILED);
	TEST_ASSERT((map = mmap(NULL, REGION_SIZE, 0, MAP_PRIVATE, shm, 0)) == MAP_FAILED);
	TEST_ASSERT((map = mmap(NULL, REGION_SIZE, PROT_READ, 0, shm, 0)) == MAP_FAILED);
	TEST_ASSERT(shm_unlink("/shm") == 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Map                                              *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Map
 */
static void test_posix_shm_bad_map(void)
{
	int shm;
	void *map;

	TEST_ASSERT((shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(ftruncate(shm, REGION_SIZE) == 0);
	TEST_ASSERT((map = mmap(NULL, REGION_SIZE, PROT_READ, MAP_PRIVATE, 1, 0)) == MAP_FAILED);
	TEST_ASSERT(shm_unlink("/shm") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Ununmap                                      *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Ununmap
 */
static void test_posix_shm_invalid_unmap(void)
{
	int shm;
	void *map;

	TEST_ASSERT((shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(ftruncate(shm, REGION_SIZE) == 0);
	TEST_ASSERT((map = mmap(NULL, REGION_SIZE, PROT_READ, MAP_PRIVATE, shm, 0)) != MAP_FAILED);
	TEST_ASSERT(munmap(NULL, REGION_SIZE) < 0);
	TEST_ASSERT(munmap(map, -REGION_SIZE) < 0);
	TEST_ASSERT(munmap(map, 0) < 0);
	TEST_ASSERT(munmap(map, REGION_SIZE + 1) < 0);
	TEST_ASSERT(munmap(map, REGION_SIZE) == 0);
	TEST_ASSERT(shm_unlink("/shm") == 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Unmap                                            *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Unmap
 */
static void test_posix_shm_bad_unmap(void)
{
	int shm;
	void *map;

	TEST_ASSERT((shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(ftruncate(shm, REGION_SIZE) == 0);
	TEST_ASSERT((map = mmap(NULL, REGION_SIZE, PROT_READ, MAP_PRIVATE, shm, 0)) != MAP_FAILED);
	TEST_ASSERT(munmap((void *)test_posix_shm_bad_unmap, REGION_SIZE) < 0);
	TEST_ASSERT(munmap(map, REGION_SIZE/2) < 0);
	TEST_ASSERT(munmap(map, REGION_SIZE) == 0);
	TEST_ASSERT(shm_unlink("/shm") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Sync                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Sync
 */
static void test_posix_shm_invalid_sync(void)
{
	int shm;
	void *map;

	TEST_ASSERT((shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(ftruncate(shm, REGION_SIZE) == 0);
	TEST_ASSERT((map = mmap(NULL, REGION_SIZE, PROT_WRITE, MAP_SHARED, shm, 0)) != MAP_FAILED);

	memset(map, 1, REGION_SIZE);

	TEST_ASSERT(msync(NULL, REGION_SIZE, MS_SYNC) < 0);
	TEST_ASSERT(msync(map, -REGION_SIZE, MS_SYNC) < 0);
	TEST_ASSERT(msync(map, REGION_SIZE + 1, MS_SYNC) < 0);
	TEST_ASSERT(msync(map, REGION_SIZE, MS_SYNC | MS_ASYNC) < 0);
	TEST_ASSERT(msync(map, REGION_SIZE, 0) < 0);

	TEST_ASSERT(munmap(map, REGION_SIZE) == 0);
	TEST_ASSERT(shm_unlink("/shm") == 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Sync                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Sync
 */
static void test_posix_shm_bad_sync(void)
{
	int shm;
	void *map;

	TEST_ASSERT((shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(ftruncate(shm, REGION_SIZE) == 0);
	TEST_ASSERT((map = mmap(NULL, REGION_SIZE, PROT_WRITE, MAP_SHARED, shm, 0)) != MAP_FAILED);
	memset(map, 1, REGION_SIZE);
	TEST_ASSERT(msync((void *)test_posix_shm_bad_sync, REGION_SIZE, MS_SYNC) < 0);
	TEST_ASSERT(munmap(map, REGION_SIZE) == 0);
	TEST_ASSERT(shm_unlink("/shm") == 0);

	TEST_ASSERT((shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(ftruncate(shm, REGION_SIZE) == 0);
	TEST_ASSERT((map = mmap(NULL, REGION_SIZE, PROT_READ, MAP_SHARED, shm, 0)) != MAP_FAILED);
	memset(map, 1, REGION_SIZE);
	TEST_ASSERT(msync(map, REGION_SIZE, MS_SYNC) < 0);
	TEST_ASSERT(munmap(map, REGION_SIZE) == 0);
	TEST_ASSERT(shm_unlink("/shm") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid OFlags                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid OFlags
 */
static void test_posix_shm_invalid_oflags(void)
{
	int shm;

	TEST_ASSERT((shm = shm_open("cool-name", O_CREAT, 0)) >= 0);
	TEST_ASSERT(shm_open("cool-name", O_CREAT, 0) < 0);
	TEST_ASSERT(shm_open("cool-name", O_RDONLY, 0) < 0);
	TEST_ASSERT(shm_open("cool-name", O_RDWR, 0) < 0);
	TEST_ASSERT(shm_unlink("cool-name") == 0);

	TEST_ASSERT((shm = shm_open("cool-name2", O_CREAT, S_IRUSR)) >= 0);
	TEST_ASSERT(shm_open("cool-name2", O_RDWR, 0) < 0);
	TEST_ASSERT(shm_unlink("cool-name2") == 0);

	TEST_ASSERT((shm = shm_open("cool-name3", O_CREAT, S_IWUSR)) >= 0);
	TEST_ASSERT(shm_open("cool-name3", O_CREAT, 0) < 0);
	TEST_ASSERT(shm_open("cool-name3", O_RDONLY, 0) < 0);
	TEST_ASSERT(shm_open("cool-name3", O_RDWR, 0) < 0);
	TEST_ASSERT(shm_unlink("cool-name3") == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test posix_shm_tests_fault[] = {
	{ test_posix_shm_invalid_create,   "Invalid Create"   },
	{ test_posix_shm_bad_create,       "Bad Create"       },
	{ test_posix_shm_double_create,    "Double Create"    },
	{ test_posix_shm_invalid_open,     "Invalid Open"     },
	{ test_posix_shm_bad_open,         "Bad Open"         },
	{ test_posix_shm_invalid_unlink,   "Invalid Unlink"   },
	{ test_posix_shm_bad_unlink,       "Bad Unlink"       },
	{ test_posix_shm_double_unlink,    "Double Unlink"    },
	{ test_posix_shm_invalid_truncate, "Invalid Truncate" },
	{ test_posix_shm_bad_truncate,     "Bad Truncate"     },
	{ test_posix_shm_invalid_map,      "Invalid Map"      },
	{ test_posix_shm_bad_map,          "Bad Map"          },
	{ test_posix_shm_invalid_unmap,    "Invalid Unmap"    },
	{ test_posix_shm_bad_unmap,        "Bad Ununmap"      },
	{ test_posix_shm_invalid_sync,     "Invalid Sync"     },
	{ test_posix_shm_bad_sync,         "Bad Sync"         },
	{ test_posix_shm_invalid_oflags,   "Invalid OFlags"   },
	{ NULL,                            NULL               },
};
