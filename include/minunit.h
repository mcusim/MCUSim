/*
 * MinUnit Test is a part of mcusim.
 *
 * Created with help of MinUnit described by John Brewer.
 * http://www.jera.com/techinfo/jtns/jtn002.html
 *
 * mcusim - Interactive simulator for microcontrollers.
 * Copyright (C) 2017 Dmitry Salychev <darkness.bsd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
