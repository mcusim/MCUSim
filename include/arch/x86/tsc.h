#ifndef MSIM_X86_TSC_H_
#define MSIM_X86_TSC_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <cpuid.h>
#include <sys/io.h>

#include "tools/util/tsc.h"

/*
 * Standard way to access the cycle counter.
 */
typedef unsigned long long cycles_t;

static inline cycles_t get_cycles(void)
{
	return rdtsc();
}

uint64_t native_calibrate_tsc(void);
uint64_t pit_calibrate_tsc(void);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_X86_TSC_H_ */
