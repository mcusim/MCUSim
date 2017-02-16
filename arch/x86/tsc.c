#include <stdio.h>

#include "arch/x86/tsc.h"

#define PIT_TICK_RATE	1193182ul

#define CAL_MS		10
#define CAL_LATCH	(PIT_TICK_RATE / (1000 / CAL_MS))
#define CAL_PIT_LOOPS	1000

uint64_t native_calibrate_tsc(void)
{
	unsigned int ebx, ecx, edx, cpuid_level;
	unsigned int eax_denominator, ebx_numerator, ecx_hz;
	unsigned int crystal_khz;

	/* Check function levels of CPUID */
	ebx = ecx = edx = 0;
	__get_cpuid(0x80000000, &cpuid_level, &ebx, &ecx, &edx);

	if (cpuid_level < 0x15)
		return 0;

	eax_denominator = ebx_numerator = ecx_hz = edx = 0;

	/* CPUID 0x15 TSC/Crystal ratio, plus optionally Crystal Hz */
	if (!__get_cpuid(0x15, &eax_denominator, &ebx_numerator,
			 &ecx_hz, &edx))
		fprintf(stderr, "CPUID request 0x15 unsupported!\n");

	if (ebx_numerator == 0 || eax_denominator == 0)
		return 0;

	crystal_khz = ecx_hz / 1000;

	return crystal_khz * ebx_numerator / eax_denominator;
}

uint64_t pit_calibrate_tsc(void)
{
	/* Preset PIT loop values */
	uint32_t latch = CAL_LATCH;
	uint64_t ms = CAL_MS;
	uint64_t t1, t2, delta;

	/* Set the Gate high, disable speaker */
	outb((inb(0x61) & ~0x02) | 0x01, 0x61);

	/*
	 * Setup CTC channel 2* for mode 0, (interrupt on terminal
	 * count mode), binary count. Set the latch register to 50ms
	 * (LSB then MSB) to begin countdown.
	 */
	outb(0xb0, 0x43);
	outb(latch & 0xff, 0x42);
	outb(latch >> 8, 0x42);

	t1 = t2 = get_cycles();

	while ((inb(0x61) & 0x20) == 0) {
		t2 = get_cycles();
	}

	/*
	 * Sanity checks (as it is performed in the kernel):
	 *
	 * If we were not able to read the PIT more than loopmin
	 * times, then we have been hit by a massive SMI
	 *
	 * If the maximum is 10 times larger than the minimum,
	 * then we got hit by an SMI as well.
	 *
	 * Note: we are working in the user space and do not have
	 * a luxury to start/stop interruptions and so. We just want
	 * to estimate TSC frequency here, i.e. measure how much
	 * ticks will be in 50 ms interval including SMI/SMM,
	 * interrupts, etc.
	 */

	/* Calculate the PIT value */
	delta = t2 - t1;
	return delta / ms;
}
