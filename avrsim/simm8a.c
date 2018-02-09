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

static void tick_timer0(struct MSIM_AVR *mcu);
static void tick_timer2(struct MSIM_AVR *mcu);

int MSIM_M8AInit(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args)
{
#include "mcusim/avr/sim/mcu_init.h"
}

int MSIM_M8ATickTimers(void *m)
{
	struct MSIM_AVR *mcu;

	mcu = (struct MSIM_AVR *)m;
	tick_timer2(mcu);
	tick_timer0(mcu);
	return 0;
}

static void tick_timer0(struct MSIM_AVR *mcu)
{
	static unsigned int tc0_presc;
	static unsigned int tc0_ticks;

	unsigned char tccr0;
	unsigned int presc;

	tccr0 = mcu->dm[TCCR0];

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
	case 0x0:	/* No clock source (stopped mode) */
	case 0x6:	/* External clock sources are not supported (fall) */
	case 0x7:	/* External clock sources are not supported (rise) */
	default:	/* Should not happen! */
		tc0_presc = 0;
		tc0_ticks = 0;
		return;
	}

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
		fprintf(stderr, "WARN: Selected mode WGM21:20 = %u of "
				"Timer/Counter2 is not supported, normal "
				"mode will be used by default\n", wgm);
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
		fprintf(stderr, "WARN: Fuse #%d is not supported by %s\n",
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

	/* Provide Timer/Counter2 Overflow Interrupt */
	if ((timsk>>TOIE2)&1 && (tifr>>TOV2)&1) {
		mcu->intr->irq[TIMER2_OVF_vect_num-1] = 1;
		/* Clear TOV2 flag */
		mcu->dm[TIFR] &= ~(1<<TOV2);
	}

	return 0;
}
