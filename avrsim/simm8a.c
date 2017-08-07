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

/* We would like to include headers specific to the ATMega8A microcontroller */
#define _SFR_ASM_COMPAT 1
#define __AVR_ATmega8A__ 1
#include "mcusim/avr/io.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"

static int is_ckopt_programmed(uint8_t ckopt_f);
static int set_fuse_bytes(struct MSIM_AVR *mcu, uint8_t fuse_high,
			  uint8_t fuse_low);
static int set_bldr_size(struct MSIM_AVR *mcu, uint8_t fuse_high);
static int set_frequency(struct MSIM_AVR *mcu, uint8_t fuse_high,
			 uint8_t fuse_low);
static int set_reset_vector(struct MSIM_AVR *mcu, uint8_t fuse_high);

int MSIM_M8AInit(struct MSIM_AVR *mcu,
		 unsigned char *pm, unsigned long pm_size,
		 unsigned char *dm, unsigned long dm_size)
{
	if (!mcu) {
		fprintf(stderr, "MCU should not be NULL\n");
		return -1;
	}

	strcpy(mcu->name, "atmega8a");
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
	mcu->io_regs = 64;

	MSIM_SetProgmem(mcu, pm, pm_size);
	MSIM_SetDatamem(mcu, dm, dm_size);

	mcu->sreg = &mcu->data_mem[_SFR_IO8(0x3F)];
	mcu->sph = &mcu->data_mem[_SFR_IO8(0x3E)];
	mcu->spl = &mcu->data_mem[_SFR_IO8(0x3D)];

	if (set_fuse_bytes(mcu, 0xD9, 0xE1)) {
		fprintf(stderr, "Fuse bytes cannot be set correctly\n");
		return -1;
	}
	return 0;
}

static int set_fuse_bytes(struct MSIM_AVR *mcu, uint8_t high, uint8_t low)
{
	mcu->fuse[1] = high;
	mcu->fuse[0] = low;

	if (set_bldr_size(mcu, high)) {
		fprintf(stderr, "Cannot set size of bootloader!\n");
		return -1;
	}

	if (set_frequency(mcu, high, low)) {
		fprintf(stderr, "Cannoe set frequency configuration!\n");
		return -1;
	}

	if (set_reset_vector(mcu, high))
		return -1;

	return 0;
}

static int set_bldr_size(struct MSIM_AVR *mcu, uint8_t fuse_high)
{
	/* Check BOOTSZ1:0 flags and set bootloader parameters accordingly */
	switch ((fuse_high >> 1) & 0x03) {
	case 0x01:
		mcu->boot_loader->start = 0xE00;
		mcu->boot_loader->end = 0xFFF;
		mcu->boot_loader->size = 512;
		break;
	case 0x02:
		mcu->boot_loader->start = 0xF00;
		mcu->boot_loader->end = 0xFFF;
		mcu->boot_loader->size = 256;
		break;
	case 0x03:
		mcu->boot_loader->start = 0xF80;
		mcu->boot_loader->end = 0xFFF;
		mcu->boot_loader->size = 128;
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

static int set_frequency(struct MSIM_AVR *mcu, uint8_t fuse_high,
			 uint8_t fuse_low)
{
	uint8_t cksel_f, ckopt_f;

	ckopt_f = (fuse_high >> 4) & 0x01;
	cksel_f = fuse_low & 0x0F;
	switch(cksel_f) {
	case 0x02:					/* Internal, 2 MHz */
		if (is_ckopt_programmed(ckopt_f))
			return -1;
		mcu->clk_source = AVR_INT_CLK;
		mcu->freq = 2000;
		break;
	case 0x03:					/* Internal, 4 MHz */
		if (is_ckopt_programmed(ckopt_f))
			return -1;
		mcu->clk_source = AVR_INT_CLK;
		mcu->freq = 4000;
		break;
	case 0x04:					/* Internal, 8 MHz */
		if (is_ckopt_programmed(ckopt_f))
			return -1;
		mcu->clk_source = AVR_INT_CLK;
		mcu->freq = 8000;
		break;
	case 0x01:
	default:					/* Internal, 1 MHz */
		if (is_ckopt_programmed(ckopt_f))
			return -1;
		mcu->clk_source = AVR_INT_CLK;
		mcu->freq = 1000;
		break;
	case 0x00:
		/* External Clock */
		mcu->clk_source = AVR_EXT_CLK;
		mcu->freq = UINT32_MAX;
		break;
	}
	return 0;
}

static int is_ckopt_programmed(uint8_t ckopt_f)
{
	if (!ckopt_f) {
		fprintf(stderr, "CKOPT fuse bit should be unprogrammed "
				"(CKOPT == 1) using internal clock source\n");
		return -1;
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
