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
#ifndef MSIM_AVR_M328_H_
#define MSIM_AVR_M328_H_ 1

/* Include headers specific to the ATMega328P */
#define _SFR_ASM_COMPAT 1
#define __AVR_ATmega328__ 1

#include <stdio.h>
#include <stdint.h>

#include "mcusim/avr/io.h"
#include "mcusim/avr/sim/m328/m328_ioregs.h"
#include "mcusim/avr/sim/io_regs.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"

const static struct MSIM_AVR ORIG_M328 = {
	.name = "ATmega328",
	.signature = { SIGNATURE_0, SIGNATURE_1, SIGNATURE_2 },
	.pc = 0x0000,
	.pc_bits = 14,
	.freq = 1000000,		/* Clock frequency, in Hz */
	.clk_source = AVR_INT_CAL_RC_CLK,
	.lockbits = 0x3F,
	.regs_num = 32,			/* # of general purpose registers */
	.ioregs_num = 224,		/* # of I/O registers */
	.xmega = 0,
	.reduced_core = 0,
	.spm_pagesize = SPM_PAGESIZE,
	.flashstart = 0x0000,
	.flashend = FLASHEND,
	.ramstart = RAMSTART,
	.ramend = RAMEND,
	.ramsize = 2048,
	.e2start = 0x0000,
	.e2end = E2END,
	.e2size = 1024,
	.e2pagesize = E2PAGESIZE,
	.sfr_off = __SFR_OFFSET,
	.fuse = { LFUSE_DEFAULT, HFUSE_DEFAULT, 0xFF },
	.bls = {			/* Bootloader config */
		.start = 0x7000,
		.end = 0x7FFF,
		.size = 4096,
	},
	.intr = {
		.reset_pc = 0x0000,
		.ivt = 0x0004,
	},
};

#endif /* MSIM_AVR_M328_H_ */
