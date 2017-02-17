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
#include <sys/io.h>

#include "minunit.h"
#include "arch/x86/tsc.h"
#include "tools/math/math.h"

#define SUITE_NAME		"x86 TSC tests"

/* Tests counter */
int tests_run = 0;

/* Test functions prototypes */
int calibrate_tsc(void);
int measure_cycle(void);

int calibrate_tsc(void)
{
	uint8_t i = 0;
	uint64_t tsc_pit_khz = 0;
	uint64_t tsc_vals[50];

	_mu_assert(ioperm(0x61, 1, 1) == 0);
	_mu_assert(ioperm(0x43, 1, 1) == 0);
	_mu_assert(ioperm(0x42, 1, 1) == 0);

	for (i = 0; i < 50; i++) {
		tsc_pit_khz = pit_calibrate_tsc();

		tsc_vals[i] = tsc_pit_khz;
		printf("%lu.%03lu\n", tsc_pit_khz / 1000,
				      tsc_pit_khz % 1000);
	}

	tsc_pit_khz = msim_median(tsc_vals, 50);
	printf("TSC median frequency: %lu.%03lu MHz\n", tsc_pit_khz / 1000,
							tsc_pit_khz % 1000);

	_mu_assert(ioperm(0x61, 1, 0) == 0);
	_mu_assert(ioperm(0x43, 1, 0) == 0);
	_mu_assert(ioperm(0x42, 1, 0) == 0);

	return 0;
}

int measure_cycle(void)
{
	uint64_t tsc_khz = 0;
	uint64_t tsc_vals[50];
	uint64_t i, t1, t2;
	const uint16_t cycles_count = 1000;

	_mu_assert(ioperm(0x61, 1, 1) == 0);
	_mu_assert(ioperm(0x43, 1, 1) == 0);
	_mu_assert(ioperm(0x42, 1, 1) == 0);

	for (i = 0; i < 50; i++) {
		tsc_vals[i] = pit_calibrate_tsc();
	}
	tsc_khz = msim_median(tsc_vals, 50);
	printf("TSC frequency: %lu.%03lu MHz\n", tsc_khz / 1000,
						 tsc_khz % 1000);

	t1 = t2 = get_cycles();

	for (i = 0; i < cycles_count; i++) {
		t2 = get_cycles();
	}
	printf("%d cycles of get_cycles() occupy: %.0f ns\n",
			cycles_count,
			((double) 1000000 / (double) tsc_khz) *
			(double) (t2-t1));

	_mu_assert(ioperm(0x61, 1, 0) == 0);
	_mu_assert(ioperm(0x43, 1, 0) == 0);
	_mu_assert(ioperm(0x42, 1, 0) == 0);

	return 0;
}

int all_tests(void)
{
	_mu_verify(calibrate_tsc);
	_mu_verify(measure_cycle);
	return 0;
}

char *suite_name(void)
{
	return SUITE_NAME;
}

void setup_tests(void)
{
}
