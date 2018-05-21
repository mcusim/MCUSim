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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mcusim/avr/sim/simm328p.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/sim.c"

int MSIM_M328PInit(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args)
{
#include "mcusim/avr/sim/mcu_init.h"
	return 0;
}

int MSIM_M328PSetFuse(void *m, unsigned int fuse_n, unsigned char fuse_v)
{
	struct MSIM_AVR *mcu;
	unsigned char cksel, bootsz, ckopt;

	mcu = (struct MSIM_AVR *)m;
	if (fuse_n > 1) {
		fprintf(stderr, "[!]: Fuse #%u is not supported by %s\n",
		        fuse_n, mcu->name);
		return -1;
	}

	mcu->fuse[fuse_n] = fuse_v;
	cksel = mcu->fuse[0]&0xF;
	ckopt = (mcu->fuse[1]>>4)&0x1;

	switch (fuse_n) {
	case FUSE_LOW:
		cksel = fuse_v&0xF;
		/* 2 - Reserved */ 
		if (cksel == 0) {
			mcu->clk_source = AVR_EXT_CLK;
		} else if (cksel == 2) {
			mcu->clk_source = AVR_RESERVED_CLK; 
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
			mcu->intr->reset_pc = mcu->pc = 0x0000;
		else			/* BOOTRST is 0(programmed) */
			mcu->intr->reset_pc = mcu->pc = mcu->bls->start;

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


