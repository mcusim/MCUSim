#ifndef MSIM_TOOLS_MATH_H_
#define MSIM_TOOLS_MATH_H_ 1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

double msim_median(const double *arr, const uint64_t size);

float msim_medianf(const float *arr, const uint64_t size);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_TOOLS_MATH_H_ */
