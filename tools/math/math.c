#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <inttypes.h>

#include "tools/math/math.h"

#define ARR_SIZE		4096

static uint64_t buf[ARR_SIZE];

static int compare_uint64(const void *a, const void *b);

static int compare_uint64(const void *a, const void *b)
{
	uint64_t arg1 = *(const uint64_t *)a;
	uint64_t arg2 = *(const uint64_t *)b;

	if (arg1 < arg2) return -1;
	if (arg1 > arg2) return 1;
	return 0;
}

uint64_t msim_median(uint64_t *arr, const uint64_t size)
{
	uint64_t i;

	if (size > ARR_SIZE) {
		fprintf(stderr, "Array size is above maximum: %lu > %d\n",
				size, ARR_SIZE);
		return UINT64_MAX;
	}

	/* Copy array to buffer in order to retain original one */
	for (i = 0; i < size; i++) {
		buf[i] = arr[i];
	}
	qsort(buf, size, sizeof(uint64_t), compare_uint64);
	if (size % 2 == 0) {
		/* Size is even */
		return (buf[size / 2] + buf[(size / 2) - 1]) / 2;
	} else {
		/* Size is odd */
		return buf[size / 2];
	}
}
