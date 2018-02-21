/*
 * Copyright (c) 2017, 2018,
 * Dmitry Salychev <darkness.bsd@gmail.com>,
 * Alexander Salychev <ppsalex@rambler.ru> et al.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
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
