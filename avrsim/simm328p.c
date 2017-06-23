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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* We would like to include headers specific to the ATMega328P
 * microcontroller. */
#define _SFR_ASM_COMPAT 1
#define __AVR_ATmega328P__ 1
#include "mcusim/avr/io.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"

#define FLASHSTART		0x0000
#define RAMSIZE			2048
#define E2START			0x0000
#define E2SIZE			1024

static int set_fuse_bytes(struct MSIM_AVR *mcu, uint8_t fuse_ext,
			  uint8_t fuse_high, uint8_t fuse_low);

int MSIM_M328PInit(struct MSIM_AVR *mcu,
		   uint8_t *pm, uint32_t pm_size,
		   uint8_t *dm, uint32_t dm_size)
{
	if (!mcu) {
		fprintf(stderr, "MCU should not be NULL\n");
		return -1;
	}

	strcpy(mcu->name, "atmega328p");
	mcu->signature[0] = SIGNATURE_0;
	mcu->signature[1] = SIGNATURE_1;
	mcu->signature[2] = SIGNATURE_2;
	mcu->spm_pagesize = SPM_PAGESIZE;
	mcu->flashstart = FLASHSTART;
	mcu->flashend = FLASHEND;
	mcu->ramstart = RAMSTART;
	mcu->ramend = RAMEND;
	mcu->ramsize = RAMSIZE;
	mcu->e2start = E2START;
	mcu->e2end = E2END;
	mcu->e2size = E2SIZE;
	mcu->e2pagesize = E2PAGESIZE;

	srand((unsigned int) time(NULL));

	mcu->id = (uint32_t) rand();
	mcu->lockbits = 0x3F;
	mcu->sfr_off = __SFR_OFFSET;
	mcu->regs = 32;
	mcu->io_regs = 224; /* 64 basic + 160 extended */

	MSIM_SetProgmem(mcu, pm, pm_size);
	MSIM_SetDatamem(mcu, dm, dm_size);

	mcu->sreg = &mcu->data_mem[_SFR_IO8(0x3F)];
	mcu->sph = &mcu->data_mem[_SFR_IO8(0x3E)];
	mcu->spl = &mcu->data_mem[_SFR_IO8(0x3D)];

	if (set_fuse_bytes(mcu, 0x00, 0xD9, 0xE1)) {
		fprintf(stderr, "Fuse bytes cannot be set correctly\n");
		return -1;
	}
	return 0;
}

static int set_fuse_bytes(struct MSIM_AVR *mcu, uint8_t fuse_ext,
			  uint8_t fuse_high, uint8_t fuse_low)
{
	return -1;
}
