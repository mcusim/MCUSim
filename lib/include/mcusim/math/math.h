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
