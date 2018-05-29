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
 *
 * Atmel ATmega8A-specific functions implemented here. You may find it
 * useful to take a look at the "simm8a.h" header first. Feel free to
 * add missing features of the microcontroller or fix bugs.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simm8a.h"

#define COMPARE_MATCH		75
#define SET_TO_BOTTOM		76
#define COMP_MATCH_UPCNT	77
#define COMP_MATCH_DOWNCNT	78

/* Initial PORTD and PIND values and to track T0/PD4 and T1/PD5. */
static unsigned char init_portd;
static unsigned char init_pind;
/* Initial PORTB and PINB value to track TOSC1/PB6 and TOSC2/PB7. */
static unsigned char init_portb;
static unsigned char init_pinb;

/* The OCR2 Compare Register is double buffered when using any of
 * the PWM modes and updated during either TOP or BOTTOM of the counting
 * sequence. */
static unsigned char ocr2_buf;
/* Timer may start counting from a value higher then the one on OCR2 and
 * for that reason misses the Compare Match. This flag is set in this case. */
static unsigned char missed_cm = 0;

static void tick_timer0(struct MSIM_AVR *mcu);
static void tick_timer1(struct MSIM_AVR *mcu);
static void tick_timer2(struct MSIM_AVR *mcu);
static void update_watched_values(struct MSIM_AVR *mcu);

/* Timer/Counter2 modes of operation */
static void timer2_normal(struct MSIM_AVR *mcu,
                          unsigned int presc, unsigned int *ticks,
                          unsigned char wgm2, unsigned char com2);
static void timer2_ctc(struct MSIM_AVR *mcu,
                       unsigned int presc, unsigned int *ticks,
                       unsigned char wgm2, unsigned char com2);
static void timer2_fastpwm(struct MSIM_AVR *mcu,
                           unsigned int presc, unsigned int *ticks,
                           unsigned char wgm2, unsigned char com2);
static void timer2_pcpwm(struct MSIM_AVR *mcu,
                         unsigned int presc, unsigned int *ticks,
                         unsigned char wgm2, unsigned char com2);

static void timer2_oc2_nonpwm(struct MSIM_AVR *mcu, unsigned char com2);
static void timer2_oc2_fastpwm(struct MSIM_AVR *mcu, unsigned char com2,
                               unsigned char state);
static void timer2_oc2_pcpwm(struct MSIM_AVR *mcu, unsigned char com2,
                             unsigned char state);

int MSIM_M8AInit(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args)
{
#include "mcusim/avr/sim/mcu_init.h"

	update_watched_values(mcu);
	/* I/O ports have internal pull-up resistors (selected for each bit) */
	mcu->dm[PORTB] = 0xFF;
	mcu->dm[PORTC] = 0xFF;
	mcu->dm[PORTD] = 0xFF;
	return 0;
}

int MSIM_M8ATickTimers(void *m)
{
	struct MSIM_AVR *mcu;

	mcu = (struct MSIM_AVR *)m;
	tick_timer2(mcu);
	tick_timer1(mcu);
	tick_timer0(mcu);
	update_watched_values(mcu);
	return 0;
}

static void update_watched_values(struct MSIM_AVR *mcu)
{
	init_portd = mcu->dm[PORTD];
	init_pind = mcu->dm[PIND];
	init_portb = mcu->dm[PORTB];
	init_pinb = mcu->dm[PINB];
}

static void tick_timer0(struct MSIM_AVR *mcu)
{
	static unsigned int tc0_presc;
	static unsigned int tc0_ticks;
	unsigned char tccr0;		/* Timer/Counter0 control register */
	unsigned int presc;		/* Prescaler value */

	tccr0 = mcu->dm[TCCR0];
	presc = 0;

	switch (tccr0) {
	case 0x1:
		presc = 1;		/* No prescaling, clk_io */
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
		if (IS_FALL(init_portd, mcu->dm[PORTD], PD4) ||
		                IS_FALL(init_pind, mcu->dm[PIND], PD4)) {
			if (mcu->dm[TCNT0] == 0xFF) {
				/* Reset Timer/Counter0 */
				mcu->dm[TCNT0] = 0;
				/* Timer/Counter0 overflow */
				mcu->dm[TIFR] |= (1<<TOV0);
			} else {
				mcu->dm[TCNT0]++;
			}
		}
		tc0_presc = 0;
		tc0_ticks = 0;
		return;
	case 0x7:			/* Ext. clock on T0/PD4 (rise) */
		if (IS_RISE(init_portd, mcu->dm[PORTD], PD4) ||
		                IS_RISE(init_pind, mcu->dm[PIND], PD4)) {
			if (mcu->dm[TCNT0] == 0xFF) {
				/* Reset Timer/Counter0 */
				mcu->dm[TCNT0] = 0;
				/* Timer/Counter0 overflow */
				mcu->dm[TIFR] |= (1<<TOV0);
			} else {
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
	/* There are several modes of operation available:
	 * - (supported) The simplest mode is a normal one, when counter
	 *               is incremented only and no counter clear is performed,
	 *               WGM21:0 = 0;
	 * - (supported) Clear Timer on Compare Match (CTC) Mode, WGM21:0 = 2;
	 * - (supported) Fast PWM Mode, WGM21:0 = 3.
	 * - (supported) Phase Correct PWM Mode, WGM21:0 = 1;
	 */
	static unsigned int tc2_presc;
	static unsigned int tc2_ticks;
	static unsigned char old_wgm2;
	unsigned char cs2;		/* Clock Select bits CS22:0 */
	unsigned char wgm2;		/* Waveform Generation WGM21:0 */
	unsigned char com2;		/* Compare Output mode COM21:0 */
	unsigned int presc;		/* Prescaler */

	cs2 = mcu->dm[TCCR2]&0x7;
	wgm2 = (((mcu->dm[TCCR2]>>WGM21)<<1)&2) | ((mcu->dm[TCCR2]>>WGM20)&1);
	com2 = (((mcu->dm[TCCR2]>>COM21)<<1)&2) | ((mcu->dm[TCCR2]>>COM20)&1);

	switch (cs2) {
	case 0x1:
		presc = 1;		/* No prescaling, clk_io */
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
		/* We may have TC2 enabled with Compare Match missed. */
		if (tc2_presc == 0 && mcu->dm[TCNT2] > mcu->dm[OCR2])
			missed_cm = 1;

		tc2_presc = presc;
		/* Should we really clean these ticks? */
		tc2_ticks = 0;
	} else if (missed_cm) {
		missed_cm = 0;
	}

	switch (wgm2) {
	case 0:
		timer2_normal(mcu, tc2_presc, &tc2_ticks, wgm2, com2);
		return;
	case 2:
		timer2_ctc(mcu, tc2_presc, &tc2_ticks, wgm2, com2);
		return;
	case 3:
		timer2_fastpwm(mcu, tc2_presc, &tc2_ticks, wgm2, com2);
		return;
	case 1:
		timer2_pcpwm(mcu, tc2_presc, &tc2_ticks, wgm2, com2);
		return;
	default:			/* Should not happen! */
		if (wgm2 != old_wgm2) {
			fprintf(stderr, "[!]: Selected mode WGM21:20 = %u "
			        "of the Timer/Counter2 is not supported\n",
			        wgm2);
			old_wgm2 = wgm2;
		}
		tc2_presc = 0;
		tc2_ticks = 0;
		return;
	}
}

static void timer2_normal(struct MSIM_AVR *mcu,
                          unsigned int presc, unsigned int *ticks,
                          unsigned char wgm2, unsigned char com2)
{
	if (*ticks == (presc-1)) {
		if (mcu->dm[TCNT2] == 0xFF) {
			/* Reset Timer/Counter2 */
			mcu->dm[TCNT2] = 0;
			/* Set Timer/Counter2 overflow flag */
			mcu->dm[TIFR] |= (1<<TOV2);

			timer2_oc2_nonpwm(mcu, com2);
		} else {
			mcu->dm[TCNT2]++;
		}
		*ticks = 0;
		return;
	}
	(*ticks)++;
}

static void timer2_ctc(struct MSIM_AVR *mcu,
                       unsigned int presc, unsigned int *ticks,
                       unsigned char wgm2, unsigned char com2)
{
	/* NOTE: We're able to generate a waveform output in this mode with a
	 * frequency which is controlled by the value in OCR2 register
	 * (top border of the timer/counter). */

	if ((*ticks) == (presc-1)) {
		if (mcu->dm[TCNT2] == mcu->dm[OCR2]) {
			/* Reset Timer/Counter2 */
			mcu->dm[TCNT2] = 0;
			/* Set Timer/Counter2 output compare flag */
			mcu->dm[TIFR] |= (1<<OCF2);

			timer2_oc2_nonpwm(mcu, com2);
		} else {
			mcu->dm[TCNT2]++;
		}
		*ticks = 0;
		return;
	}
	(*ticks)++;
}

static void timer2_fastpwm(struct MSIM_AVR *mcu,
                           unsigned int presc, unsigned int *ticks,
                           unsigned char wgm2, unsigned char com2)
{
	/* NOTE: This mode allows PWM to be generated using single-slope
	 * operation. Duty cycle can be controlled by the value in
	 * OCR2 register. */

	/* Single-slope operation means counting from BOTTOM(0) to MAX(255),
	 * reset back to BOTTOM in the following clock cycle and
	 * start again. */
	if ((*ticks) == (presc-1)) {
		if (mcu->dm[TCNT2] == 0xFF) {
			mcu->dm[TCNT2] = 0;
			ocr2_buf = mcu->dm[OCR2];
			timer2_oc2_fastpwm(mcu, com2, SET_TO_BOTTOM);
			mcu->dm[TIFR] |= (1<<TOV2);
		} else if (mcu->dm[TCNT2] == ocr2_buf) {
			mcu->dm[TIFR] |= (1<<OCF2);
			timer2_oc2_fastpwm(mcu, com2, COMPARE_MATCH);
			mcu->dm[TCNT2]++;
		} else {
			mcu->dm[TCNT2]++;
		}
		*ticks = 0;
		return;
	}
	(*ticks)++;
}

static void timer2_pcpwm(struct MSIM_AVR *mcu,
                         unsigned int presc, unsigned int *ticks,
                         unsigned char wgm2, unsigned char com2)
{
	/* NOTE: This mode allows PWM to be generated using dual-slope
	 * operation (preferred for motor control applications). Duty cycle
	 * can be controlled by the value in OCR2 register. */
	static unsigned char cnt_down = 0;

	/* Dual-slope operation means counting from BOTTOM(0) to MAX(255),
	 * then from MAX back to the BOTTOM and start again. */
	if ((*ticks) == (presc-1)) {
		if (mcu->dm[TCNT2] == 0xFF) {
			if (ocr2_buf == 0xFF)
				mcu->dm[TIFR] |= (1<<OCF2);
			if (missed_cm || (ocr2_buf == 0xFF &&
			                  mcu->dm[OCR2] < 0xFF))
				timer2_oc2_pcpwm(mcu, com2, COMP_MATCH_UPCNT);

			cnt_down = 1;
			ocr2_buf = mcu->dm[OCR2];
			if (ocr2_buf == 0xFF)
				timer2_oc2_pcpwm(mcu, com2,
				                 COMP_MATCH_DOWNCNT);
			mcu->dm[TCNT2]--;
		} else if (mcu->dm[TCNT2] == 0) {
			cnt_down = 0;
			mcu->dm[TIFR] |= (1<<TOV2);
			mcu->dm[TCNT2]++;
		} else if (mcu->dm[TCNT2] == ocr2_buf) {
			mcu->dm[TIFR] |= (1<<OCF2);
			timer2_oc2_pcpwm(mcu, com2, cnt_down
			                 ? COMP_MATCH_DOWNCNT
			                 : COMP_MATCH_UPCNT);
			if (!cnt_down)
				mcu->dm[TCNT2]++;
			else
				mcu->dm[TCNT2]--;
		} else {
			if (!cnt_down)
				mcu->dm[TCNT2]++;
			else
				mcu->dm[TCNT2]--;
		}
		*ticks = 0;
		return;
	}
	(*ticks)++;
}

static void timer2_oc2_nonpwm(struct MSIM_AVR *mcu, unsigned char com2)
{
	/* Check Data Direction Register first. DDRB3 should be set to
	 * enable the output driver (according to a datasheet).*/
	if (!IS_SET(mcu->dm[DDRB], PB3))
		return;

	/* Update Output Compare pin (OC2) */
	switch (com2) {
	case 1:
		if (IS_SET(mcu->dm[PORTB], PB3))
			CLEAR(mcu->dm[PORTB], PB3);
		else
			SET(mcu->dm[PORTB], PB3);
		break;
	case 2:
		CLEAR(mcu->dm[PORTB], PB3);
		break;
	case 3:
		SET(mcu->dm[PORTB], PB3);
		break;
	case 0:
	default:
		/* OC2 disconnected, do nothing */
		break;
	}
}

static void timer2_oc2_fastpwm(struct MSIM_AVR *mcu, unsigned char com2,
                               unsigned char state)
{
	/* Check Data Direction Register first. DDRB3 should be set to
	 * enable the output driver (according to a datasheet).*/
	if (!IS_SET(mcu->dm[DDRB], PB3))
		return;

	/* Update Output Compare pin (OC2) */
	switch (com2) {
	case 1:
		if (state == COMPARE_MATCH) {
			if (IS_SET(mcu->dm[PORTB], PB3))
				CLEAR(mcu->dm[PORTB], PB3);
			else
				SET(mcu->dm[PORTB], PB3);
		}
		break;
	case 2:		/* Non-inverting compare output mode */
		if (state == COMPARE_MATCH)
			CLEAR(mcu->dm[PORTB], PB3);
		else
			SET(mcu->dm[PORTB], PB3);
		break;
	case 3:		/* Inverting compare output mode */
		if (state == COMPARE_MATCH)
			SET(mcu->dm[PORTB], PB3);
		else
			CLEAR(mcu->dm[PORTB], PB3);
		break;
	case 0:
	default:
		/* OC2 disconnected, do nothing */
		break;
	}
}

static void timer2_oc2_pcpwm(struct MSIM_AVR *mcu, unsigned char com2,
                             unsigned char state)
{
	/* Check Data Direction Register first. DDRB3 should be set to
	 * enable the output driver (according to a datasheet).*/
	if (!IS_SET(mcu->dm[DDRB], PB3))
		return;

	/* Update Output Compare pin (OC2) */
	switch (com2) {
	case 1:
		fprintf(stderr, "[!] COM21:0=1 is reserved in the phase "
		        "correct PWM mode\n");
		break;
	case 2:		/* Non-inverting compare output mode */
		if (state == COMP_MATCH_UPCNT)
			CLEAR(mcu->dm[PORTB], PB3);
		else
			SET(mcu->dm[PORTB], PB3);
		break;
	case 3:		/* Inverting compare output mode */
		if (state == COMP_MATCH_UPCNT)
			SET(mcu->dm[PORTB], PB3);
		else
			CLEAR(mcu->dm[PORTB], PB3);
		break;
	case 0:
	default:
		/* OC2 disconnected, do nothing */
		break;
	}
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

	/* TIMER0 OVF - Timer/Counter0 Overflow */
	if ((timsk>>TOIE0)&1 && (tifr>>TOV0)&1) {
		mcu->intr->irq[TIMER0_OVF_vect_num-1] = 1;
		/* Clear TOV0 flag */
		mcu->dm[TIFR] &= ~(1<<TOV0);
	}

	/* TIMER2 OVF - Timer/Counter2 Overflow */
	if ((timsk>>TOIE2)&1 && (tifr>>TOV2)&1) {
		mcu->intr->irq[TIMER2_OVF_vect_num-1] = 1;
		/* Clear TOV2 flag */
		mcu->dm[TIFR] &= ~(1<<TOV2);
	}
	/* TIMER2 COMP - Timer/Counter2 Compare Match */
	if ((timsk>>OCIE2)&1 && (tifr>>OCF2)&1) {
		mcu->intr->irq[TIMER2_COMP_vect_num-1] = 1;
		/* Clear OCF2 flag */
		mcu->dm[TIFR] &= ~(1<<OCF2);
	}

	return 0;
}
