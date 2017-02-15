#ifndef MSIM_TOOLS_MATH_H_
#define MSIM_TOOLS_MATH_H_ 1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Calculates median for the given array.
 *
 * arr		Array to calculate median.
 * size		Size of array _arr_.
 *
 * Returns median or UINT64_MAX in case of error.
 */
uint64_t msim_median(uint64_t *arr, const uint64_t size);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_TOOLS_MATH_H_ */
