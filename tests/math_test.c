/*
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
#include <stdint.h>
#include <inttypes.h>

#include "minunit.h"
#include "tools/math/math.h"

#define SUITE_NAME		"Math tests"

#define ARR_OVERSIZE		4097

/* Tests counter */
int tests_run = 0;

static uint64_t huge_arr[ARR_OVERSIZE];

/* Test functions prototypes */
int calc_median(void);

int calc_median(void)
{
	uint64_t a[] = { 1, 2, 3 };
	uint64_t b[] = { 10, 10, 20, 30 };

	_mu_test(msim_median(a, sizeof(a)/sizeof(a[0])) == 2);
	_mu_test(msim_median(b, sizeof(b)/sizeof(b[0])) == 15);
	_mu_test(msim_median(huge_arr, sizeof(huge_arr)/sizeof(huge_arr[0]))
			== UINT64_MAX);

	return 0;
}

int all_tests(void)
{
	_mu_verify(calc_median);
	return 0;
}

char *suite_name(void)
{
	return SUITE_NAME;
}

void setup_tests(void)
{
}

