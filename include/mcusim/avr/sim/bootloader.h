/*
 * This file is part of MCUSim, an XSPICE library with microcontrollers.
 *
 * Copyright (C) 2017-2019 MCUSim Developers, see AUTHORS.txt for contributors.
 *
 * MCUSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MCUSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MSIM_AVR_BLDR_H_
#define MSIM_AVR_BLDR_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

/* Forward declaration of the structure to describe AVR microcontroller
 * instance. */
struct MSIM_AVR;

/* AVR instructions are 16-bits or 32-bits wide. This is why AVR program
 * memory is a linear and regular array of 16-bits words. However, MCUSim
 * treats program memory as an array of bytes.
 *
 * AVR program memory is little endian, so "start" is an address of the LSB
 * of the first instruction in bootloader, "end" - address of the MSB of
 * the last bootloader instruction. */
typedef struct MSIM_AVR_BLD {
	uint32_t start;		/* First Bootloader byte in PM, in bytes */
	uint32_t end;		/* Last Bootloader byte in PM, in bytes */
	uint32_t size;		/* Bootloader size, in bytes */
} MSIM_AVR_BLD;

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_BLDR_H_ */
