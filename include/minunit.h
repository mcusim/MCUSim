/*
 * MinUnit Test
 *
 * Created with help of MinUnit described by John Brewer.
 * http://www.jera.com/techinfo/jtns/jtn002.html
 */
#ifndef MSIM_MINUNIT_H_
#define MSIM_MINUNIT_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Define this to count all tests in a test suite */
extern int tests_run;

#define __MU_FAIL() printf("\n  Failure in %s:%s():%d\n",		\
			   __FILE__, __func__, __LINE__)		\

#define _mu_assert(test) do {						\
				if (!(test)) {				\
					__MU_FAIL();			\
					return 1;			\
				}					\
			 } while(0)

#define _mu_test(test) do {						\
				if (!(test)) {				\
					__MU_FAIL();			\
					return 0;			\
				}					\
			 } while(0)

#define _mu_verify(test) do {						\
				printf(" |--TEST\n");				\
				int r=test();				\
				tests_run++;				\
				if (r) return r;			\
			 } while(0)

/* Entry point for the MinUnit tests runner */
int main(int argc, char *argv[]);

/* Define this function to run desired tests */
int all_tests(void);

/* Define suite name - it'll be printed during suite running */
char *suite_name(void);

/* Define this function to perform setup actions before all tests */
void setup_tests(void);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_MINUNIT_H_ */
