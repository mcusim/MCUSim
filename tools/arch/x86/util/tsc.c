#include "tools/util/tsc.h"

uint64_t rdtsc(void)
{
	uint32_t low, high;

	__asm volatile("rdtscp" : "=a" (low), "=d" (high));

	return low | ((uint64_t) high) << 32;
}
