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
#ifndef MSIM_X86_TSC_H_
#define MSIM_X86_TSC_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <cpuid.h>
#include <sys/io.h>

#include "mcusim/clock/tsc.h"

/*
 * Standard way to access the cycle counter.
 */
typedef uint64_t cycles_t;

static inline cycles_t get_cycles(void)
{
	return rdtsc();
}

uint64_t pit_calibrate_tsc(void);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_X86_TSC_H_ */
