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

static void tick_timer0(struct MSIM_AVR *mcu);

int MSIM_M328PInit(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args)
{
#include "mcusim/avr/sim/mcu_init.h"
	return 0;
}

int MSIM_M328PTickTimers(void *m)
{
	struct MSIM_AVR *mcu;

	mcu = (struct MSIM_AVR *)m;
	tick_timer0(mcu);

	return 0;
}

static void tick_timer0(struct MSIM_AVR *mcu){
	
	static unsigned int tc0_presc;
	static unsigned int tc0_ticks;
	unsigned char cs0;			/* Clock Select bits CS0[2:0] */
	/* TODO: Wave form generation WGM */
	unsigned int presc;			/* Prescaler value */

	cs0 = mcu->dm[TCCR0B]&0x7;	/* &00000111 */

	switch (cs0) {
	case 0x1:
		presc = 1;			/* No prescaling, clk_io */
		break;
	case 0x2:
		presc = 8;			/* clk_io/8 */
		break;
	case 0x3:
		presc = 64;			/* clk_io/64 */
		break;
	case 0x4:
		presc = 256;		/* clk_io/256 */
		break;
	case 0x5:
		presc = 1024;		/* clk_io/1024 */
		break;
	case 0x6:				/* Ext. clock on T0(PD4) (fall) */
		if (IS_FALL(init_portd, mcu->dm[PORTD], PD4) ||
		                IS_FALL(init_pind, mcu->dm[PIND], PD4)) {
			if (mcu->dm[TCNT0] == 0xFF) {
				mcu->dm[TCNT0] = 0;			/* Reset Timer/Counter0 */
				mcu->dm[TIFR] |= (1<<TOV0);	/* Timer/Counter0 overflow occured */
			} else {						/* Count UP on tick */
				mcu->dm[TCNT0]++;
			}
		}
		tc0_presc = 0;
		tc0_ticks = 0;
		return;
	case 0x7:				/* Ext. clock on T0(PD4) (rise) */
		if (IS_RISE(init_portd, mcu->dm[PORTD], PD4) ||
		                IS_RISE(init_pind, mcu->dm[PIND], PD4)) {
			if (mcu->dm[TCNT0] == 0xFF) {
				mcu->dm[TCNT0] = 0;			/* Reset Timer/Counter0 */
				mcu->dm[TIFR] |= (1<<TOV0);	/* Timer/Counter0 overflow occured */
			} else {						/* Count UP on tick */
				mcu->dm[TCNT0]++;
			}
		}
		tc0_presc = 0;
		tc0_ticks = 0;
		return;
	case 0x0:			/* No clock source (stopped mode) */
	default:			/* Should not happen! */
		tc0_presc = 0;
		tc0_ticks = 0;
		return;
	}
		/* Internal clock */
	if (presc != tc0_presc) {
		tc0_presc = presc;
		tc0_ticks = 0;
	}
		/* Timer Counting mechanism */	
	if (tc0_ticks == (tc0_presc-1)) {
		if (mcu->dm[TCNT0] == 0xFF) {
			mcu->dm[TCNT0] = 0;			/* Reset Timer/Counter0 */
			mcu->dm[TIFR] |= (1<<TOV0);	/* Timer/Counter0 overflow occured */
		} else {						/* Count UP on tick */
			mcu->dm[TCNT0]++;
		}
		tc0_ticks = 0;					/* Calculate next tick */			
		return;
	}
	tc0_ticks++;
}

int MSIM_M328PSetFuse(void *m, unsigned int fuse_n, unsigned char fuse_v)
{
	struct MSIM_AVR *mcu;
	unsigned char cksel, bootsz;

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
		/* 2 - Reserved */
		if (cksel == 0) {
			mcu->clk_source = AVR_EXT_CLK;
		} else if (cksel == 1) {
			printf("[!]: Fuse #%u is reserved on %s\n",
			       fuse_v, mcu->name);
			return -1;
		} else if (cksel == 2) {
			mcu->clk_source = AVR_INT_CAL_RC_CLK;
			mcu->freq = 8000000;	/* max 8 MHz */
		} else if (cksel == 3) {
			mcu->clk_source = AVR_INT_128K_RC_CLK;
			mcu->freq = 128000000;	/* max 128 kHz */
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
		} else if	(cksel >= 8 && cksel <= 15) {
			mcu->clk_source = AVR_LOWP_CRYSTAL_CLK;
			cksel = cksel&0x1;
			switch (cksel) {
			case 8:
				mcu->freq = 900000;	/* max 0.9 MHz */
				break;
			case 10:
				mcu->freq = 3000000; /* max 3 MHz */
				break;
			case 12:
				mcu->freq = 8000000; /* max 8MHz */
				break;
			case 14:
				mcu->freq = 16000000; /* max 16 MHz */
			}
		}
		break;
	case FUSE_HIGH:
		bootsz = (fuse_v>>1)&0x3;
		switch (bootsz) {
		case 3:
			mcu->bls->start = 0x3F00;
			mcu->bls->end = 0x3FFF;
			mcu->bls->size = 256;
			break;
		case 2:
			mcu->bls->start = 0x3E00;
			mcu->bls->end = 0x3FFF;
			mcu->bls->size = 512;
			break;
		case 1:
			mcu->bls->start = 0x3C00;
			mcu->bls->end = 0x3FFF;
			mcu->bls->size = 1024;
			break;
		case 0:
			mcu->bls->start = 0x3800;
			mcu->bls->end = 0x3FFF;
			mcu->bls->size = 2048;
			break;
		}

		if (fuse_v&0x1)
			mcu->intr->reset_pc = mcu->pc = 0x0000;
		else
			mcu->intr->reset_pc = mcu->pc = mcu->bls->start;

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

