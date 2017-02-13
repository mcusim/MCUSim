#ifndef MSIM_UTIL_TSC_H_
#define MSIM_UTIL_TSC_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint64_t rdtsc(void);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_UTIL_TSC_H_ */
