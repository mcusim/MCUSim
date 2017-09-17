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

#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simm8a.h"

int MSIM_M8AInit(struct MSIM_AVR *mcu,
		 unsigned char *pm, unsigned long pm_size,
		 unsigned char *dm, unsigned long dm_size)
{
#include "mcusim/avr/sim/mcu_init.h"
}

int MSIM_M8ASetFuse(void *m, unsigned int fuse_n, unsigned char fuse_v)
{
	struct MSIM_AVR *mcu;
	unsigned char cksel, bootsz, ckopt;

	mcu = (struct MSIM_AVR *)m;
	if (fuse_n > 1) {
		fprintf(stderr, "Fuse #%d is not supported by %s\n",
				fuse_n, mcu->name);
		return -1;
	}

	mcu->fuse[fuse_n] = fuse_v;
	cksel = mcu->fuse[0]&0xF;
	ckopt = (mcu->fuse[1]>>4)&0x1;

	switch (fuse_n) {
	case FUSE_LOW:
		cksel = fuse_v&0xF;
		if (cksel == 0) {
			mcu->clk_source = AVR_EXT_CLK;
		} else if (cksel >= 1 && cksel <= 4) {
			mcu->clk_source = AVR_INT_CAL_RC_CLK;
			switch (cksel) {
			case 1:
				mcu->freq = 1000000;	/* 1 MHz */
				break;
			case 2:
				mcu->freq = 2000000;	/* 2 MHz */
				break;
			case 3:
				mcu->freq = 4000000;	/* 4 MHz */
				break;
			case 4:
				mcu->freq = 8000000;	/* 8 MHz */
				break;
			}
		} else if (cksel >= 5 && cksel <= 8) {
			mcu->clk_source = AVR_EXT_RC_CLK;
			switch (cksel) {
			case 5:
				mcu->freq = 900000;	/* max 0.9 MHz */
				break;
			case 6:
				mcu->freq = 3000000;	/* max 3 MHz */
				break;
			case 7:
				mcu->freq = 8000000;	/* max 4 MHz */
				break;
			case 8:
				mcu->freq = 12000000;	/* max 12 MHz */
				break;
			}
		} else if (cksel == 9) {
			mcu->clk_source = AVR_EXT_LOWF_CRYSTAL_CLK;
			mcu->freq = 32768;		/* 32.768 kHz */
		} else if (cksel >= 10 && cksel <= 15) {
			mcu->clk_source = AVR_EXT_CRYSTAL;
			cksel = (cksel>>1)&0x7;
			switch (cksel) {
			case 5:
				mcu->freq = 900000;	/* max 0.9 MHz */
				break;
			case 6:
				mcu->freq = 3000000;	/* max 3 MHz */
				break;
			case 7:
				mcu->freq = 8000000;	/* max 8 MHz */
				break;
			}
			if (!ckopt)
				mcu->freq = 16000000;	/* max 16 MHz */
		}
		break;
	case FUSE_HIGH:
		bootsz = (fuse_v>>1)&0x3;
		ckopt = (fuse_v>>4)&0x1;
		switch (bootsz) {
		case 3:
			mcu->bls->start = 0x1F00;
			mcu->bls->end = 0x1FFF;
			mcu->bls->size = 256;
			break;
		case 2:
			mcu->bls->start = 0x1E00;
			mcu->bls->end = 0x1FFF;
			mcu->bls->size = 512;
			break;
		case 1:
			mcu->bls->start = 0x1C00;
			mcu->bls->end = 0x1FFF;
			mcu->bls->size = 1024;
			break;
		case 0:
			mcu->bls->start = 0x1800;
			mcu->bls->end = 0x1FFF;
			mcu->bls->size = 2048;
			break;
		}

		if (fuse_v&0x1)		/* BOOTRST is 1(unprogrammed) */
			mcu->reset_pc = mcu->pc = 0x0000;
		else			/* BOOTRST is 0(programmed) */
			mcu->reset_pc = mcu->pc = mcu->bls->start;

		switch (cksel) {
		case 5:
			mcu->freq = 900000;	/* max 0.9 MHz */
			break;
		case 6:
			mcu->freq = 3000000;	/* max 3 MHz */
			break;
		case 7:
			mcu->freq = 8000000;	/* max 8 MHz */
			break;
		}
		if (!ckopt)
			mcu->freq = 16000000;	/* max 16 MHz */

		break;
	default:			/* Should not happen */
		return -1;
	}
	return 0;
}

int MSIM_M8ASetLock(void *m, unsigned char lock_v)
{
	return 0;
}

int MSIM_M8ATick8Timers(void *mcu)
{
	return 0;
}
