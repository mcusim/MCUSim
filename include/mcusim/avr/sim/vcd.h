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
 *
 * Data types and function to dump AVR MCU registers to VCD file.
 */
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
 * i		Offset of the register (or MSB of 16-bit register) in data
 * 		memory. Can be negative to show this instance of the structure
 * 		does not describe any register.
 * reg_lowi	Offset of the LSB register part in data memory (usually
 * 		followed by L suffix, like TCNT1L). Can be negative in case
 * 		of 8-bit register.
 * n		Number of a specific bit to track only. Can be negative to
 * 		include all bits of the register.
 * old_val	Previous value of the register (8-bit or 16-bit).
 * name		Name of a register requested by user (TCNT1 instead of TCNT1H,
 * 		for example). */
struct MSIM_AVR_VCDReg {
	int32_t i;
	int32_t reg_lowi;
	int8_t n;
	uint32_t old_val;
	char name[16];
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

