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
static int set_bootloader_size(struct MSIM_AVR *mcu, uint8_t fuse_high);
static int set_frequency(struct MSIM_AVR *mcu, uint8_t fuse_ext,
			 uint8_t fuse_high, uint8_t fuse_low);
static int set_reset_vector(struct MSIM_AVR *mcu, uint8_t fuse_high);

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

	if (set_fuse_bytes(mcu, 0xFF, 0xD9, 0x62)) {
		fprintf(stderr, "Fuse bytes cannot be set correctly\n");
		return -1;
	}
	return 0;
}

static int set_fuse_bytes(struct MSIM_AVR *mcu, uint8_t fuse_ext,
			  uint8_t fuse_high, uint8_t fuse_low)
{
	mcu->fuse[2] = fuse_ext;
	mcu->fuse[1] = fuse_high;
	mcu->fuse[0] = fuse_low;

	if (set_bootloader_size(mcu, fuse_high)) {
		fprintf(stderr, "Cannot set size of bootloader!\n");
		return -1;
	}
	if (set_frequency(mcu, fuse_ext, fuse_high, fuse_low)) {
		fprintf(stderr, "Cannoe set frequency configuration!\n");
		return -1;
	}
	if (set_reset_vector(mcu, fuse_high))
		return -1;
	return 0;
}

static int set_bootloader_size(struct MSIM_AVR *mcu, uint8_t fuse_high)
{
	/* Check BOOTSZ1:0 flags and set bootloader parameters accordingly */
	switch ((fuse_high >> 1) & 0x03) {
	case 0x03:
		mcu->boot_loader->start = 0xF80;
		mcu->boot_loader->end = 0xFFF;
		mcu->boot_loader->size = 128;
		break;
	case 0x02:
		mcu->boot_loader->start = 0xF00;
		mcu->boot_loader->end = 0xFFF;
		mcu->boot_loader->size = 256;
		break;
	case 0x01:
		mcu->boot_loader->start = 0xE00;
		mcu->boot_loader->end = 0xFFF;
		mcu->boot_loader->size = 512;
		break;
	case 0x00:
	default:
		mcu->boot_loader->start = 0xC00;
		mcu->boot_loader->end = 0xFFF;
		mcu->boot_loader->size = 1024;
		break;
	}

	return 0;
}

static int set_frequency(struct MSIM_AVR *mcu, uint8_t fuse_ext,
			 uint8_t fuse_high, uint8_t fuse_low)
{
	uint8_t cksel, ckdiv8;

	cksel = fuse_low & 0x0F;
	ckdiv8 = (fuse_low >> 7) & 1;

	if (!ckdiv8) {
		mcu->data_mem[CLKPR] &= ~((uint8_t) 0x0C);
		mcu->data_mem[CLKPR] |= 0x03;
	} else
		mcu->data_mem[CLKPR] &= ~((uint8_t) 0x0F);

	switch(cksel) {
	case 0x03:					/* Internal, 128 kHz */
		mcu->clk_source = AVR_INT_CLK;
		mcu->freq = 128;
		break;
	case 0x02:					/* Internal, 8(1) MHz */
		mcu->clk_source = AVR_INT_CLK;
		mcu->freq = !ckdiv8 ? 1000 : 8000;
		break;
	case 0x01:					/* Reserved */
		fprintf(stderr, "Value 0001 is reserved for CKSEL3..0!\n");
		return -1;
	case 0x00:
		mcu->clk_source = AVR_EXT_CLK;		/* External clock */
		mcu->freq = UINT32_MAX;			/* Unknown frequency */
		break;
	default:
		switch ((fuse_low >> 1) & 0x07) {
		case 0x02:
			mcu->clk_source = AVR_LOWFREQ_CRYSTAL_CLK;
			mcu->freq = 32768;
			break;
		case 0x03:
			mcu->clk_source = AVR_FULLSWING_CRYSTAL_CLK;
			mcu->freq = UINT32_MAX;
			break;
		case 0x04:
			mcu->clk_source = AVR_LOWP_CRYSTAL_CLK;
			mcu->freq = UINT32_MAX;
			break;
		case 0x05:
			mcu->clk_source = AVR_LOWP_CRYSTAL_CLK;
			mcu->freq = UINT32_MAX;
			break;
		case 0x06:
			mcu->clk_source = AVR_LOWP_CRYSTAL_CLK;
			mcu->freq = UINT32_MAX;
			break;
		case 0x07:
			mcu->clk_source = AVR_LOWP_CRYSTAL_CLK;
			mcu->freq = UINT32_MAX;
			break;
		}
		break;
	}
	return 0;
}

static int set_reset_vector(struct MSIM_AVR *mcu, uint8_t fuse_high)
{
	switch (fuse_high & 0x01) {
	case 0x00:
		/* Boot reset address */
		mcu->reset_pc = mcu->boot_loader->start;
		break;
	case 0x01:
	default:
		/* Lowest address of the program memory */
		mcu->reset_pc = 0x000;
		break;
	}
	mcu->pc = mcu->reset_pc;
	return 0;
}
