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
#include "mcusim/avr/sim/simm8a.h"

#define EXT_CLK_NONE		0
#define EXT_CLK_RISE		1
#define EXT_CLK_FALL		2

#define IS_SET(byte, bit)	(((byte)&(1UL<<(bit)))>>(bit))
#define IS_RISE(init, val, bit)	((!((init>>bit)&1)) & ((val>>bit)&1))
#define IS_FALL(init, val, bit)	(((init>>bit)&1) & (!((val>>bit)&1)))

static unsigned char init_pd;		/* Initial port D value to track
					   T0/PD4 and T1/PD5. */
static unsigned char init_pb;		/* Initial port B value to track
					   TOSC1/PB6 and TOSC2/PB7. */


static void tick_timer0(struct MSIM_AVR *mcu);
static void tick_timer1(struct MSIM_AVR *mcu);
static void tick_timer2(struct MSIM_AVR *mcu);

int MSIM_M8AInit(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args)
{
#include "mcusim/avr/sim/mcu_init.h"
	/* Keep initial port values */
	init_pd = mcu->dm[PORTD];
	init_pb = mcu->dm[PORTB];
	return 0;
}

int MSIM_M8ATickTimers(void *m)
{
	struct MSIM_AVR *mcu;

	mcu = (struct MSIM_AVR *)m;
	tick_timer2(mcu);
	tick_timer1(mcu);
	tick_timer0(mcu);

	init_pd = mcu->dm[PORTD];
	init_pb = mcu->dm[PORTB];
	return 0;
}

static void tick_timer0(struct MSIM_AVR *mcu)
{
	static unsigned int tc0_presc;
	static unsigned int tc0_ticks;
	unsigned char tccr0;		/* Timer/Counter0 control register */
	unsigned int presc;		/* Prescaler value */
	unsigned char extclk;		/* External clock source flag */

	tccr0 = mcu->dm[TCCR0];
	extclk = EXT_CLK_NONE;
	presc = 0;

	switch (tccr0) {
	case 0x1:
		presc = 0;		/* No prescaling, clk_io */
		break;
	case 0x2:
		presc = 8;		/* clk_io/8 */
		break;
	case 0x3:
		presc = 64;		/* clk_io/64 */
		break;
	case 0x4:
		presc = 256;		/* clk_io/256 */
		break;
	case 0x5:
		presc = 1024;		/* clk_io/1024 */
		break;
	case 0x6:			/* Ext. clock on T0/PD4 (fall) */
		extclk = EXT_CLK_FALL;
		tc0_presc = 0;
		tc0_ticks = 0;
		break;
	case 0x7:			/* Ext. clock on T0/PD4 (rise) */
		extclk = EXT_CLK_RISE;
		tc0_presc = 0;
		tc0_ticks = 0;
		break;
	case 0x0:			/* No clock source (stopped mode) */
	default:			/* Should not happen! */
		tc0_presc = 0;
		tc0_ticks = 0;
		return;
	}

	/* External clock */
	if (extclk && (((extclk == EXT_CLK_FALL) &&
	                IS_FALL(init_pd, mcu->dm[PORTD], PD4)) ||
	                ((extclk == EXT_CLK_RISE) &&
	                 IS_RISE(init_pd, mcu->dm[PORTD], PD4)))) {
		if (mcu->dm[TCNT0] == 0xFF) {
			/* Reset Timer/Counter0 */
			mcu->dm[TCNT0] = 0;
			/* Timer/Counter0 overflow occured */
			mcu->dm[TIFR] |= (1<<TOV0);
		} else {
			mcu->dm[TCNT0]++;
		}
		return;
	}

	/* Internal clock */
	if (presc != tc0_presc) {
		tc0_presc = presc;
		tc0_ticks = 0;
	}
	if (tc0_ticks == (tc0_presc-1)) {
		if (mcu->dm[TCNT0] == 0xFF) {
			/* Reset Timer/Counter0 */
			mcu->dm[TCNT0] = 0;
			/* Timer/Counter0 overflow occured */
			mcu->dm[TIFR] |= (1<<TOV0);
		} else {
			mcu->dm[TCNT0]++;
		}
		tc0_ticks = 0;
		return;
	}
	tc0_ticks++;
}

static void tick_timer1(struct MSIM_AVR *mcu)
{
	/* ... */
}

static void tick_timer2(struct MSIM_AVR *mcu)
{
	static unsigned int tc2_presc;
	static unsigned int tc2_ticks;
	static unsigned char prev_wgm;

	unsigned char tccr2;
	unsigned char cs;		/* Clock Select bits CS22:CS20 */
	unsigned char wgm;		/* Waveform Generation WGM21:20 */
	unsigned int presc;

	tccr2 = mcu->dm[TCCR2];
	cs = tccr2 & 0x7;
	wgm = (((tccr2>>WGM21)<<1)&2) | ((tccr2>>WGM20)&1);

	/*
	 * There are several modes of operation available.
	 *
	 * - (supported) The simplest mode is a normal one, when counter
	 *   is incremented only and no counter clear is performed,
	 *   WGM21:20 = 0;
	 *
	 * - (planned) Phase Correct PWM Mode, WGM21:20 = 1.
	 * - (planned) Clear Timer on Compare Match (CTC) Mode, WGM21:20 = 2;
	 * - (planned) Fast PWM Mode, WGM21:20 = 3;
	 */
	if (wgm > 0 && wgm != prev_wgm) {
		fprintf(stderr, "[!]: Selected mode WGM21:20 = %u of "
		        "Timer/Counter2 is not supported, normal mode "
		        "will be used by default\n", wgm);
		prev_wgm = wgm;
	}

	switch (cs) {
	case 0x1:
		presc = 0;		/* No prescaling, clk_io */
		break;
	case 0x2:
		presc = 8;		/* clk_io/8 */
		break;
	case 0x3:
		presc = 32;		/* clk_io/32 */
		break;
	case 0x4:
		presc = 64;		/* clk_io/64 */
		break;
	case 0x5:
		presc = 128;		/* clk_io/128 */
		break;
	case 0x6:
		presc = 256;		/* clk_io/256 */
		break;
	case 0x7:
		presc = 1024;		/* clk_io/1024 */
		break;
	case 0x0:	/* No clock source (stopped mode) */
	default:	/* Should not happen! */
		tc2_presc = 0;
		tc2_ticks = 0;
		return;
	}

	if (presc != tc2_presc) {
		tc2_presc = presc;
		tc2_ticks = 0;
	}
	if (tc2_ticks == (tc2_presc-1)) {
		if (mcu->dm[TCNT2] == 0xFF) {
			/* Reset Timer/Counter2 */
			mcu->dm[TCNT2] = 0;
			/* Timer/Counter2 overflow occured */
			mcu->dm[TIFR] |= (1<<TOV2);
		} else {
			mcu->dm[TCNT2]++;
		}
		tc2_ticks = 0;
		return;
	}
	tc2_ticks++;
}

int MSIM_M8ASetFuse(void *m, unsigned int fuse_n, unsigned char fuse_v)
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

int MSIM_M8ASetLock(void *m, unsigned char lock_v)
{
	/* Waiting to be implemented */
	return 0;
}

int MSIM_M8AProvideIRQs(void *m)
{
	struct MSIM_AVR *mcu;
	unsigned char timsk, tifr;

	mcu = (struct MSIM_AVR *)m;
	timsk = mcu->dm[TIMSK];
	tifr = mcu->dm[TIFR];

	/* Provide Timer/Counter0 Overflow Interrupt */
	if ((timsk>>TOIE0)&1 && (tifr>>TOV0)&1) {
		mcu->intr->irq[TIMER0_OVF_vect_num-1] = 1;
		/* Clear TOV0 flag */
		mcu->dm[TIFR] &= ~(1<<TOV0);
	}

	/* Provide TIMER2 OVF */
	if ((timsk>>TOIE2)&1 && (tifr>>TOV2)&1) {
		mcu->intr->irq[TIMER2_OVF_vect_num-1] = 1;
		/* Clear TOV2 flag */
		mcu->dm[TIFR] &= ~(1<<TOV2);
	}
	/* Provide TIMER2 COMP */
	/* ... */

	return 0;
}
