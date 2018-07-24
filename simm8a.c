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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simm8a.h"
#include "mcusim/avr/sim/mcu_init.h"

#define NOT_CONNECTED		0xFFU
#define COMPARE_MATCH		75
#define SET_TO_BOTTOM		76
#define COMP_MATCH_UPCNT	77
#define COMP_MATCH_DOWNCNT	78

/* Two arbitrary constants to mark two distinct output compare channels
 * of the microcontroller. */
#define A_CHAN			79
#define B_CHAN			80

#define DM(v)			(mcu->dm[v])

/* Initial PORTD and PIND values and to track T0/PD4 and T1/PD5. */
static uint8_t init_portd;
static uint8_t init_pind;
/* Initial PORTB and PINB value to track TOSC1/PB6 and TOSC2/PB7. */
static uint8_t init_portb;
static uint8_t init_pinb;

/* The OCR2 Compare Register is double buffered when using any of
 * the PWM modes and updated during either TOP or BOTTOM of the counting
 * sequence. */
static uint8_t ocr2_buf;
/* Timer may start counting from a value higher then the one stored in OCR2
 * and for that reason misses the Compare Match. This flag is set in
 * this case. */
static uint8_t missed_cm = 0;

static void tick_timer0(struct MSIM_AVR *mcu);
static void tick_timer1(struct MSIM_AVR *mcu);
static void tick_timer2(struct MSIM_AVR *mcu);
static void update_watched_values(struct MSIM_AVR *mcu);

/* Timer/Counter1 modes of operation */
static void timer1_normal(struct MSIM_AVR *mcu, uint32_t presc,
                          uint32_t *ticks, uint8_t wgm1, uint8_t com1a,
                          uint8_t com1b);

static void timer1_oc1_nonpwm(struct MSIM_AVR *mcu, uint8_t com1a,
                              uint8_t com1b, uint8_t chan);

/* Timer/Counter2 modes of operation */
static void timer2_normal(struct MSIM_AVR *mcu,
                          uint32_t presc, uint32_t *ticks,
                          uint8_t wgm2, uint8_t com2);
static void timer2_ctc(struct MSIM_AVR *mcu,
                       uint32_t presc, uint32_t *ticks,
                       uint8_t wgm2, uint8_t com2);
static void timer2_fastpwm(struct MSIM_AVR *mcu,
                           uint32_t presc, uint32_t *ticks,
                           uint8_t wgm2, uint8_t com2);
static void timer2_pcpwm(struct MSIM_AVR *mcu,
                         uint32_t presc, uint32_t *ticks,
                         uint8_t wgm2, uint8_t com2);

static void timer2_oc2_nonpwm(struct MSIM_AVR *mcu, uint8_t com2);
static void timer2_oc2_fastpwm(struct MSIM_AVR *mcu, uint8_t com2,
                               uint8_t state);
static void timer2_oc2_pcpwm(struct MSIM_AVR *mcu, uint8_t com2,
                             uint8_t state);

int MSIM_M8AInit(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args)
{
	int r;

	r = mcu_init(mcu, args);
	if (r == 0) {
		update_watched_values(mcu);
		/* I/O ports have internal pull-up resistors (selected
		 * for each bit) */
		mcu->dm[PORTB] = 0xFF;
		mcu->dm[PORTC] = 0xFF;
		mcu->dm[PORTD] = 0xFF;
	}
	return r;
}

int MSIM_M8ATickTimers(struct MSIM_AVR *mcu)
{
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
	static uint32_t tc0_presc;
	static uint32_t tc0_ticks;
	uint8_t tccr0;			/* Timer/Counter0 control register */
	uint32_t presc;			/* Prescaler value */
	uint8_t stop_mode;

	stop_mode = 0;

	tccr0 = mcu->dm[TCCR0];
	presc = 0;

	switch (tccr0) {
	case 0x1:
		/* No prescaling, clk_io */
		presc = 1;
		break;
	case 0x2:
		/* clk_io/8 */
		presc = 8;
		break;
	case 0x3:
		/* clk_io/64 */
		presc = 64;
		break;
	case 0x4:
		/* clk_io/256 */
		presc = 256;
		break;
	case 0x5:
		/* clk_io/1024 */
		presc = 1024;
		break;
	case 0x6:
		/* Ext. clock on T0/PD4 (fall) */
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
		/* This is an external clock source mode (and not a stopped
		 * mode) actually, but we use this variable to skip all other
		 * actions. */
		stop_mode = 1;
		break;
	case 0x7:
		/* Ext. clock on T0/PD4 (rise) */
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
		/* This is an external clock source mode (and not a stopped
		 * mode) actually, but we use this variable to skip all other
		 * actions. */
		stop_mode = 1;
		break;
	case 0x0:			/* No clock source (stopped mode) */
	default:			/* Should not happen! */
		tc0_presc = 0;
		tc0_ticks = 0;
		stop_mode = 1;
		break;
	}

	/* Internal clock */
	if (stop_mode == 0U) {
		if (presc != tc0_presc) {
			tc0_presc = presc;
			tc0_ticks = 0;
		}
		if (tc0_ticks < (tc0_presc-1U)) {
			tc0_ticks++;
		} else if (tc0_ticks > (tc0_presc-1U)) {
			fprintf(stderr, "[e]: Number of Timer0 ticks=%" PRIu32
			        " should be less then or equal to "
			        "(prescaler-1)=%" PRIu32 ". Timer0 will not "
			        "be updated!\n", tc0_ticks, (tc0_presc-1U));
		} else {
			if (mcu->dm[TCNT0] == 0xFF) {
				/* Reset Timer/Counter0 */
				mcu->dm[TCNT0] = 0;
				/* Timer/Counter0 overflow occured */
				mcu->dm[TIFR] |= (1<<TOV0);
			} else {
				mcu->dm[TCNT0]++;
			}
			tc0_ticks = 0;
		}
	}
}

static void tick_timer1(struct MSIM_AVR *mcu)
{
	/* 16-bit Timer/Counter1
	 * There are several modes of operation available:
	 *
	 * - (supported) Normal Mode, WGM13:0 = 0;
	 * - (planned) Clear Timer on Compare Match (CTC) Mode,
	 *   WGM13:0 = 4 or 12;
	 * - (planned) Fast PWM Mode, WGM13:0 = 5, 6, 7, 14 or 15;
	 * - (planned) Phase Correct PWM Mode, WGM13:0 = 1, 2, 3, 10 or 11
	 * - (planned) Phase and Frequency Correct PWM Mode, WGM13:0 = 8 or 9
	 */
	static uint32_t tc1_presc;
	static uint32_t tc1_ticks;
	static uint8_t old_wgm1;
	uint8_t cs1;		/* Clock select bits CS12:0 */
	uint8_t wgm1;		/* Waveform Generation WGM13:0 */
	uint8_t com1a;		/* Compare Output mode COM1A1:0 */
	uint8_t com1b;		/* Compare Output mode COM1B1:0 */
	uint32_t presc;		/* Prescaler */
	uint8_t stop_mode;

	stop_mode = 0;

	cs1 = DM(TCCR1B)&0x7;
	wgm1 = ((DM(TCCR1B)>>4)&1) | ((DM(TCCR1B)>>3)&1) |
	       ((DM(TCCR1A)>>1)&1) | (DM(TCCR1A)&1);
	com1a = ((DM(TCCR1A)>>7)&1) | ((DM(TCCR1A)>>6)&1);
	com1b = ((DM(TCCR1A)>>5)&1) | ((DM(TCCR1A)>>4)&1);

	switch (cs1) {
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
	case 0x6:			/* Ext. clock on T1/PD5 (fall) */
		/* External clock isn't supported at the moment. */
		stop_mode = 1;
		break;
	case 0x7:			/* Ext. clock on T1/PD5 (rise) */
		/* External clock isn't supported at the moment. */
		stop_mode = 1;
		break;
	case 0x0:			/* No clock source (stopped mode) */
	default:			/* Should not happen! */
		tc1_presc = 0;
		tc1_ticks = 0;
		stop_mode = 1;
	}

	if ((stop_mode == 0U) && (presc != tc1_presc)) {
		tc1_presc = presc;
		/* Should we really clean these ticks? */
		tc1_ticks = 0;
	}

	if (stop_mode == 0U) {
		switch (wgm1) {
		case 0:
			timer1_normal(mcu, tc1_presc, &tc1_ticks, wgm1,
			              com1a, com1b);
			break;
		case 2:
			/* timer1_ctc(mcu, tc2_presc, &tc2_ticks,
			 *            wgm2, com2); */
			break;
		case 3:
			/* timer1_fastpwm(mcu, tc2_presc, &tc2_ticks,
			 *                wgm2, com2); */
			break;
		case 1:
			/* timer1_pcpwm(mcu, tc2_presc, &tc2_ticks,
			 *              wgm2, com2); */
			break;
		default:			/* Should not happen! */
			if (wgm1 != old_wgm1) {
				fprintf(stderr, "[!]: Selected mode "
				        "WGM13:0 = %u of the Timer/Counter1 "
				        "is not supported\n", wgm1);
				old_wgm1 = wgm1;
			}
			tc1_presc = 0;
			tc1_ticks = 0;
			break;
		}
	}
}

static void tick_timer2(struct MSIM_AVR *mcu)
{
	/* 8-bit Timer/Counter1
	 * There are several modes of operation available:
	 *
	 * - (supported) The simplest mode is a normal one, when counter
	 *               is incremented only and no counter clear is performed,
	 *               WGM21:0 = 0;
	 * - (supported) Clear Timer on Compare Match (CTC) Mode, WGM21:0 = 2;
	 * - (supported) Fast PWM Mode, WGM21:0 = 3.
	 * - (supported) Phase Correct PWM Mode, WGM21:0 = 1;
	 */
	static uint32_t tc2_presc;
	static uint32_t tc2_ticks;
	static uint8_t old_wgm2;
	uint8_t cs2;		/* Clock Select bits CS22:0 */
	uint8_t wgm2;		/* Waveform Generation WGM21:0 */
	uint8_t com2;		/* Compare Output mode COM21:0 */
	uint32_t presc;		/* Prescaler */
	uint8_t stop_mode;

	stop_mode = 0;

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
	case 0x0:			/* No clock source (stopped mode) */
	default:			/* Should not happen! */
		tc2_presc = 0;
		tc2_ticks = 0;
		stop_mode = 1;
	}

	if ((stop_mode == 0U) && (presc != tc2_presc)) {
		/* We may have TC2 enabled with Compare Match missed. */
		if ((tc2_presc == 0U) && (mcu->dm[TCNT2] > mcu->dm[OCR2])) {
			missed_cm = 1;
		}

		tc2_presc = presc;
		/* Should we really clean these ticks? */
		tc2_ticks = 0;
	}
	if ((stop_mode == 0U) && (missed_cm != 0U) && (presc == tc2_presc)) {
		missed_cm = 0;
	}

	if (stop_mode == 0U) {
		switch (wgm2) {
		case 0:
			timer2_normal(mcu, tc2_presc, &tc2_ticks, wgm2, com2);
			break;
		case 2:
			timer2_ctc(mcu, tc2_presc, &tc2_ticks, wgm2, com2);
			break;
		case 3:
			timer2_fastpwm(mcu, tc2_presc, &tc2_ticks, wgm2, com2);
			break;
		case 1:
			timer2_pcpwm(mcu, tc2_presc, &tc2_ticks, wgm2, com2);
			break;
		default:			/* Should not happen! */
			if (wgm2 != old_wgm2) {
				fprintf(stderr, "[!]: Selected mode "
				        "WGM21:0 = %u of the Timer/Counter2 "
				        "is not supported\n", wgm2);
				old_wgm2 = wgm2;
			}
			tc2_presc = 0;
			tc2_ticks = 0;
			break;
		}
	}
}

static void timer1_normal(struct MSIM_AVR *mcu,
                          uint32_t presc, uint32_t *ticks,
                          uint8_t wgm1, uint8_t com1a, uint8_t com1b)
{
	uint32_t tcnt1;
	uint32_t ocr1a, ocr1b;

	tcnt1 = ((DM(TCNT1H)<<8)&0xFF00) | (DM(TCNT1L));
	ocr1a = ((DM(OCR1AH)<<8)&0xFF00) | (DM(OCR1AL));
	ocr1b = ((DM(OCR1BH)<<8)&0xFF00) | (DM(OCR1BL));

	if ((*ticks) < (presc-1U)) {
		(*ticks)++;
	} else if ((*ticks) > (presc-1U)) {
		fprintf(stderr, "[e]: Number of Timer1 ticks=%" PRIu32
		        " should be less then or equal to "
		        "(prescaler-1)=%" PRIu32 ". Timer1 will not "
		        "be updated!\n", *ticks, (presc-1U));
	} else {
		if (tcnt1 == 0xFFFFU) {
			/* Reset Timer/Counter1 */
			tcnt1 = 0;
			/* Set Timer/Counter1 overflow flag */
			mcu->dm[TIFR] |= (1<<TOV1);
		} else if (tcnt1 == ocr1a) {
			mcu->dm[TIFR] |= (1<<OCF1A);
			timer1_oc1_nonpwm(mcu, com1a, com1b, A_CHAN);
			tcnt1++;
		} else if (tcnt1 == ocr1b) {
			mcu->dm[TIFR] |= (1<<OCF1B);
			timer1_oc1_nonpwm(mcu, com1a, com1b, B_CHAN);
			tcnt1++;
		} else {
			tcnt1++;
		}
		DM(TCNT1H) = (tcnt1>>8)&0xFFU;
		DM(TCNT1L) = tcnt1&0xFFU;
		(*ticks) = 0;
	}
}

static void timer1_oc1_nonpwm(struct MSIM_AVR *mcu, uint8_t com1a,
                              uint8_t com1b, uint8_t chan)
{
	uint8_t pin, com;
	uint8_t err;

	err = 0;

	/* Check Data Direction Register first. DDRB3 should be set to
	 * enable the output driver (according to a datasheet).*/
	if (chan == (uint8_t)A_CHAN) {
		pin = PB1;
		com = com1a;
	} else if (chan == (uint8_t)B_CHAN) {
		pin = PB2;
		com = com1b;
	} else {
		fprintf(stderr, "[e] Unsupported channel of the Output "
		        "Compare unit is used in Timer/Counter1! "
		        "It's highly likely a bug - please, report it.\n");
		err = 1;
	}
	if ((err == 0U) && (!IS_SET(mcu->dm[DDRB], pin))) {
		err = 1;
	}

	/* Update Output Compare pin (OC1A or OC1B) */
	if (err == 0U) {
		switch (com) {
		case 1:
			if (IS_SET(mcu->dm[PORTB], pin) == 1) {
				CLEAR(mcu->dm[PORTB], pin);
			} else {
				SET(mcu->dm[PORTB], pin);
			}
			break;
		case 2:
			CLEAR(mcu->dm[PORTB], pin);
			break;
		case 3:
			SET(mcu->dm[PORTB], pin);
			break;
		case 0:
		default:
			/* OC1A/OC1B disconnected, do nothing */
			break;
		}
	}
}

static void timer2_normal(struct MSIM_AVR *mcu,
                          uint32_t presc, uint32_t *ticks,
                          uint8_t wgm2, uint8_t com2)
{
	if ((*ticks) < (presc-1U)) {
		(*ticks)++;
	} else if ((*ticks) > (presc-1U)) {
		fprintf(stderr, "[e]: Number of Timer2 ticks=%" PRIu32
		        " should be less then or equal to "
		        "(prescaler-1)=%" PRIu32 ". Timer2 will not "
		        "be updated!\n", *ticks, (presc-1U));
	} else {
		if (mcu->dm[TCNT2] == 0xFF) {
			/* Reset Timer/Counter2 */
			mcu->dm[TCNT2] = 0;
			/* Set Timer/Counter2 overflow flag */
			mcu->dm[TIFR] |= (1<<TOV2);
		} else if (mcu->dm[TCNT2] == mcu->dm[OCR2]) {
			mcu->dm[TIFR] |= (1<<OCF2);
			timer2_oc2_nonpwm(mcu, com2);
			mcu->dm[TCNT2]++;
		} else {
			mcu->dm[TCNT2]++;
		}
		(*ticks) = 0;
	}
}

static void timer2_ctc(struct MSIM_AVR *mcu, uint32_t presc, uint32_t *ticks,
                       uint8_t wgm2, uint8_t com2)
{
	/* NOTE: We're able to generate a waveform output in this mode with a
	 * frequency which is controlled by the value in OCR2 register
	 * (top border of the timer/counter). */

	if ((*ticks) < (presc-1U)) {
		(*ticks)++;
	} else if ((*ticks) > (presc-1U)) {
		fprintf(stderr, "[e]: Number of Timer2 ticks=%" PRIu32
		        " should be less then or equal to "
		        "(prescaler-1)=%" PRIu32 ". Timer2 will not "
		        "be updated!\n", *ticks, (presc-1U));
	} else {
		if (mcu->dm[TCNT2] == mcu->dm[OCR2]) {
			/* Reset Timer/Counter2 */
			mcu->dm[TCNT2] = 0;
			/* Set Timer/Counter2 output compare flag */
			mcu->dm[TIFR] |= (1<<OCF2);

			timer2_oc2_nonpwm(mcu, com2);
		} else {
			mcu->dm[TCNT2]++;
		}
		(*ticks) = 0;
	}
}

static void timer2_fastpwm(struct MSIM_AVR *mcu,
                           uint32_t presc, uint32_t *ticks,
                           uint8_t wgm2, uint8_t com2)
{
	/* NOTE: This mode allows PWM to be generated using single-slope
	 * operation. Duty cycle can be controlled by the value in
	 * OCR2 register.
	 *
	 * Single-slope operation means counting from BOTTOM(0) to MAX(255),
	 * reset back to BOTTOM in the following clock cycle and
	 * start again. */
	if ((*ticks) < (presc-1U)) {
		(*ticks)++;
	} else if ((*ticks) > (presc-1U)) {
		fprintf(stderr, "[e]: Number of Timer2 ticks=%" PRIu32
		        " should be less then or equal to "
		        "(prescaler-1)=%" PRIu32 ". Timer2 will not "
		        "be updated!\n", *ticks, (presc-1U));
	} else {
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
		(*ticks) = 0;
	}
}

static void timer2_pcpwm(struct MSIM_AVR *mcu,
                         uint32_t presc, uint32_t *ticks,
                         uint8_t wgm2, uint8_t com2)
{
	/* NOTE: This mode allows PWM to be generated using dual-slope
	 * operation (preferred for motor control applications). Duty cycle
	 * can be controlled by the value in OCR2 register.
	 *
	 * Dual-slope operation means counting from BOTTOM(0) to MAX(255),
	 * then from MAX back to the BOTTOM and start again. */
	static uint8_t cnt_down = 0;

	if ((*ticks) < (presc-1U)) {
		(*ticks)++;
	} else if ((*ticks) > (presc-1U)) {
		fprintf(stderr, "[e]: Number of Timer2 ticks=%" PRIu32
		        " should be less then or equal to "
		        "(prescaler-1)=%" PRIu32 ". Timer2 will not "
		        "be updated!\n", *ticks, (presc-1U));
	} else {
		if (mcu->dm[TCNT2] == 0xFF) {
			if (ocr2_buf == 0xFFU) {
				mcu->dm[TIFR] |= (1<<OCF2);
			}
			if (missed_cm || ((ocr2_buf == 0xFFU) &&
			                  (mcu->dm[OCR2] < 0xFF))) {
				timer2_oc2_pcpwm(mcu, com2, COMP_MATCH_UPCNT);
			}

			cnt_down = 1;
			ocr2_buf = mcu->dm[OCR2];
			if (ocr2_buf == 0xFFU) {
				timer2_oc2_pcpwm(mcu, com2,
				                 COMP_MATCH_DOWNCNT);
			}
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
			if (!cnt_down) {
				mcu->dm[TCNT2]++;
			} else {
				mcu->dm[TCNT2]--;
			}
		} else {
			if (!cnt_down) {
				mcu->dm[TCNT2]++;
			} else {
				mcu->dm[TCNT2]--;
			}
		}
		(*ticks) = 0;
	}
}

static void timer2_oc2_nonpwm(struct MSIM_AVR *mcu, uint8_t com2)
{
	uint8_t com2_v;

	/* Check Data Direction Register first. DDRB3 should be set to
	 * enable the output driver (according to a datasheet). */
	if (!IS_SET(mcu->dm[DDRB], PB3)) {
		com2_v = NOT_CONNECTED;
	} else {
		com2_v = com2;
	}

	/* Update Output Compare pin (OC2) */
	switch (com2) {
	case 1:
		if (IS_SET(mcu->dm[PORTB], PB3) == 1) {
			CLEAR(mcu->dm[PORTB], PB3);
		} else {
			SET(mcu->dm[PORTB], PB3);
		}
		break;
	case 2:
		CLEAR(mcu->dm[PORTB], PB3);
		break;
	case 3:
		SET(mcu->dm[PORTB], PB3);
		break;
	case 0:
	case NOT_CONNECTED:
	default:
		/* OC2 disconnected, do nothing */
		break;
	}
}

static void timer2_oc2_fastpwm(struct MSIM_AVR *mcu, uint8_t com2,
                               uint8_t state)
{
	uint8_t com2_v;

	/* Check Data Direction Register first. DDRB3 should be set to
	 * enable the output driver (according to a datasheet). */
	if (!IS_SET(mcu->dm[DDRB], PB3)) {
		com2_v = NOT_CONNECTED;
	} else {
		com2_v = com2;
	}

	/* Update Output Compare pin (OC2) */
	switch (com2) {
	case 1:
		if (state == (uint8_t)COMPARE_MATCH) {
			if (IS_SET(mcu->dm[PORTB], PB3) == 1) {
				CLEAR(mcu->dm[PORTB], PB3);
			} else {
				SET(mcu->dm[PORTB], PB3);
			}
		}
		break;
	case 2:		/* Non-inverting compare output mode */
		if (state == (uint8_t)COMPARE_MATCH) {
			CLEAR(mcu->dm[PORTB], PB3);
		} else {
			SET(mcu->dm[PORTB], PB3);
		}
		break;
	case 3:		/* Inverting compare output mode */
		if (state == (uint8_t)COMPARE_MATCH) {
			SET(mcu->dm[PORTB], PB3);
		} else {
			CLEAR(mcu->dm[PORTB], PB3);
		}
		break;
	case 0:
	case NOT_CONNECTED:
	default:
		/* OC2 disconnected, do nothing */
		break;
	}
}

static void timer2_oc2_pcpwm(struct MSIM_AVR *mcu, uint8_t com2,
                             uint8_t state)
{
	uint8_t com2_v;

	/* Check Data Direction Register first. DDRB3 should be set to
	 * enable the output driver (according to a datasheet).*/
	if (!IS_SET(mcu->dm[DDRB], PB3)) {
		com2_v = NOT_CONNECTED;
	} else {
		com2_v = com2;
	}

	/* Update Output Compare pin (OC2) */
	switch (com2_v) {
	case 1:
		fprintf(stderr, "[!] COM21:0=1 is reserved in the phase "
		        "correct PWM mode\n");
		break;
	case 2:		/* Non-inverting compare output mode */
		if (state == (uint8_t)COMP_MATCH_UPCNT) {
			CLEAR(mcu->dm[PORTB], PB3);
		} else {
			SET(mcu->dm[PORTB], PB3);
		}
		break;
	case 3:		/* Inverting compare output mode */
		if (state == (uint8_t)COMP_MATCH_UPCNT) {
			SET(mcu->dm[PORTB], PB3);
		} else {
			CLEAR(mcu->dm[PORTB], PB3);
		}
		break;
	case 0:
	case NOT_CONNECTED:
	default:
		/* OC2 disconnected, do nothing */
		break;
	}
}

int MSIM_M8ASetFuse(struct MSIM_AVR *mcu, uint32_t fuse_n, uint8_t fuse_v)
{
	uint8_t cksel, bootsz, ckopt;
	int ret;

	ret = 0;
	if (fuse_n > 1U) {
		fprintf(stderr, "[e]: Fuse #%u is not supported by %s\n",
		        fuse_n, mcu->name);
		ret = 1;
	}

	if (ret == 0) {
		mcu->fuse[fuse_n] = fuse_v;
		cksel = mcu->fuse[0]&0xF;
		ckopt = (mcu->fuse[1]>>4)&0x1;

		switch (fuse_n) {
		case FUSE_LOW:
			cksel = fuse_v&0xFU;
			if (cksel == 0U) {
				mcu->clk_source = AVR_EXT_CLK;
			} else if ((cksel >= 1U) && (cksel <= 4U)) {
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
				default:
					/* Shouldn't happen! */
					fprintf(stderr, "[e]: CKSEL = %" PRIu8
					        ", but it should be within "
					        "[1,4] inclusively!\n", cksel);
					break;
				}
			} else if ((cksel >= 5U) && (cksel <= 8U)) {
				mcu->clk_source = AVR_EXT_RC_CLK;
				switch (cksel) {
				case 5:
					/* max 0.9 MHz */
					mcu->freq = 900000;
					break;
				case 6:
					/* max 3 MHz */
					mcu->freq = 3000000;
					break;
				case 7:
					/* max 4 MHz */
					mcu->freq = 8000000;
					break;
				case 8:
					/* max 12 MHz */
					mcu->freq = 12000000;
					break;
				default:
					/* Shouldn't happen! */
					fprintf(stderr, "[e]: CKSEL = %" PRIu8
					        ", but it should be within "
					        "[5,8] inclusively!\n", cksel);
					break;
				}
			} else if (cksel == 9U) {
				/* 32.768 kHz low frequency crystal */
				mcu->clk_source = AVR_EXT_LOWF_CRYSTAL_CLK;
				mcu->freq = 32768;
			} else if ((cksel >= 10U) && (cksel <= 15U)) {
				mcu->clk_source = AVR_EXT_CRYSTAL;
				cksel = (cksel>>1)&0x7U;
				switch (cksel) {
				case 5:
					/* max 0.9 MHz */
					mcu->freq = 900000;
					break;
				case 6:
					/* max 3 MHz */
					mcu->freq = 3000000;
					break;
				case 7:
					/* max 8 MHz */
					mcu->freq = 8000000;
					break;
				default:
					fprintf(stderr, "[e]: (CKSEL>>1) = "
					        "%" PRIu8 ", but it should be "
					        "within [5,7] inclusively!\n",
					        cksel);
					break;
				}
				if (!ckopt) {
					/* max 16 MHz */
					mcu->freq = 16000000;
				}
			} else {
				/* Shouldn't happen! */
				fprintf(stderr, "[e]: CKSEL = %" PRIu8 ", "
				        "but it should be within [0,15] "
				        "inclusively!\n", cksel);
			}
			break;
		case FUSE_HIGH:
			bootsz = (fuse_v>>1)&0x3U;
			ckopt = (fuse_v>>4)&0x1U;
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
			default:
				/* Shouldn't happen! */
				fprintf(stderr, "[e]: BOOTSZ = %" PRIu8 ", "
				        "but it should be within [0,3] "
				        "inclusively!\n", bootsz);
				break;
			}

			if ((fuse_v&1U) == 1U) {
				/* BOOTRST is 1(unprogrammed) */
				mcu->intr->reset_pc = 0x0000;
				mcu->pc = 0x0000;
			} else {
				/* BOOTRST is 0(programmed) */
				mcu->intr->reset_pc = mcu->bls->start;
				mcu->pc = mcu->bls->start;
			}

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
			default:
				fprintf(stderr, "[e]: CKSEL = %" PRIu8 ", "
				        "but it should be within [5,7] "
				        "inclusively!\n", cksel);
				break;
			}
			if (!ckopt) {
				mcu->freq = 16000000;	/* max 16 MHz */
			}

			break;
		default:			/* Should not happen */
			fprintf(stderr, "[e]: Unknown fuse = %" PRIu32 ", "
			        "%s will not be modified!\n",
			        fuse_n, mcu->name);
			ret = 1;
		}
	}
	return ret;
}

int MSIM_M8ASetLock(struct MSIM_AVR *mcu, uint8_t lock_v)
{
	/* Waiting to be implemented */
	return 0;
}

int MSIM_M8AProvideIRQs(struct MSIM_AVR *mcu)
{
	uint8_t timsk, tifr;

	timsk = mcu->dm[TIMSK];
	tifr = mcu->dm[TIFR];

	/* TIMER0 OVF - Timer/Counter0 Overflow */
	if (((timsk>>TOIE0)&1U) && ((tifr>>TOV0)&1U)) {
		mcu->intr->irq[TIMER0_OVF_vect_num-1] = 1;
		/* Clear TOV0 flag */
		mcu->dm[TIFR] &= ~(1<<TOV0);
	}

	/* TIMER2 OVF - Timer/Counter2 Overflow */
	if (((timsk>>TOIE2)&1U) && ((tifr>>TOV2)&1U)) {
		mcu->intr->irq[TIMER2_OVF_vect_num-1] = 1;
		/* Clear TOV2 flag */
		mcu->dm[TIFR] &= ~(1<<TOV2);
	}
	/* TIMER2 COMP - Timer/Counter2 Compare Match */
	if (((timsk>>OCIE2)&1U) && ((tifr>>OCF2)&1U)) {
		mcu->intr->irq[TIMER2_COMP_vect_num-1] = 1;
		/* Clear OCF2 flag */
		mcu->dm[TIFR] &= ~(1<<OCF2);
	}

	return 0;
}
