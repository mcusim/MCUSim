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

/* Data types and function to dump AVR MCU registers to VCD file. */
#ifndef MSIM_AVR_VCD_H_
#define MSIM_AVR_VCD_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

/* Forward declaration of the structure to describe AVR microcontroller
 * instance. */
struct MSIM_AVR;

/* Maximum registers to be stored in a VCD file */
#define MSIM_AVR_VCD_REGS		512

/* Structure to describe an AVR I/O register to be tracked in a VCD file.
 *
 * i		Offset to the register (or MSB of 16-bit register) in the data
 * 		memory. It can be negative to show this instance of the
 *		structure does not describe any register.
 *
 * reg_lowi	Offset to the LSB register part in data memory (usually
 * 		followed by L suffix, like TCNT1L). Can be negative in case
 * 		of 8-bit register.
 *
 * n		Number of a specific bit to track only. Can be negative to
 * 		include all bits of the register.
 *
 * old_val	Previous value of the register (8-bit or 16-bit).
 *
 * name		Name of a register requested by user (TCNT1 instead of TCNT1H,
 * 		for example). */
typedef struct MSIM_AVR_VCDReg {
	int32_t i;
	int32_t reg_lowi;
	int8_t n;
	uint32_t old_val;
	char name[16];
} MSIM_AVR_VCDReg;

/* The main structure to describe a VCD dump. */
typedef struct MSIM_AVR_VCD {
	FILE *dump;
	struct MSIM_AVR_VCDReg regs[MSIM_AVR_VCD_REGS];
	char dump_file[4096];
} MSIM_AVR_VCD;

int MSIM_AVR_VCDOpen(struct MSIM_AVR *mcu);

int MSIM_AVR_VCDClose(struct MSIM_AVR *mcu);

/* Function to dump MCU registers to VCD file.
 * This one is usually called each iteration of the main simulation loop. */
void MSIM_AVR_VCDDumpFrame(struct MSIM_AVR *mcu, uint64_t tick);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_VCD_H_ */
