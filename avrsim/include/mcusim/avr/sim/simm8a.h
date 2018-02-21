/*
 * Copyright (c) 2017, 2018,
 * Dmitry Salychev <darkness.bsd@gmail.com>,
 * Alexander Salychev <ppsalex@rambler.ru> et al.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
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

#define RESET_PC	0x0000	/* Reset vector address, in bytes */
#define IVT_ADDR	0x0002	/* Interrupt vectors address, in bytes */
#define PC_BITS		12	/* PC bit capacity */
#define LBITS_DEFAULT	0x3F	/* Default lock bits */

#define CLK_SOURCE	AVR_INT_CAL_RC_CLK /* Calibrated Internal RC */
#define CLK_FREQ	1000000	/* Oscillator frequency, in Hz */

#define GP_REGS		32	/* GP registers (r0, r1, etc.) */
#define IO_REGS		64	/* I/O registers (PORTD, SREG, etc.) */

#define BLS_START	0x1800	/* First address in BLS, in bytes */
#define BLS_END		0x1FFF	/* Last address in BLS, in bytes */
#define BLS_SIZE	2048	/* BLS size, in bytes */

#define SET_FUSE_F	MSIM_M8ASetFuse
#define SET_LOCK_F	MSIM_M8ASetLock
#define TICK_TIMERS_F	MSIM_M8ATickTimers
#define PROVIDE_IRQS_F	MSIM_M8AProvideIRQs

int MSIM_M8ASetFuse(void *mcu, unsigned int fuse_n, unsigned char fuse_v);
int MSIM_M8ASetLock(void *mcu, unsigned char lock_v);
int MSIM_M8ATickTimers(void *mcu);
int MSIM_M8AProvideIRQs(void *mcu);

#define SREG		_SFR_IO8(0x3F)
#define SPH		_SFR_IO8(0x3E)
#define SPL		_SFR_IO8(0x3D)
#include "mcusim/avr/sim/vcd_regs.h"

#endif /* MSIM_AVR_SIMM8A_H_ */
