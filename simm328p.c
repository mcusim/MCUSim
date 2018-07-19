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
#include <inttypes.h>

#include "mcusim/avr/sim/simm328p.h"

int MSIM_M328PInit(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args)
{
#include "mcusim/avr/sim/mcu_init.h"
	return 0;
}

int MSIM_M328PSetFuse(void *m, unsigned int fuse_n, unsigned char fuse_v)
{
	struct MSIM_AVR *mcu;
	uint8_t cksel, bootsz;

	mcu = (struct MSIM_AVR *)m;

	if (fuse_n > 2) {
		fprintf(stderr, "[!]: Fuse #%u is not supported by %s\n",
		        fuse_n, mcu->name);
		return -1;
	}

	mcu->fuse[fuse_n] = fuse_v;
	cksel = mcu->fuse[0]&0xF;

	switch (fuse_n) {
	case FUSE_LOW:
		cksel = fuse_v&0xF;

		if (cksel == 0) {
			mcu->clk_source = AVR_EXT_CLK;
		} else if (cksel == 1) {
			printf("[!]: Fuse #%u is reserved on %s\n",
			       fuse_v, mcu->name);
			return -1;
		} else if (cksel == 2) {
			mcu->clk_source = AVR_INT_CAL_RC_CLK;
			mcu->freq = 8000000;		/* max 8 MHz */
		} else if (cksel == 3) {
			mcu->clk_source = AVR_INT_128K_RC_CLK;
			mcu->freq = 128000;		/* max 128 kHz */
		}  else if (cksel == 4 || cksel == 5) {
			mcu->clk_source = AVR_EXT_LOWF_CRYSTAL_CLK;
			switch (cksel) {
			case 4:
				mcu->freq = 1000000;	/* max 1 MHz */
				break;
			case 5:
				mcu->freq = 32768;	/* max 32,768 kHz */
				break;
			}
		} else if (cksel == 6 || cksel == 7) {
			mcu->clk_source = AVR_FULLSWING_CRYSTAL_CLK;
			mcu->freq = 20000000; /* max 20 MHz */
		} else if (cksel >= 8 && cksel <= 15) {
			mcu->clk_source = AVR_LOWP_CRYSTAL_CLK;

			/* CKSEL0 can be used to adjust start-up time and
			 * additional delay from MCU reset. */

			/* CKSEL3...1 sets frequency range in this case. */
			cksel = cksel&0xE;
			switch (cksel) {
			case 8:
				mcu->freq = 900000;	/* max 0.9 MHz */
				break;
			case 10:
				mcu->freq = 3000000;	/* max 3 MHz */
				break;
			case 12:
				mcu->freq = 8000000;	/* max 8MHz */
				break;
			case 14:
				mcu->freq = 16000000;	/* max 16 MHz */
				break;
			}
		} else {
			fprintf(stderr, "[e]: CKSEL = %" PRIu8 ", but it "
			        "should be in [0, 15] inclusively!\n", cksel);
		}
		break;
	case FUSE_HIGH:
		bootsz = (fuse_v>>1)&0x3;
		switch (bootsz) {
		case 3:
			mcu->bls->start = 0x7E00;	/* first byte */
			mcu->bls->end = 0x7FFF;		/* last byte */
			mcu->bls->size = 512;		/* bytes! */
			break;
		case 2:
			mcu->bls->start = 0x7C00;
			mcu->bls->end = 0x7FFF;
			mcu->bls->size = 1024;		/* bytes! */
			break;
		case 1:
			mcu->bls->start = 0x7800;
			mcu->bls->end = 0x7FFF;
			mcu->bls->size = 2048;		/* bytes! */
			break;
		case 0:
			mcu->bls->start = 0x7000;
			mcu->bls->end = 0x7FFF;
			mcu->bls->size = 4096;		/* bytes! */
			break;
		}

		if (fuse_v&0x1) {
			mcu->intr->reset_pc = mcu->pc = 0x0000;
		} else {
			mcu->intr->reset_pc = mcu->pc = mcu->bls->start;
		}

		break;
	case FUSE_EXT:

		break;
	default:		/* Should not happen */
		return -1;
	}

	return 0;
}

int MSIM_M328PSetLock(void *mcu, unsigned char lock_v)
{
	/* It's waiting to be implemented. */
	return 0;
}

