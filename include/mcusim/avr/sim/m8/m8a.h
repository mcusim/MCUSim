/*
 * Copyright (c) 2017, 2018, The MCUSim Contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the MCUSim or its parts nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef MSIM_AVR_M8A_H_
#define MSIM_AVR_M8A_H_ 1

/* Include headers specific to the ATMega8A */
#define _SFR_ASM_COMPAT 1
#define __AVR_ATmega8A__ 1

#include <stdio.h>
#include <stdint.h>

#include "mcusim/avr/io.h"
#include "mcusim/avr/sim/m8/m8_ioregs.h"
#include "mcusim/avr/sim/io_regs.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"
#include "mcusim/avr/sim/timer.h"

#define MCU_NAME	"ATmega8A"
#define RESET_PC	0x0000	/* Reset vector address (in bytes) */
#define IVT_ADDR	0x0002	/* Interrupt vectors address (in bytes) */
#define PC_BITS		12	/* PC bit capacity */
#define LBITS_DEFAULT	0x3F	/* Default lock bits */
#define CLK_SOURCE	AVR_INT_CAL_RC_CLK /* Calibrated Internal RC */
#define CLK_FREQ	1000000	/* Oscillator frequency, in Hz */
#define GP_REGS		32	/* # of GP registers */
#define IO_REGS		64	/* # of I/O registers */
#define BLS_START	0x1800	/* First address in BLS, in bytes */
#define BLS_END		0x1FFF	/* Last address in BLS, in bytes */
#define BLS_SIZE	2048	/* BLS size, in bytes */

#define TICK_PERF_F	MSIM_M8AUpdate
#define SET_FUSE_F	MSIM_M8ASetFuse
#define SET_LOCK_F	MSIM_M8ASetLock
#define PASS_IRQS_F	MSIM_M8APassIRQs
#define RESET_SPM_F	MSIM_M8AResetSPM

int MSIM_M8AUpdate(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);
int MSIM_M8ASetFuse(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);
int MSIM_M8ASetLock(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);
int MSIM_M8APassIRQs(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);
int MSIM_M8AResetSPM(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);

#endif /* MSIM_AVR_M8A_H_ */

