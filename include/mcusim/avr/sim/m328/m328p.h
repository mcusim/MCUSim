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
#ifndef MSIM_AVR_M328P_H_
#define MSIM_AVR_M328P_H_ 1

/* Include headers specific to the ATMega328P */
#define _SFR_ASM_COMPAT 1
#define __AVR_ATmega328P__ 1

#include <stdio.h>
#include <stdint.h>

#include "mcusim/avr/io.h"
#include "mcusim/avr/sim/m328/m328_ioregs.h"
#include "mcusim/avr/sim/io_regs.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"

#define MCU_NAME	"ATmega328P"
#define RESET_PC	0x0000	/* Reset vector address, in bytes */
#define IVT_ADDR	0x0002	/* Interrupt vectors address, in bytes */
#define PC_BITS		14	/* PC bit capacity */
#define LBITS_DEFAULT	0x3F	/* Default lock bits */
#define CLK_SOURCE	AVR_INT_CAL_RC_CLK /* Calibrated Internal RC */
#define CLK_FREQ	1000000	/* Oscillator frequency, in Hz */
#define GP_REGS		32	/* GP registers, R0, R1, ..., R31 */
#define IO_REGS		224	/* I/O registers, PORTD, SREG, etc. */
#define BLS_START	0x7000	/* First address in BLS, in bytes */
#define BLS_END		0x7FFF	/* Last address in BLS, in bytes */
#define BLS_SIZE	4096	/* BLS size, in bytes */
#define FLASHSTART	0x0000
#define RAMSIZE		2048
#define E2START		0x0000
#define E2SIZE		1024

#define SET_FUSE_F	MSIM_M328PSetFuse
#define SET_LOCK_F	MSIM_M328PSetLock
#define TICK_PERF_F	MSIM_M328PTickPerf

int MSIM_M328PSetFuse(struct MSIM_AVR *mcu, uint32_t fuse_n, uint8_t fuse_v);
int MSIM_M328PSetLock(struct MSIM_AVR *mcu, uint8_t lock_v);
int MSIM_M328PTickPerf(struct MSIM_AVR *mcu);

/* ATMega328P Fuse Low Byte */
enum MSIM_AVRFuseLowByte {
	AVR_FLB_CKSEL0,
	AVR_FLB_CKSEL1,
	AVR_FLB_CKSEL2,
	AVR_FLB_CKSEL3,
	AVR_FLB_SUT0,
	AVR_FLB_SUT1,
	AVR_FLB_CKOUT,
	AVR_FLB_CKDIV8
};
enum MSIM_AVRFuseHighByte {
	AVR_FHB_BOOTRST,
	AVR_FHB_BOOTSZ0,
	AVR_FHB_BOOTSZ1,
	AVR_FHB_EESAVE,
	AVR_FHB_WDTON,
	AVR_FHB_SPIEN,
	AVR_FHB_DWEN,
	AVR_FHB_RSTDISBL
};
enum MSIM_AVRFuseExtended {
	AVR_FEXT_BODLEVEL0,
	AVR_FEXT_BODLEVEL1,
	AVR_FEXT_BODLEVEL2,
};

#endif /* MSIM_AVR_M328P_H_ */
