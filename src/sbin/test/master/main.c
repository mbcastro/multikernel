
#include <stdlib.h>

/* Forward definitions. */
extern void test_hal_sync(void);

/**
 * @brief Launches automated tests.
 */
int main2(int argc, const char **argv)
{
	((void) argc);
	((void) argv);

	test_hal_sync();

	return (EXIT_SUCCESS);
}
