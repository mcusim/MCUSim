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
#ifndef MSIM_AVR_VCD_H_
#define MSIM_AVR_VCD_H_ 1

#ifndef MSIM_MAIN_HEADER_H_
#error "Please, include mcusim/mcusim.h instead of this header."
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#include "mcusim/mcusim.h"

/* Maximum registers to dump in VCD file */
#define MSIM_AVR_VCD_REGS		512

/* Register of MCU which can be written into VCD file */
struct MSIM_AVR_VCDRegister {
	char name[16];			/* Name of a register (DDRB, etc.) */
	long off;			/* Offset to the register in RAM */
	unsigned char *addr;		/* Pointer to the register in RAM*/
	uint32_t oldv;
};

/* Specific bit of a register */
struct MSIM_AVR_VCDBit {
	/* Index of a register (or MSB of a 16-bit register) */
	short regi;
	/* Bit number (may be negative to include all bits of a register
	 * to dump) */
	short n;
	/* Index of LSB of a register (usually followed by L suffix,
	 * like TCNT1L, may be negative to show that register is 8-bit one) */
	int16_t reg_lowi;
	/* Name of a register requested by user (TCNT1 instead of TCNT1H,
	 * for instance) */
	char name[16];
};

/* Structure to keep details about registers to be dumped into VCD file */
struct MSIM_AVR_VCDDetails {
	/* List of all available registers for VCD dump */
	struct MSIM_AVR_VCDRegister regs[MSIM_AVR_VCD_REGS];
	/* Flags to dump the whole register (negative) or
	 * selected bit only (bit index) */
	struct MSIM_AVR_VCDBit bit[MSIM_AVR_VCD_REGS];
};

FILE *MSIM_AVR_VCDOpenDump(struct MSIM_AVR *mcu, const char *dumpname);

/* Function to dump MCU registers to VCD file.
 * This one is usually called each iteration of the main simulation loop. */
void MSIM_AVR_VCDDumpFrame(FILE *f, struct MSIM_AVR *mcu, unsigned long tick,
                           unsigned char fall);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_VCD_H_ */
