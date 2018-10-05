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

#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simm328p.h"

/* The OCR0 Compare Register is double buffered when using any of
 * the PWM modes and updated during either TOP or BOTTOM of the counting
 * sequence. */
static unsigned char ocr0_buf;
/* Timer may start counting from a value higher then the one on OCR0 and
 * for that reason misses the Compare Match. This flag is set in this case. */
static unsigned char missed_cm = 0;

static void tick_timer0(struct MSIM_AVR *mcu);
static void timer0_normal(struct MSIM_AVR *mcu,
                          unsigned int presc, unsigned int *ticks,
                          unsigned char wgm0, unsigned char com0);
static void timer0_pcpwm(struct MSIM_AVR *mcu,
                         unsigned int presc, unsigned int *ticks,
                         unsigned char wgm0, unsigned char com0);

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
	static unsigned char old_wgm2; /*should it have initial value? */
	unsigned char cs0;			/* Clock Select bits CS0[2:0] */
	unsigned char wgm0;							/* TODO: Wave form generation WGM */
	unsigned int presc;			/* Prescaler value */

	cs0 = mcu->dm[TCCR0B]&0x7;	/* &00000111 */
	wgm0 = ((mcu->dm[TCCR0A]>>WGM00)&1) | ((mcu->dm[TCCR0A]>>WGM01)&2) | ((mcu->dm[TCCR0B]>>WGM02)&8);

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
		if(tc0_presc == 0 && mcu->dm[TCNT0] > mcu->dm[OCR0])
			missed_cm = 1;
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
	
	switch (wgm0){
		case 0: 
			timer0_normal(mcu, tc0_presc, &tc0_ticks, wgm0, com0);
			return;
		case 1:
			timer0_pcpwm(mcu, tc0_presc, &tc0_ticks, wgm0, com0);
			return;
		case 2:
			timer0_ctc(mcu, tc0_presc, &tc0_ticks, wgm0, com0);
			return;
		case 3:
			timer0_fastpwm(mcu, tc0_presc, &tc0_ticks, wgm0, com0);
			return;
		default:
			if (wgm0 != old_wgm0) {
			fprintf(stderr, "[!]: Selected mode WGM21:20 = %u "
			        "of the Timer/Counter2 is not supported\n",
			        wgm0);
			old_wgm0 = wgm0;			
			}
			tc0_presc = 0;
			tc0_ticks = 0;
			return;
	}
}

static void timer0_normal(struct MSIM_AVR *mcu,
                          unsigned int presc, unsigned int *ticks,
                          unsigned char wgm0, unsigned char com0)
{
	if (*ticks == (presc-1)) {
		if (mcu->dm[TCNT0] == 0xFF) {
			/* Reset Timer/Counter2 */
			mcu->dm[TCNT0] = 0;
			/* Set Timer/Counter2 overflow flag */
			mcu->dm[TIFR] |= (1<<TOV0);

			//timer2_oc2_nonpwm(mcu, com0);
		} else {
			mcu->dm[TCNT0]++;
		}
		*ticks = 0;
		return;
	}
	(*ticks)++;
}

static void timer0_ctc(struct MSIM_AVR *mcu,
                       unsigned int presc, unsigned int *ticks,
                       unsigned char wgm0, unsigned char com0)
{
	/* NOTE: We're able to generate a waveform output in this mode with a
	 * frequency which is controlled by the value in OCR0A register
	 * (top border of the timer/counter). */

	if ((*ticks) == (presc-1)) {
		if (mcu->dm[TCNT0] == mcu->dm[OCR0A]) {
			/* Reset Timer/Counter0 */
			mcu->dm[TCNT0] = 0;
			/* Set Timer/Counter0 output compare flag */
			mcu->dm[TIFR] |= (1<<OCF0A);

			timer2_oc2_nonpwm(mcu, com2);
		} else {
			mcu->dm[TCNT0]++;
		}
		*ticks = 0;
		return;
	}
	(*ticks)++;
}

static void timer0_oc0_nonpwm(struct MSIM_AVR *mcu, unsigned char com0)
{
	
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

