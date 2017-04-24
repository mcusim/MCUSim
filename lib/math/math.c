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
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <inttypes.h>

#include "mcusim/math/math.h"

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
