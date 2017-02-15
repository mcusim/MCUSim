#include <sys/io.h>

#include "minunit.h"
#include "arch/x86/tsc.h"
#include "tools/math/math.h"

#define SUITE_NAME		"x86 TSC tests"

/* Tests counter */
int tests_run = 0;

/* Test functions prototypes */
int calibrate_tsc(void);

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
	printf("Median: %lu.%03lu\n", tsc_pit_khz / 1000,
				      tsc_pit_khz % 1000);

	_mu_assert(ioperm(0x61, 1, 0) == 0);
	_mu_assert(ioperm(0x43, 1, 0) == 0);
	_mu_assert(ioperm(0x42, 1, 0) == 0);

	return 0;
}

int all_tests(void)
{
	_mu_verify(calibrate_tsc);
	return 0;
}

char *suite_name(void)
{
	return SUITE_NAME;
}

void setup_tests(void)
{
}
