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
#ifndef MSIM_AVR_VCD_DUMP_H_
#define MSIM_AVR_VCD_DUMP_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

/* Register of MCU which can be written into VCD file */
struct MSIM_VCDRegister {
	char name[16];			/* Name of a register (DDRB, etc.) */
	long off;			/* Offset to the register in RAM */
	unsigned char *addr;		/* Pointer to the register in RAM*/
	unsigned char oldv;
};

FILE *MSIM_VCDOpenDump(void *vmcu, const char *dumpname);

/*
 * Function to dump MCU registers to VCD file.
 * This one is usually called each iteration of the main simulation loop.
 */
void MSIM_VCDDumpFrame(FILE *f, void *vmcu, unsigned long tick,
		       unsigned char fall);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_VCD_DUMP_H_ */
