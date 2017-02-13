#include "minunit.h"
#include "tools/math/math.h"

#define SUITE_NAME		"Math tests"

/* Tests counter */
int tests_run = 0;

/* Test functions prototypes */
int calc_median(void);
int calc_medianf(void);

int calc_median(void)
{
	return 0;
}

int calc_medianf(void)
{
	return 0;
}

int all_tests(void)
{
	_mu_verify(calc_median);
	_mu_verify(calc_medianf);
	return 0;
}

char *suite_name(void)
{
	return SUITE_NAME;
}

void setup_tests(void)
{
}

