/*
 * AVRSim - Simulator for AVR microcontrollers.
 * This software is a part of MCUSim, interactive simulator for
 * microcontrollers.
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
#ifndef MSIM_AVR_SIMM8A_H_
#define MSIM_AVR_SIMM8A_H_ 1

#include <stdio.h>
#include <stdint.h>

/* Include headers specific to the ATMega8A */
#define _SFR_ASM_COMPAT 1
#define __AVR_ATmega8A__ 1
#include "mcusim/avr/io.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"

#define MCU_NAME	"ATmega8A"

#define RESET_PC	0x0000
#define PC_BITS		12		/* PC bit capacity */
#define LBITS_DEFAULT	0x3F		/* Default lock bits */

#define CLK_SOURCE	AVR_INT_CAL_RC_CLK /* Calibrated Internal RC */
#define CLK_FREQ	1000000		/* Oscillator frequency, in Hz */

#define GP_REGS		32		/* GP registers, R0, R1, ..., R31 */
#define IO_REGS		64		/* I/O registers, PORTD, SREG, etc. */

#define BLS_START	0x1800		/* First address in BLS, in bytes */
#define BLS_END		0x1FFF		/* Last address in BLS, in bytes */
#define BLS_SIZE	2048		/* BLS size, in bytes */


#define SET_FUSE_FUNC	MSIM_M8ASetFuse
#define SET_LOCK_FUNC	MSIM_M8ASetLock


int MSIM_M8ASetFuse(void *mcu, unsigned int fuse_n, unsigned char fuse_v);
int MSIM_M8ASetLock(void *mcu, unsigned char lock_v);

#endif /* MSIM_AVR_SIMM8A_H_ */
