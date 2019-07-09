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
#ifndef MSIM_AVR_M2560_H_
#define MSIM_AVR_M2560_H_ 1

/* Include headers specific to the ATMega2560 */
#define _SFR_ASM_COMPAT 1
#define __AVR_ATmega2560__ 1

#include <stdio.h>
#include <stdint.h>

#include "mcusim/avr/io.h"
#include "mcusim/avr/sim/m2560/m2560_ioregs.h"
#include "mcusim/avr/sim/io_regs.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"

#define MCU_NAME	"ATmega2560"
#define RESET_PC	0x0000	/* Reset vector address, in bytes */
#define IVT_ADDR	0x0002	/* Interrupt vectors address, in bytes */
#define PC_BITS		17		/* PC bit capacity */
#define LBITS_DEFAULT	0xFF		/* Default lock bits */
#define CLK_SOURCE	AVR_INT_CAL_RC_CLK /* Calibrated Internal RC */
#define CLK_FREQ	1000000		/* Oscillator frequency, in Hz */
#define GP_REGS		32		/* GP registers, R0, R1, ..., R31 */
#define IO_REGS		480		/* I/O registers, PORTD, SREG, etc. */
#define BLS_START	0x03E000	/* First address in BLS, in bytes */
#define BLS_END		0x03FFFF	/* Last address in BLS, in bytes */
#define BLS_SIZE	8192		/* BLS size, in bytes */
#define FLASHSTART	0x0000
#define RAMSIZE		8192		/* Internal SRAM */
#define E2START		0x0000
#define E2SIZE		4096

#endif /* MSIM_AVR_M2560_H_ */
