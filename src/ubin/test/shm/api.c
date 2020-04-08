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

#include <nanvix/limits.h>
#include <posix/sys/mman.h>
#include <posix/sys/stat.h>
#include <posix/fcntl.h>
#include <posix/unistd.h>
#include "../test.h"

/*==========================================================================*
 * API Test: Create Unlink                                                  *
 *==========================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_shm_create_unlink(void)
{
	int shm;
	char shm_name[SHM_NAME_MAX];

	/* Create and unlink shm. */
	ustrcpy(shm_name, "/shm");
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT, 0)) >= 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
}

/*==========================================================================*
 * API Test: Create Unlink 2                                                *
 *==========================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_shm_create_unlink2(void)
{
	int shm;
	char shm_name[SHM_NAME_MAX];

	/* Create and unlink shm. */
	ustrcpy(shm_name, "/shm");
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, (O_CREAT | O_EXCL), 0)) >= 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
}

/*==========================================================================*
 * API Test: Create Unlink 3                                                *
 *==========================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_shm_create_unlink3(void)
{
	int shm;
	char shm_name[SHM_NAME_MAX];

	/* Create and unlink shm. */
	ustrcpy(shm_name, "/shm");
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT, 0)) >= 0);
	TEST_ASSERT(nanvix_shm_open(shm_name, (O_CREAT | O_EXCL), 0) < 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
}

/*==========================================================================*
 * API Test: Create Unlink 4                                                *
 *==========================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_posix_shm_create_unlink4(void)
{
	int shm1, shm2;
	char shm_name[SHM_NAME_MAX];

	/* Create and unlink shm. */
	ustrcpy(shm_name, "/shm");
	TEST_ASSERT((shm1 = nanvix_shm_open(shm_name, (O_CREAT | O_EXCL), S_IRUSR)) >= 0);
	TEST_ASSERT((shm2 = nanvix_shm_open(shm_name, O_CREAT, S_IRUSR)) >= 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
}

/*==========================================================================*
 * API Test: Open Close 1                                                   *
 *==========================================================================*/

/**
 * @brief API Test: Open Close 1
 */
static void test_posix_nanvix_shm_open_close1(void)
{
	int shm1, shm2;
	char shm_name[SHM_NAME_MAX];

	/* Create and unlink shm. */
	ustrcpy(shm_name, "/shm");
	TEST_ASSERT((shm1 = nanvix_shm_open(shm_name, O_CREAT, S_IRUSR)) >= 0);
	TEST_ASSERT((shm2 = nanvix_shm_open(shm_name, 0, S_IRUSR)) >= 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
}

/*==========================================================================*
 * API Test: Open Close 2                                                   *
 *==========================================================================*/

/**
 * @brief API Test: Open Close 2
 */
static void test_posix_nanvix_shm_open_close2(void)
{
	int shm1, shm2;
	char shm_name[SHM_NAME_MAX];

	/* Create and unlink shm. */
	ustrcpy(shm_name, "/shm");
	TEST_ASSERT((shm1 = nanvix_shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT((shm2 = nanvix_shm_open(shm_name, O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
}

/*==========================================================================*
 * API Test: Truncate                                                       *
 *==========================================================================*/

/**
 * @brief API Test: Truncate
 */
static void test_posix_shm_truncate(void)
{
	int shm;
	char shm_name[SHM_NAME_MAX];

	/* Create and unlink shm. */
	ustrcpy(shm_name, "/shm");
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(nanvix_ftruncate(shm, PAGE_SIZE) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
}

#if 0

/*==========================================================================*
 * API Test: Map Unmap 1                                                    *
 *==========================================================================*/

/**
 * @brief API Test: Map Unmap 1
 */
static void test_posix_shm_map_unmap1(void)
{
	int shm;
	void *map;
	char shm_name[SHM_NAME_MAX];

	ustrcpy(shm_name, "/shm");
    TEST_ASSERT(memalloc() == 0);
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(nanvix_ftruncate(shm, PAGE_SIZE) == 0);
	TEST_ASSERT((map = nanvix_mmap(NULL, PAGE_SIZE, PROT_READ, MAP_PRIVATE, shm, 0)) != MAP_FAILED);
	TEST_ASSERT(nanvix_munmap(map, PAGE_SIZE) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
    TEST_ASSERT(memfree(0) == 0);
}

/*==========================================================================*
 * API Test: Map_Unmap 2                                                    *
 *==========================================================================*/

/**
 * @brief API Test: Map Unmap 2
 */
static void test_posix_shm_map_unmap2(void)
{
	int shm;
	void *map;
	char shm_name[SHM_NAME_MAX];

	ustrcpy(shm_name, "/shm");
    TEST_ASSERT(memalloc() == 0);
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(nanvix_ftruncate(shm, PAGE_SIZE) == 0);
	TEST_ASSERT((map = nanvix_mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_PRIVATE, shm, 0)) != MAP_FAILED);
	TEST_ASSERT(nanvix_munmap(map, PAGE_SIZE) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
    TEST_ASSERT(memfree(0) == 0);
}

/*==========================================================================*
 * API Test: Map Unmap 3                                                    *
 *==========================================================================*/

/**
 * @brief API Test: Map Unmap 3
 */
static void test_posix_shm_map_unmap3(void)
{
	int shm;
	void *map;
	char shm_name[SHM_NAME_MAX];

	ustrcpy(shm_name, "/shm");
    TEST_ASSERT(memalloc() == 0);
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(nanvix_ftruncate(shm, PAGE_SIZE) == 0);
	TEST_ASSERT((map = nanvix_mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, shm, 0)) != MAP_FAILED);
	TEST_ASSERT(nanvix_munmap(map, PAGE_SIZE) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
    TEST_ASSERT(memfree(0) == 0);
}

/*==========================================================================*
 * API Test: Map_Unmap 4                                                    *
 *==========================================================================*/

/**
 * @brief API Test: Map Unmap 4
 */
static void test_posix_shm_map_unmap4(void)
{
	int shm;
	void *map;
	char shm_name[SHM_NAME_MAX];

	ustrcpy(shm_name, "/shm");
    TEST_ASSERT(memalloc() == 0);
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(nanvix_ftruncate(shm, PAGE_SIZE) == 0);
	TEST_ASSERT((map = nanvix_mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_SHARED, shm, 0)) != MAP_FAILED);
	TEST_ASSERT(nanvix_munmap(map, PAGE_SIZE) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
    TEST_ASSERT(memfree(0) == 0);
}

/*==========================================================================*
 * API Test: Map Unmap 5                                                    *
 *==========================================================================*/

/**
 * @brief API Test: Map Unmap 5
 */
static void test_posix_shm_map_unmap5(void)
{
	int shm;
	void *map;
	char shm_name[SHM_NAME_MAX];

	ustrcpy(shm_name, "/shm");
    TEST_ASSERT(memalloc() == 0);
    TEST_ASSERT(memalloc() == 1);
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(nanvix_ftruncate(shm, 2*PAGE_SIZE) == 0);
	TEST_ASSERT((map = nanvix_mmap(NULL, PAGE_SIZE, PROT_READ, MAP_PRIVATE, shm, PAGE_SIZE)) != MAP_FAILED);
	TEST_ASSERT(nanvix_munmap(map, PAGE_SIZE) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
    TEST_ASSERT(memfree(0) == 0);
}

/*==========================================================================*
 * API Test: Map_Unmap 6                                                    *
 *==========================================================================*/

/**
 * @brief API Test: Map Unmap 6
 */
static void test_posix_shm_map_unmap6(void)
{
	int shm;
	void *map;
	char shm_name[SHM_NAME_MAX];

	ustrcpy(shm_name, "/shm");
    TEST_ASSERT(memalloc() == 0);
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(nanvix_ftruncate(shm, 2*PAGE_SIZE) == 0);
	TEST_ASSERT((map = nanvix_mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_PRIVATE, shm, PAGE_SIZE)) != MAP_FAILED);
	TEST_ASSERT(nanvix_munmap(map, PAGE_SIZE) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
    TEST_ASSERT(memfree(0) == 0);
}

/*==========================================================================*
 * API Test: Map Unmap 7                                                    *
 *==========================================================================*/

/**
 * @brief API Test: Map Unmap 7
 */
static void test_posix_shm_map_unmap7(void)
{
	int shm;
	void *map;
	char shm_name[SHM_NAME_MAX];

	ustrcpy(shm_name, "/shm");
    TEST_ASSERT(memalloc() == 0);
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(nanvix_ftruncate(shm, 2*PAGE_SIZE) == 0);
	TEST_ASSERT((map = nanvix_mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, shm, PAGE_SIZE)) != MAP_FAILED);
	TEST_ASSERT(nanvix_munmap(map, PAGE_SIZE) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
    TEST_ASSERT(memfree(0) == 0);
}

/*==========================================================================*
 * API Test: Map_Unmap 8                                                    *
 *==========================================================================*/

/**
 * @brief API Test: Map Unmap 8
 */
static void test_posix_shm_map_unmap8(void)
{
	int shm;
	void *map;
	char shm_name[SHM_NAME_MAX];

	ustrcpy(shm_name, "/shm");
    TEST_ASSERT(memalloc() == 0);
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(nanvix_ftruncate(shm, 2*PAGE_SIZE) == 0);
	TEST_ASSERT((map = nanvix_mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_SHARED, shm, PAGE_SIZE)) != MAP_FAILED);
	TEST_ASSERT(nanvix_munmap(map, PAGE_SIZE) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
    TEST_ASSERT(memfree(0) == 0);
}
/*==========================================================================*
 * API Test: Map_Unmap 9                                                    *
 *==========================================================================*/

/**
 * @brief API Test: Map Unmap 9
 */
static void test_posix_shm_map_unmap9(void)
{
	int shm;
	void *map1, *map2;
	char shm_name[SHM_NAME_MAX];

	ustrcpy(shm_name, "/shm");
    TEST_ASSERT(memalloc() == 0);
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(nanvix_ftruncate(shm, PAGE_SIZE) == 0);
	TEST_ASSERT((map1 = nanvix_mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_SHARED, shm, 0)) != MAP_FAILED);
	TEST_ASSERT(nanvix_munmap(map1, PAGE_SIZE) == 0);
	TEST_ASSERT((map2 = nanvix_mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_SHARED, shm, 0)) != MAP_FAILED);
	TEST_ASSERT(nanvix_munmap(map2, PAGE_SIZE) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
    TEST_ASSERT(memfree(0) == 0);
}

/*==========================================================================*
 * API Test: Sync 1                                                         *
 *==========================================================================*/

/**
 * @brief API Test: Sync 1
 */
static void test_posix_shm_sync1(void)
{
	int shm;
	void *map;
	char shm_name[SHM_NAME_MAX];

	ustrcpy(shm_name, "/shm");
    TEST_ASSERT(memalloc() == 0);
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(nanvix_ftruncate(shm, PAGE_SIZE) == 0);
	TEST_ASSERT((map = nanvix_mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_SHARED, shm, 0)) != MAP_FAILED);

	memset(map, 1, PAGE_SIZE);

	TEST_ASSERT(msync(map, PAGE_SIZE, MS_SYNC) == 0);

	TEST_ASSERT(nanvix_munmap(map, PAGE_SIZE) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
    TEST_ASSERT(memfree(0) == 0);
}

/*==========================================================================*
 * API Test: Sync 2                                                         *
 *==========================================================================*/

/**
 * @brief API Test: Sync 2
 */
static void test_posix_shm_sync2(void)
{
	int shm;
	char *map;
	char shm_name[SHM_NAME_MAX];

	ustrcpy(shm_name, "/shm");
    TEST_ASSERT(memalloc() == 0);
	TEST_ASSERT((shm = nanvix_shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) >= 0);
	TEST_ASSERT(nanvix_ftruncate(shm, PAGE_SIZE) == 0);
	TEST_ASSERT((map = nanvix_mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_SHARED, shm, 0)) != MAP_FAILED);

	/* Check sum. */
	for (int i = 0; i < PAGE_SIZE; i++)
		TEST_ASSERT(map[i] == 1);

	memset(map, 0, PAGE_SIZE);

	TEST_ASSERT(msync(map, PAGE_SIZE, MS_INVALIDATE) == 0);

	/* Check sum. */
	for (int i = 0; i < PAGE_SIZE; i++)
		TEST_ASSERT(map[i] == 1);

	TEST_ASSERT(nanvix_munmap(map, PAGE_SIZE) == 0);
	TEST_ASSERT(nanvix_shm_unlink(shm_name) == 0);
    TEST_ASSERT(memfree(0) == 0);
}

#endif

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test posix_shm_tests_api[] = {
	{ test_posix_shm_create_unlink,      "Create Unlink"   },
	{ test_posix_shm_create_unlink2,     "Create Unlink 2" },
	{ test_posix_shm_create_unlink3,     "Create Unlink 3" },
	{ test_posix_shm_create_unlink4,     "Create Unlink 4" },
	{ test_posix_nanvix_shm_open_close1, "Open Close 1"    },
	{ test_posix_nanvix_shm_open_close2, "Open Close 2"    },
	{ test_posix_shm_truncate,           "Truncate"        },
#if 0
	{ test_posix_shm_map_unmap1,     "Map Unmap 1"     },
	{ test_posix_shm_map_unmap2,     "Map Unmap 2"     },
	{ test_posix_shm_map_unmap3,     "Map Unmap 3"     },
	{ test_posix_shm_map_unmap4,     "Map Unmap 4"     },
	{ test_posix_shm_map_unmap5,     "Map Unmap 5"     },
	{ test_posix_shm_map_unmap6,     "Map Unmap 6"     },
	{ test_posix_shm_map_unmap7,     "Map Unmap 7"     },
	{ test_posix_shm_map_unmap8,     "Map Unmap 8"     },
	{ test_posix_shm_map_unmap9,     "Map Unmap 9"     },
	{ test_posix_shm_sync1,          "Sync 1"          },
	{ test_posix_shm_sync2,          "Sync 2"          },
#endif
	{ NULL,                          NULL              },
};

