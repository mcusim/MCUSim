/*
 * MinUnit Test
 *
 * Created with help of MinUnit described by John Brewer.
 * http://www.jera.com/techinfo/jtns/jtn002.html
 */

#include <stdio.h>

#include "minunit.h"

int main(int argc, char *argv[])
{
	char *sn = suite_name();
	printf("\n--- MinUnit '%s' Suite ---\n", sn);
	setup_tests();
	int r = all_tests();
	if (r == 0)
		printf("\n  ALL TESTS PASSED\n");
	printf("  Tests run: %d\n", tests_run);
	printf("\n--- MinUnit '%s' End Suite ---\n", sn);

	return r != 0;
}
