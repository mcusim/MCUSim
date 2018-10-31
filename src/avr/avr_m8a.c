/*
 * Copyright (c) 2017, 2018, The MCUSim Contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the MCUSim or its parts nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Atmel ATmega8A-specific functions implemented here.
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "mcusim/mcusim.h"
#include "mcusim/log.h"
#include "mcusim/avr/sim/m8/m8a.h"
#include "mcusim/avr/sim/mcu_init.h"

#define FUSE_LOW		0
#define FUSE_HIGH		1
#define FUSE_EXT		2
#define IS_SET(byte, bit)	(((byte)>>(bit))&1)
#define IS_CLEAR(byte, bit)	((~(((byte)>>(bit))&1))&1)
#define IS_RISE(init, val, bit)	((!((init>>bit)&1)) & ((val>>bit)&1))
#define IS_FALL(init, val, bit)	(((init>>bit)&1) & (!((val>>bit)&1)))
#define CLEAR(byte, bit)	((byte)&=(~(1<<(bit))))
#define SET(byte, bit)		((byte)|=(1<<(bit)))

#define IS_WRIT(mcu, byte)	(((mcu->writ_io[0]) == (byte)) ||	\
				 ((mcu->writ_io[1]) == (byte)) ||	\
				 ((mcu->writ_io[2]) == (byte)) ||	\
				 ((mcu->writ_io[3]) == (byte)))
#define IS_READ(mcu, byte)	(((mcu->read_io[0]) == (byte)) ||	\
				 ((mcu->read_io[1]) == (byte)) ||	\
				 ((mcu->read_io[2]) == (byte)) ||	\
				 ((mcu->read_io[3]) == (byte)))

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

static uint8_t init_portd;	/* PORTD (buffer) */
static uint8_t init_pind;	/* PIND (buffer) */
static uint8_t init_portb;	/* PORTB (buffer) */
static uint8_t init_pinb;	/* PINB (buffer) */

static uint8_t ucsra_buf;	/* USART Control and Status A (buffer) */
static uint8_t ucsrc_buf;	/* USART Control and Status C (buffer) */
static uint8_t ubrrh_buf;	/* USART Baud Rate High (buffer) */
static uint8_t ubrrl_buf;	/* USART Baud Rate Low (buffer) */
static uint8_t udr_buf;		/* USART Data Register */

static uint8_t wdtcr_buf;	/* Watchdog Timer Control Register */
static uint8_t wdce_cycles = 0;	/* Clean WDCE bit in this number of cycles */

/* The OCRx Compare Registers are double buffered when using any of
 * the PWM modes and updated during either TOP or BOTTOM of the counting
 * sequence. */
static uint8_t ocr2_buf;
static uint32_t ocr1a_buf;
static uint32_t ocr1b_buf;

/* Timer may start counting from a value higher then the one stored in OCR2
 * and for that reason misses the Compare Match. This flag is set in
 * this case. */
static uint8_t missed_cm = 0;

static void tick_timer0(struct MSIM_AVR *mcu);
static void tick_timer1(struct MSIM_AVR *mcu);
static void tick_timer2(struct MSIM_AVR *mcu);
static void update_watched(struct MSIM_AVR *mcu);

static void tick_usart(struct MSIM_AVR *mcu);
static void usart_transmit(struct MSIM_AVR *mcu);
static void usart_receive(struct MSIM_AVR *mcu);

static void tick_wdt(struct MSIM_AVR *mcu);

/* Timer/Counter1 modes of operation */
static void timer1_normal(struct MSIM_AVR *mcu, uint32_t presc,
                          uint32_t *ticks, uint8_t wgm1, uint8_t com1a,
                          uint8_t com1b);
static void timer1_ctc(struct MSIM_AVR *mcu, uint32_t presc,
                       uint32_t *ticks, uint8_t wgm1, uint8_t com1a,
                       uint8_t com1b);
static void timer1_fastpwm(struct MSIM_AVR *mcu, uint32_t presc,
                           uint32_t *ticks, uint8_t wgm1, uint8_t com1a,
                           uint8_t com1b);
static void timer1_pcpwm(struct MSIM_AVR *mcu, uint32_t presc,
                         uint32_t *ticks, uint8_t wgm1, uint8_t com1a,
                         uint8_t com1b);
static void timer1_pfcpwm(struct MSIM_AVR *mcu, uint32_t presc,
                          uint32_t *ticks, uint8_t wgm1, uint8_t com1a,
                          uint8_t com1b);

/* Timer/Counter1 helper functions */
static void timer1_oc1_nonpwm(struct MSIM_AVR *mcu, uint8_t com1a,
                              uint8_t com1b, uint8_t chan);
static void timer1_oc1_fastpwm(struct MSIM_AVR *mcu, uint8_t com1a,
                               uint8_t com1b, uint8_t chan, uint8_t state);
static void timer1_oc1_pcpwm(struct MSIM_AVR *mcu, uint8_t com1a,
                             uint8_t com1b, uint8_t chan, uint8_t state);

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

/* Timer/Counter2 helper functions */
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
		/* I/O ports have internal pull-up resistors */
		DM(PORTB) = 0xFF;
		DM(PORTC) = 0xFF;
		DM(PORTD) = 0xFF;

		/* Set default WDT parameters */
		mcu->wdt.sys_presc = 1;
		mcu->wdt.sys_ticks = 0;
		mcu->wdt.presc = 16*1024;	/* Default WDT prescaler */
		mcu->wdt.ticks = 0;
		mcu->wdt.always_on = 0; 	/* WDT initially disabled */
		mcu->wdt.checked = 0;		/* WDT will be adjusted */

		/* Set USART registers */
		DM(UCSRA) = 0x20;
		DM(UCSRC) = 0x82;

		update_watched(mcu);

		/* Set USART registers */
		ubrrh_buf = 0;
	}
	return r;
}

int MSIM_M8ATickPerf(struct MSIM_AVR *mcu)
{
	tick_timer2(mcu);
	tick_timer1(mcu);
	tick_timer0(mcu);
	tick_usart(mcu);
	tick_wdt(mcu);

	/* Update watched values after all of the peripherals. */
	update_watched(mcu);

	return 0;
}

static void update_watched(struct MSIM_AVR *mcu)
{
	init_portd = DM(PORTD);
	init_pind = DM(PIND);
	init_portb = DM(PORTB);
	init_pinb = DM(PINB);

	/* NOTE: The UBRRH register shares the same I/O location as the
	 * UCSRC Register (24.10. Accessing UBRRH/UCSRC Registers). It means
	 * that URSEL (MSB bit) should be checked additionally to understand
	 * which register should be updated */
	if (((DM(UBRRH)>>URSEL)&1) == 0) {
		ubrrh_buf = DM(UBRRH);
	} else {
		ucsrc_buf = DM(UCSRC);
	}
	ubrrl_buf = DM(UBRRL);
	ucsra_buf = DM(UCSRA);
	udr_buf = DM(UDR);

	wdtcr_buf = DM(WDTCR);
}

static void tick_timer0(struct MSIM_AVR *mcu)
{
	static uint32_t tc0_presc;
	static uint32_t tc0_ticks;
	uint8_t tccr0;			/* Timer/Counter0 control register */
	uint32_t presc;			/* Prescaler value */
	uint8_t stop_mode;

	stop_mode = 0;
	tccr0 = DM(TCCR0);
	presc = 0;

	switch (tccr0) {
	case 0x1:			/* No prescaling, clk_io */
		presc = 1;
		break;
	case 0x2:			/* clk_io/8 */
		presc = 8;
		break;
	case 0x3:			/* clk_io/64 */
		presc = 64;
		break;
	case 0x4:			/* clk_io/256 */
		presc = 256;
		break;
	case 0x5:			/* clk_io/1024 */
		presc = 1024;
		break;
	case 0x6:			/* Ext. clock on T0/PD4 (fall) */
		if (IS_FALL(init_portd, DM(PORTD), PD4) ||
		                IS_FALL(init_pind, DM(PIND), PD4)) {
			if (DM(TCNT0) == 0xFF) {
				/* Reset Timer/Counter0 */
				DM(TCNT0) = 0;
				/* Timer/Counter0 overflow */
				DM(TIFR) |= (1<<TOV0);
			} else {
				DM(TCNT0)++;
			}
		}
		tc0_presc = 0;
		tc0_ticks = 0;
		/* This is an external clock source mode (and not a stopped
		 * mode) actually, but we use this variable to skip all other
		 * actions. */
		stop_mode = 1;
		break;
	case 0x7:			/* Ext. clock on T0/PD4 (rise) */
		if (IS_RISE(init_portd, DM(PORTD), PD4) ||
		                IS_RISE(init_pind, DM(PIND), PD4)) {
			if (DM(TCNT0) == 0xFF) {
				/* Reset Timer/Counter0 */
				DM(TCNT0) = 0;
				/* Timer/Counter0 overflow */
				DM(TIFR) |= (1<<TOV0);
			} else {
				DM(TCNT0)++;
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
			snprintf(mcu->log, sizeof mcu->log, "number of Timer0 "
			         "ticks=%" PRIu32 " should be <= "
			         "(prescaler-1)=%" PRIu32 "; timer0 will not "
			         "be updated", tc0_ticks, (tc0_presc-1U));
			MSIM_LOG_ERROR(mcu->log);
		} else {
			if (DM(TCNT0) == 0xFF) {
				/* Reset Timer/Counter0 */
				DM(TCNT0) = 0;
				/* Timer/Counter0 overflow occured */
				DM(TIFR) |= (1<<TOV0);
			} else {
				DM(TCNT0)++;
			}
			tc0_ticks = 0;
		}
	}
}

static void tick_timer1(struct MSIM_AVR *mcu)
{
	/* 16-bit Timer/Counter1
	 *
	 * There are several modes of operation available;
	 * - (supported) Normal Mode, WGM13:0 = 0;
	 * - (supported) Clear Timer on Compare Match (CTC) Mode,
	 *   WGM13:0 = 4 or 12;
	 * - (supported) Fast PWM Mode, WGM13:0 = 5, 6, 7, 14 or 15;
	 * - (supported) Phase Correct PWM Mode, WGM13:0 = 1, 2, 3, 10 or 11
	 * - (supported) Phase and Frequency Correct PWM Mode, WGM13:0 = 8, 9
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
	wgm1 = (uint8_t) (((DM(TCCR1B)>>WGM13)&1)<<3) |
	       (uint8_t) (((DM(TCCR1B)>>WGM12)&1)<<2) |
	       (uint8_t) (((DM(TCCR1A)>>WGM11)&1)<<1) |
	       (uint8_t) (((DM(TCCR1A)>>WGM10)&1));
	com1a = (uint8_t) (((DM(TCCR1A)>>COM1A1)&1)<<1) |
	        (uint8_t) ((DM(TCCR1A)>>COM1A0)&1);
	com1b = (uint8_t) (((DM(TCCR1A)>>COM1B1)&1)<<1) |
	        (uint8_t) ((DM(TCCR1A)>>COM1B0)&1);

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
		ocr1a_buf = ((DM(OCR1AH)<<8)&0xFF00) | (DM(OCR1AL)&0xFF);
		ocr1b_buf = ((DM(OCR1BH)<<8)&0xFF00) | (DM(OCR1BL)&0xFF);
		/* Should we really clean these ticks? */
		tc1_ticks = 0;
	}

	if (stop_mode == 0U) {
		switch (wgm1) {
		case 0:
			timer1_normal(mcu, tc1_presc, &tc1_ticks, wgm1,
			              com1a, com1b);
			break;
		case 4:
		case 12:
			timer1_ctc(mcu, tc1_presc, &tc1_ticks, wgm1, com1a,
			           com1b);
			break;
		case 5:
		case 6:
		case 7:
		case 14:
		case 15:
			timer1_fastpwm(mcu, tc1_presc, &tc1_ticks, wgm1,
			               com1a, com1b);
			break;
		case 1:
		case 2:
		case 3:
		case 10:
		case 11:
			timer1_pcpwm(mcu, tc1_presc, &tc1_ticks, wgm1, com1a,
			             com1b);
			break;
		case 8:
		case 9:
			timer1_pfcpwm(mcu, tc1_presc, &tc1_ticks, wgm1, com1a,
			              com1b);
			break;
		default:
			if (wgm1 != old_wgm1) {
				snprintf(mcu->log, sizeof mcu->log, "Selected "
				         "mode WGM13:0 = %u of the timer1 "
				         "is not supported", wgm1);
				old_wgm1 = wgm1;
				MSIM_LOG_WARN(mcu->log);
			}
			tc1_presc = 0;
			tc1_ticks = 0;
			break;
		}
	}
}

static void tick_timer2(struct MSIM_AVR *mcu)
{
	/* 8-bit Timer/Counter2
	 *
	 * There are several modes of operation available:
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

	cs2 = DM(TCCR2)&0x7;
	wgm2 = (((DM(TCCR2)>>WGM21)<<1)&2) | ((DM(TCCR2)>>WGM20)&1);
	com2 = (((DM(TCCR2)>>COM21)<<1)&2) | ((DM(TCCR2)>>COM20)&1);

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
		if ((tc2_presc == 0U) && (DM(TCNT2) > DM(OCR2))) {
			missed_cm = 1;
		}

		tc2_presc = presc;
		ocr2_buf = mcu->dm[OCR2];
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
				snprintf(mcu->log, sizeof mcu->log, "Selected "
				         "mode WGM21:0 = %u of timer2 "
				         "is not supported", wgm2);
				old_wgm2 = wgm2;
				MSIM_LOG_WARN(mcu->log);
			}
			tc2_presc = 0;
			tc2_ticks = 0;
			break;
		}
	}
}

static void tick_usart(struct MSIM_AVR *mcu)
{
	/* You may think about this function as a programmable prescaler or
	 * baud rate generator.
	 *
	 * It is running at the system clock (Fosc) and is loaded with the
	 * 12-bit UBRR register value each time the counter has counted down
	 * to zero or when the UBRRL register is written.
	 * (see 24.3.1. Internal Clock Generation...) */
	uint32_t *rx_ticks = &mcu->usart.rx_ticks;
	uint32_t *tx_ticks = &mcu->usart.tx_ticks;
	uint32_t *baud = &mcu->usart.baud;
	uint32_t *rx_presc = &mcu->usart.rx_presc;
	uint32_t *tx_presc = &mcu->usart.tx_presc;
	uint8_t mult = 1;

	if ((ubrrl_buf != DM(UBRRL)) || (*tx_ticks == 0U)) {
		/* Load a new baud rate value */
		if (((DM(UBRRH)>>UMSEL)&1) == 0U) {
			/* There is a UBRRH value stored in data memory after
			 * the last tick of the AVR decoder. */
			if (DM(UBRRH) != ubrrh_buf) {
				*baud = (uint32_t)((DM(UBRRH)&0x0F)<<8) |
				        (uint32_t)DM(UBRRL);
			} else {
				*baud = (uint32_t)((ubrrh_buf&0x0F)<<8) |
				        (uint32_t)DM(UBRRL);
			}
		} else {
			/* There is a UCSRC value stored in data memory after
			 * the last tick of the AVR decoder. */
			*baud = (uint32_t)((ubrrh_buf&0x0F)<<8) |
			        (uint32_t)DM(UBRRL);
		}

		/* Update Rx clock prescaler (and ticks?) */
		*rx_presc = *baud+1;
		*rx_ticks = *rx_presc;

		if (((DM(UCSRC)>>UMSEL)&1) == 0U) {
			if (((DM(UCSRA)>>U2X)&1) == 0U) {
				mult = 16; /* Asynchronous Normal mode */
			} else {
				mult = 8; /* Asynchronous Double Speed mode */
			}
		} else {
			mult = 1; /* Synchronous mode */
			MSIM_LOG_WARN("USART synchronous mode is not "
			              "supported yet, Txclk=Fosc/(UBRR+1)");
		}
		/* Update Tx clock prescaler (and ticks?) */
		*tx_presc = mult*(*baud+1);
		*tx_ticks = *tx_presc;
	}

	/* UDR has been written with UDRE flag set. It means that the Transmit
	 * Data Buffer Register (TXB) is a destination for data stored in the
	 * UDR Register location. */
	if ((IS_WRIT(mcu, UDR)) && (IS_SET(DM(UCSRA), UDRE) == 1U)) {
		mcu->usart.txb = DM(UDR);
		/* Clear UDRE flag */
		DM(UCSRA) &= ~(1<<UDRE);
	}

	if (IS_READ(mcu, UDR)) {
		DM(UCSRA) &= ~(1<<RXC);
	}

	/* Count-down Rx ticks */
	if (*rx_ticks > 0) {
		(*rx_ticks)--;
	}
	if ((*rx_ticks == 0U) && (((DM(UCSRB)>>RXEN)&1) == 1U)) {
		/* Generate Rx clock */
		usart_receive(mcu);
		*rx_ticks = *rx_presc;
	} else {
		/* USART Rx inactive, do nothing */
	}

	/* Count-down Tx ticks */
	if (*tx_ticks > 0) {
		(*tx_ticks)--;
	}
	if ((*tx_ticks == 0U) && (((DM(UCSRB)>>TXEN)&1) == 1U)) {
		/* Generate Tx clock */
		usart_transmit(mcu);
	} else {
		/* USART Tx inactive, do nothing */
	}
}

static void tick_wdt(struct MSIM_AVR *mcu)
{
	uint8_t wdp;		/* Watchdog Prescaler */

	/* Initial WDT adjustment */
	if (mcu->wdt.checked == 0U) {
		/* WDT oscillator frequency is 1 MHz */
		mcu->wdt.sys_presc = mcu->freq/1000000;
		mcu->wdt.sys_ticks = 0;
		mcu->wdt.checked = 1;
	}

	/* Enable WDT if this is required */
	if ((mcu->wdt.always_on == 0U) && (mcu->wdt.on == 0U) &&
	                (DM(WDTCR) != wdtcr_buf) &&
	                (IS_SET(DM(WDTCR), WDE) == 1)) {
		MSIM_LOG_DEBUG("WDT is enabled");
		mcu->wdt.on = 1;
		mcu->wdt.sys_ticks = 0;
		mcu->wdt.ticks = 0;
	}

	/* Watch for attempts to adjust WDE (off) and/or WDP (prescaler) */
	if (wdce_cycles > 0U) {
		if (IS_CLEAR(DM(WDTCR), WDCE) == 1U) {
			/* Disable WDT if this is required */
			if (IS_CLEAR(DM(WDTCR), WDE) == 1U) {
				MSIM_LOG_DEBUG("WDT is disabled");
				mcu->wdt.on = 0;
				mcu->wdt.sys_ticks = 0;
				mcu->wdt.ticks = 0;
			}
			/* Adjust WDT prescaler if this is required */
			wdp = DM(WDTCR)&0x07;
			switch (wdp) {
			case 0:
				mcu->wdt.presc = 16*1024;
				break;
			case 1:
				mcu->wdt.presc = 32*1024;
				break;
			case 2:
				mcu->wdt.presc = 64*1024;
				break;
			case 3:
				mcu->wdt.presc = 128*1024;
				break;
			case 4:
				mcu->wdt.presc = 256*1024;
				break;
			case 5:
				mcu->wdt.presc = 512*1024;
				break;
			case 6:
				mcu->wdt.presc = 1024*1024;
				break;
			case 7:
				mcu->wdt.presc = 2048*1024;
				break;
			default:
				snprintf(mcu->log, sizeof mcu->log, "illegal "
				         "watchdog prescaler: WDP=0x%" PRIX8,
				         wdp);
				MSIM_LOG_ERROR(mcu->log);
				break;
			}
			wdce_cycles = 0;
		} else {
			/* WDCE bit is set, do nothing */
		}
		wdce_cycles--;
		if (wdce_cycles == 0U) {
			CLEAR(DM(WDTCR), WDCE);
		}
	}

	/* Watch for a timed sequence start */
	if ((IS_WRIT(mcu, WDTCR)) && (IS_SET(DM(WDTCR), WDCE) == 1U) &&
	                ((IS_SET(DM(WDTCR), WDE)) == 1U)) {
		/* It looks like a firmware is going to adjust WDT parameters
		 * within next four system clock cycles. */
		wdce_cycles = 4;
	}

	/* Update WDT if this one is enabled */
	if ((mcu->wdt.always_on == 1U) || (mcu->wdt.on == 1U)) {
		if (mcu->wdt.sys_ticks < mcu->wdt.sys_presc) {
			/* Pre-scaling system clock */
			mcu->wdt.sys_ticks++;
		} else {
			/* WDT oscillator clock */
			if (mcu->wdt.ticks < mcu->wdt.presc) {
				mcu->wdt.ticks++;
			} else {
				/* Watchdog MCU reset */
				mcu->pc = mcu->intr.reset_pc;
				DM(MCUCSR) |= (1<<WDRF);
				mcu->wdt.sys_ticks = 0;
				mcu->wdt.ticks = 0;
				mcu->wdt.on = 0;
				MSIM_LOG_DEBUG("Watchdog MCU reset");
			}
			mcu->wdt.sys_ticks = 0;
		}
	}
}

static void usart_transmit(struct MSIM_AVR *mcu)
{
	uint8_t buf[2];
	uint32_t buf_len = 1;
	uint8_t ucsz;
	uint8_t err = 0;
	int written;

	/* Find how many bits to transmit */
	if (((DM(UBRRH)>>UMSEL)&1) == 0U) {
		/* There is a UBRRH value stored in data memory after
		 * the last tick of the AVR decoder. */
		ucsz = (uint8_t)(((DM(UCSRB)>>UCSZ2)&1U)<<2) |
		       (uint8_t)(((ucsrc_buf>>UCSZ1)&1U)<<1) |
		       (uint8_t)((ucsrc_buf>>UCSZ0)&1U);
	} else {
		/* There is a UCSRC value stored in data memory after
		 * the last tick of the AVR decoder. */
		ucsz = (uint8_t)(((DM(UCSRB)>>UCSZ2)&1U)<<2) |
		       (uint8_t)(((DM(UCSRC)>>UCSZ1)&1U)<<1) |
		       (uint8_t)((DM(UCSRC)>>UCSZ0)&1U);
	}

	buf[1] = 0;
	switch (ucsz) {
	case 0:			/* 5-bit */
		buf[0] = mcu->usart.txb&0x1F;
		break;
	case 1:			/* 6-bit */
		buf[0] = mcu->usart.txb&0x3F;
		break;
	case 2:			/* 7-bit */
		buf[0] = mcu->usart.txb&0x7F;
		break;
	case 3:			/* 8-bit */
		buf[0] = mcu->usart.txb;
		break;
	case 7:			/* 9-bit */
		/* NOTE: Should all other bits of buf[1] be filled from the
		 * next portion of USART transmit data (and not with zeroes)?*/
		buf[0] = mcu->usart.txb;
		buf[1] = (DM(UCSRB)>>TXB8)&1U;
		buf_len = 2;
		break;
	default:
		snprintf(mcu->log, sizeof mcu->log, "these bits to select "
		         "USART character size are reserved: UCSZ=0x%" PRIX8,
		         ucsz);
		MSIM_LOG_ERROR(mcu->log);
		err = 1;
		break;
	}

	if ((err == 0) && (IS_CLEAR(DM(UCSRA), UDRE) == 1)) {
		if (mcu->pty.master_fd >= 0) {
			written = MSIM_AVR_PTYWrite(mcu, buf, buf_len);
			if (written != (int)buf_len) {
				snprintf(mcu->log, sizeof mcu->log, "failed "
				         "to feed PTY master with USART data, "
				         "slave=%s", mcu->pty.slave_name);
				MSIM_LOG_ERROR(mcu->log);
			}
#ifdef DEBUG
			snprintf(mcu->log, sizeof mcu->log, "-> 0x%" PRIX8
			         ", pc=0x%06" PRIX32, buf[0], mcu->pc);
			MSIM_LOG_DEBUG(mcu->log);
#endif
			/* Set UDRE flag */
			DM(UCSRA) |= (1<<UDRE);
			/* Should TXC be cleared here? */
			DM(UCSRA) |= (1<<TXC);
		} else {
			MSIM_LOG_DEBUG("cannot feed PTY master with USART "
			               "data: master_fd < 0");
		}
	}
}

static void usart_receive(struct MSIM_AVR *mcu)
{
	uint8_t buf[2];
	uint32_t buf_len = 1;
	uint32_t mask;
	uint8_t ucsz;
	uint8_t err = 0;
	int recv;

	/* Find how many bits to receive */
	if (((DM(UBRRH)>>UMSEL)&1) == 0U) {
		/* There is a UBRRH value stored in data memory after
		 * the last tick of the AVR decoder. */
		ucsz = (uint8_t)(((DM(UCSRB)>>UCSZ2)&1U)<<2) |
		       (uint8_t)(((ucsrc_buf>>UCSZ1)&1U)<<1) |
		       (uint8_t)((ucsrc_buf>>UCSZ0)&1U);
	} else {
		/* There is a UCSRC value stored in data memory after
		 * the last tick of the AVR decoder. */
		ucsz = (uint8_t)(((DM(UCSRB)>>UCSZ2)&1U)<<2) |
		       (uint8_t)(((DM(UCSRC)>>UCSZ1)&1U)<<1) |
		       (uint8_t)((DM(UCSRC)>>UCSZ0)&1U);
	}

	switch (ucsz) {
	case 0:			/* 5-bit */
		mask = 0x1F;
		break;
	case 1:			/* 6-bit */
		mask = 0x3F;
		break;
	case 2:			/* 7-bit */
		mask = 0x7F;
		break;
	case 3:			/* 8-bit */
		mask = 0xFF;
		break;
	case 7:			/* 9-bit */
		/* NOTE: Should all other bits of buf[1] be filled from the
		 * next portion of USART receive data (and not with zeroes)? */
		mask = 0xFF;
		buf_len = 2;
		break;
	default:
		snprintf(mcu->log, sizeof mcu->log, "these bits to select "
		         "USART character size are reserved: UCSZ=0x%" PRIX8,
		         ucsz);
		MSIM_LOG_ERROR(mcu->log);
		err = 1;
		break;
	}

	if ((err == 0) && (IS_CLEAR(DM(UCSRA), RXC) == 1)) {
		if (mcu->pty.master_fd >= 0) {
			recv = MSIM_AVR_PTYRead(mcu, buf, buf_len);
			if (recv != (int)buf_len) {
				snprintf(mcu->log, sizeof mcu->log, "failed "
				         "to read USART data from PTY master, "
				         "slave=%s", mcu->pty.slave_name);
				MSIM_LOG_ERROR(mcu->log);
			}
#ifdef DEBUG
			snprintf(mcu->log, sizeof mcu->log, "<- 0x%" PRIX8
			         ", mask 0x%" PRIX32, buf[0], mask);
			MSIM_LOG_DEBUG(mcu->log);
#endif
			DM(UDR) = 0;
			DM(UDR) |= buf[0]&mask;
			DM(UCSRB) &= ~(1<<TXB8);
			if ((buf_len == 2U) && ((buf[1]&1) == 1U)) {
				DM(UCSRB) |= (1<<TXB8);
			}
			DM(UCSRA) |= (1<<RXC);
		} else {
			MSIM_LOG_DEBUG("cannot read USART data from PTY "
			               "master: master_fd < 0");
		}
	}
}

static void timer1_normal(struct MSIM_AVR *mcu,
                          uint32_t presc, uint32_t *ticks,
                          uint8_t wgm1, uint8_t com1a, uint8_t com1b)
{
	uint32_t tcnt1;
	uint32_t ocr1a, ocr1b;

	tcnt1 = ((DM(TCNT1H)<<8)&0xFF00) | (DM(TCNT1L)&0xFF);
	ocr1a = ((DM(OCR1AH)<<8)&0xFF00) | (DM(OCR1AL)&0xFF);
	ocr1b = ((DM(OCR1BH)<<8)&0xFF00) | (DM(OCR1BL)&0xFF);

	if ((*ticks) < (presc-1U)) {
		(*ticks)++;
	} else if ((*ticks) > (presc-1U)) {
		snprintf(mcu->log, sizeof mcu->log, "number of timer1 ticks=%"
		         PRIu32 " should be <= (prescaler-1)=%" PRIu32
		         "; timer1 will not be updated", *ticks, (presc-1U));
		MSIM_LOG_ERROR(mcu->log);
	} else {
		if (tcnt1 == 0xFFFFU) {
			/* Reset Timer/Counter1 */
			tcnt1 = 0;
			/* Set Timer/Counter1 overflow flag */
			DM(TIFR) |= (1<<TOV1);
		} else if (tcnt1 == ocr1a) {
			DM(TIFR) |= (1<<OCF1A);
			timer1_oc1_nonpwm(mcu, com1a, com1b, A_CHAN);
			tcnt1++;
		} else if (tcnt1 == ocr1b) {
			DM(TIFR) |= (1<<OCF1B);
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

static void timer1_ctc(struct MSIM_AVR *mcu, uint32_t presc,
                       uint32_t *ticks, uint8_t wgm1, uint8_t com1a,
                       uint8_t com1b)
{
	/* Clear Timer on Compare Match mode */
	uint32_t tcnt1;
	uint32_t ocr1a, ocr1b;
	uint32_t icr1;
	uint32_t top;
	uint8_t err;

	tcnt1 = ((DM(TCNT1H)<<8)&0xFF00) | (DM(TCNT1L)&0xFF);
	ocr1a = ((DM(OCR1AH)<<8)&0xFF00) | (DM(OCR1AL)&0xFF);
	ocr1b = ((DM(OCR1BH)<<8)&0xFF00) | (DM(OCR1BL)&0xFF);
	icr1 = (((DM(ICR1H)<<8)&0xFF00) | (DM(ICR1L)&0xFF));
	err = 0;

	switch (wgm1) {
	case 4U:
		top = ocr1a;
		break;
	case 12U:
		top = icr1;
		break;
	default:
		/* All other values of WGM13:0 are not allowed in CTC Mode. */
		snprintf(mcu->log, sizeof mcu->log, "WGM13:0=%" PRIu8 " "
		         "is not allowed in CTC Mode of timer1; It looks like "
		         "a bug (please report at trac.mcusim.org)", wgm1);
		MSIM_LOG_ERROR(mcu->log);
		err = 1;
		break;
	}

	if ((err == 0U) && ((*ticks) < (presc-1U))) {
		(*ticks)++;
	} else if ((err == 0U) && ((*ticks) > (presc-1U))) {
		snprintf(mcu->log, sizeof mcu->log, "number of timer1 ticks=%"
		         PRIu32 " should be <= (prescaler-1)=%" PRIu32 "; "
		         "timer1 will not be updated", *ticks, (presc-1U));
		MSIM_LOG_ERROR(mcu->log);
	} else if (err == 0U) {
		if ((tcnt1 == top) || (tcnt1 == 0xFFFFU)) {
			if ((wgm1 == 4U) && (tcnt1 == ocr1a)) {
				DM(TIFR) |= (1<<OCF1A);
				timer1_oc1_nonpwm(mcu, com1a, com1b, A_CHAN);
			}
			if (tcnt1 == ocr1b) {
				DM(TIFR) |= (1<<OCF1B);
				timer1_oc1_nonpwm(mcu, com1a, com1b, B_CHAN);
			}
			if ((wgm1 == 12U) && (tcnt1 == icr1)) {
				DM(TIFR) |= (1<<ICF1);
			}

			/* Set Timer/Counter1 overflow flag at MAX only */
			if (tcnt1 == 0xFFFFU) {
				DM(TIFR) |= (1<<TOV1);
			}
			tcnt1 = 0;
		} else {
			if (tcnt1 == ocr1a) {
				DM(TIFR) |= (1<<OCF1A);
				timer1_oc1_nonpwm(mcu, com1a, com1b, A_CHAN);
			} else {
				/* Do nothing in this case */
			}
			if (tcnt1 == ocr1b) {
				DM(TIFR) |= (1<<OCF1B);
				timer1_oc1_nonpwm(mcu, com1a, com1b, B_CHAN);
			} else {
				/* Do nothing in this case */
			}
			if (tcnt1 == icr1) {
				DM(TIFR) |= (1<<ICF1);
			} else {
				/* Do nothing in this case */
			}
			tcnt1++;
		}

		DM(TCNT1H) = (tcnt1>>8)&0xFFU;
		DM(TCNT1L) = tcnt1&0xFFU;
		(*ticks) = 0;
	} else {
		MSIM_LOG_WARN("timer1 in CTC Mode was not updated due to "
		              "error occured earlier");
	}
}

static void timer1_fastpwm(struct MSIM_AVR *mcu, uint32_t presc,
                           uint32_t *ticks, uint8_t wgm1, uint8_t com1a,
                           uint8_t com1b)
{
	uint32_t tcnt1;
	uint32_t ocr1a, ocr1b;
	uint32_t icr1;
	uint32_t top;
	uint8_t err;

	tcnt1 = ((DM(TCNT1H)<<8)&0xFF00) | (DM(TCNT1L)&0xFF);
	ocr1a = ((DM(OCR1AH)<<8)&0xFF00) | (DM(OCR1AL)&0xFF);
	ocr1b = ((DM(OCR1BH)<<8)&0xFF00) | (DM(OCR1BL)&0xFF);
	icr1 = (((DM(ICR1H)<<8)&0xFF00) | (DM(ICR1L)&0xFF));
	err = 0;

	switch (wgm1) {
	case 5:
		top = 0x00FFU;
		break;
	case 6:
		top = 0x01FFU;
		break;
	case 7:
		top = 0x03FFU;
		break;
	case 14:
		if (icr1 < 0x0003U) {
			snprintf(mcu->log, sizeof mcu->log, "ICR1 = %" PRIu32
			         ", but minimum resolution for Fast PWM mode "
			         "of timer1 is 2-bit (0x0003)", icr1);
			MSIM_LOG_ERROR(mcu->log);
			top = 0x0003U;
		} else {
			top = icr1;
		}
		break;
	case 15:
		if (ocr1a_buf < 0x0003U) {
			snprintf(mcu->log, sizeof mcu->log, "OCR1A = %" PRIu32
			         ", but minimum resolution for Fast PWM mode "
			         "of timer1 is 2-bit (0x0003)", ocr1a_buf);
			MSIM_LOG_ERROR(mcu->log);
			top = 0x0003U;
		} else {
			top = ocr1a_buf;
		}
		break;
	default:
		/* All other values of WGM13:0 are not allowed in
		 * Fast PWM Mode. */
		snprintf(mcu->log, sizeof mcu->log, "WGM13:0=%" PRIu8 " is not"
		         " allowed in Fast PWM mode of timer1; It looks like "
		         "a bug (please report at trac.mcusim.org)", wgm1);
		MSIM_LOG_ERROR(mcu->log);
		err = 1;
		break;
	}

	if ((err == 0U) && ((*ticks) < (presc-1U))) {
		(*ticks)++;
	} else if ((err == 0U) && ((*ticks) > (presc-1U))) {
		snprintf(mcu->log, sizeof mcu->log, "number of timer1 ticks=%"
		         PRIu32 " should be <= (prescaler-1)=%" PRIu32 "; "
		         "timer1 will not be updated", *ticks, (presc-1U));
		MSIM_LOG_ERROR(mcu->log);
	} else if (err == 0U) {
		if ((tcnt1 == top) || (tcnt1 == 0xFFFFU)) {
			/* Set Timer/Counter1 overflow flag */
			DM(TIFR) |= (1<<TOV1);

			if ((wgm1 == 14U) && (tcnt1 == icr1)) {
				DM(TIFR) |= (1<<ICF1);
			}
			if ((wgm1 == 15U) && (tcnt1 == ocr1a_buf)) {
				DM(TIFR) |= (1<<OCF1A);
				timer1_oc1_fastpwm(mcu, com1a, com1b, A_CHAN,
				                   COMPARE_MATCH);
			}
			if (tcnt1 == ocr1b_buf) {
				DM(TIFR) |= (1<<OCF1B);
				timer1_oc1_fastpwm(mcu, com1a, com1b, B_CHAN,
				                   COMPARE_MATCH);
			}

			/* Update buffered values */
			ocr1a_buf = ocr1a;
			ocr1b_buf = ocr1b;

			/* Update OC1A/OC1B at BOTTOM */
			timer1_oc1_fastpwm(mcu, com1a, com1b, A_CHAN,
			                   SET_TO_BOTTOM);
			timer1_oc1_fastpwm(mcu, com1a, com1b, B_CHAN,
			                   SET_TO_BOTTOM);
			tcnt1 = 0;
		} else {
			if (tcnt1 == ocr1a_buf) {
				DM(TIFR) |= (1<<OCF1A);
				timer1_oc1_fastpwm(mcu, com1a, com1b, A_CHAN,
				                   COMPARE_MATCH);
			} else {
				/* Do nothing in this case */
			}
			if (tcnt1 == ocr1b_buf) {
				DM(TIFR) |= (1<<OCF1B);
				timer1_oc1_fastpwm(mcu, com1a, com1b, B_CHAN,
				                   COMPARE_MATCH);
			} else {
				/* Do nothing in this case */
			}
			if (tcnt1 == icr1) {
				DM(TIFR) |= (1<<ICF1);
			} else {
				/* Do nothing in this case */
			}
			tcnt1++;
		}

		DM(TCNT1H) = (tcnt1>>8)&0xFFU;
		DM(TCNT1L) = tcnt1&0xFFU;
		(*ticks) = 0;
	} else {
		MSIM_LOG_WARN("timer1 in Fast PWM mode was not updated due to "
		              "error occured earlier");
	}
}

static void timer1_pcpwm(struct MSIM_AVR *mcu, uint32_t presc,
                         uint32_t *ticks, uint8_t wgm1, uint8_t com1a,
                         uint8_t com1b)
{
	/* Phase Correct PWM mode.
	 *
	 * NOTE: This mode allows PWM to be generated using dual-slope
	 * operation (preferred for motor control applications). Duty cycle
	 * can be controlled by the fixed values 0x00FF, 0x01FF, or 0x03FF
	 * (WGM13:0 = 1, 2, or 3), the value in ICR1 (WGM13:0 = 10), or
	 * the value in OCR1A (WGM13:0 = 11).
	 *
	 * Dual-slope operation means counting from BOTTOM to TOP,
	 * then from TOP back to the BOTTOM and start again. */
	static uint8_t cnt_down = 0;
	uint32_t tcnt1;
	uint32_t ocr1a, ocr1b;
	uint32_t icr1;
	uint32_t top;
	uint8_t err;

	tcnt1 = ((DM(TCNT1H)<<8)&0xFF00) | (DM(TCNT1L)&0xFF);
	ocr1a = ((DM(OCR1AH)<<8)&0xFF00) | (DM(OCR1AL)&0xFF);
	ocr1b = ((DM(OCR1BH)<<8)&0xFF00) | (DM(OCR1BL)&0xFF);
	icr1 = (((DM(ICR1H)<<8)&0xFF00) | (DM(ICR1L)&0xFF));
	err = 0;

	switch (wgm1) {
	case 1:
		top = 0x00FFU;
		break;
	case 2:
		top = 0x01FFU;
		break;
	case 3:
		top = 0x03FFU;
		break;
	case 10:
		if (icr1 < 0x0003U) {
			snprintf(mcu->log, sizeof mcu->log, "ICR1 = %" PRIu32
			         ", but minimum resolution for Phase Correct "
			         "PWM mode of timer1 is 2-bit, i.e. "
			         "ICR1 = 0x0003", icr1);
			MSIM_LOG_ERROR(mcu->log);
			top = 0x0003U;
		} else {
			top = icr1;
		}
		break;
	case 11:
		if (ocr1a_buf < 0x0003U) {
			snprintf(mcu->log, sizeof mcu->log, "OCR1A = %" PRIu32
			         ", but minimum resolution for Phase Correct "
			         "PWM mode of timer1 is 2-bit, i.e. "
			         "OCR1A = 0x0003", ocr1a_buf);
			MSIM_LOG_ERROR(mcu->log);
			top = 0x0003U;
		} else {
			top = ocr1a_buf;
		}
		break;
	default:
		/* All other values of WGM13:0 are not allowed in
		 * Phase Correct PWM mode. */
		snprintf(mcu->log, sizeof mcu->log, "WGM13:0=%" PRIu8
		         " is not allowed in Phase Correct PWM mode of "
		         "timer1; It looks like a bug (please report it "
		         "at trac.mcusim.org)", wgm1);
		MSIM_LOG_FATAL(mcu->log);
		err = 1;
		break;
	}

	if ((err == 0U) && ((*ticks) < (presc-1U))) {
		(*ticks)++;
	} else if ((err == 0U) && ((*ticks) > (presc-1U))) {
		snprintf(mcu->log, sizeof mcu->log, "number of timer1 ticks=%"
		         PRIu32 " should be <= (prescaler-1)=%" PRIu32
		         "; timer1 will not be updated", *ticks, (presc-1U));
		MSIM_LOG_ERROR(mcu->log);
	} else if (err == 0U) {
		if ((tcnt1 == top) || (tcnt1 == 0xFFFFU)) {
			if ((wgm1 == 11U) && (tcnt1 == ocr1a_buf)) {
				DM(TIFR) |= (1<<OCF1A);
				timer1_oc1_pcpwm(mcu, com1a, com1b, A_CHAN,
				                 COMP_MATCH_UPCNT);
			}
			if (tcnt1 == ocr1b) {
				DM(TIFR) |= (1<<OCF1B);
				timer1_oc1_pcpwm(mcu, com1a, com1b, B_CHAN,
				                 COMP_MATCH_UPCNT);
			}
			if ((wgm1 == 10U) && (tcnt1 == icr1)) {
				DM(TIFR) |= (1<<ICF1);
			}

			cnt_down = 1;
			ocr1a_buf = ((DM(OCR1AH)<<8)&0xFF00) |
			            (DM(OCR1AL)&0x00FF);
			ocr1b_buf = ((DM(OCR1BH)<<8)&0xFF00) |
			            (DM(OCR1BL)&0x00FF);
			tcnt1--;
		} else if (tcnt1 == 0U) {
			cnt_down = 0;
			/* Set Timer/Counter1 overflow flag at BOTTOM only */
			DM(TIFR) |= (1<<TOV1);
			tcnt1++;
		} else {
			if (tcnt1 == ocr1a_buf) {
				DM(TIFR) |= (1<<OCF1A);
				timer1_oc1_pcpwm(mcu, com1a, com1b, A_CHAN,
				                 (cnt_down == 1)
				                 ? COMP_MATCH_DOWNCNT
				                 : COMP_MATCH_UPCNT);
			} else {
				/* Do nothing in this case */
			}
			if (tcnt1 == ocr1b_buf) {
				DM(TIFR) |= (1<<OCF1B);
				timer1_oc1_pcpwm(mcu, com1a, com1b, B_CHAN,
				                 (cnt_down == 1)
				                 ? COMP_MATCH_DOWNCNT
				                 : COMP_MATCH_UPCNT);
			} else {
				/* Do nothing in this case */
			}
			if (tcnt1 == icr1) {
				DM(TIFR) |= (1<<ICF1);
			} else {
				/* Do nothing in this case */
			}
			if (cnt_down == 1) {
				tcnt1--;
			} else {
				tcnt1++;
			}
		}

		DM(TCNT1H) = (tcnt1>>8)&0xFFU;
		DM(TCNT1L) = tcnt1&0xFFU;
		(*ticks) = 0;
	} else {
		MSIM_LOG_WARN("timer1 in Phase Correct mode was not updated "
		              "due to error occured earlier");
	}
}

static void timer1_pfcpwm(struct MSIM_AVR *mcu, uint32_t presc,
                          uint32_t *ticks, uint8_t wgm1, uint8_t com1a,
                          uint8_t com1b)
{
	/* Phase and Frequency Correct PWM mode.
	 *
	 * NOTE: This mode allows PWM to be generated using dual-slope
	 * operation (preferred for motor control applications). Duty cycle
	 * can be controlled by the value in ICR1 (WGM13:0 = 8) or the value
	 * in OCR1A (WGM13:0 = 9).
	 *
	 * Dual-slope operation means counting from BOTTOM to TOP,
	 * then from TOP back to the BOTTOM and start again.
	 *
	 * The main difference between the Phase and Frequency Correct PWM
	 * mode and the Phase Correct one is the OCR1x register is updated by
	 * the OCR1x buffer register (at the BOTTOM for the first one,
	 * at the TOP for the last one).
	 */
	static uint8_t cnt_down = 0;
	uint32_t tcnt1;
	uint32_t ocr1a, ocr1b;
	uint32_t icr1;
	uint32_t top;
	uint8_t err;

	tcnt1 = ((DM(TCNT1H)<<8)&0xFF00) | (DM(TCNT1L)&0xFF);
	ocr1a = ((DM(OCR1AH)<<8)&0xFF00) | (DM(OCR1AL)&0xFF);
	ocr1b = ((DM(OCR1BH)<<8)&0xFF00) | (DM(OCR1BL)&0xFF);
	icr1 = (((DM(ICR1H)<<8)&0xFF00) | (DM(ICR1L)&0xFF));
	err = 0;

	switch (wgm1) {
	case 8:
		if (icr1 < 0x0003U) {
			snprintf(mcu->log, sizeof mcu->log, "ICR1 = %" PRIu32
			         ", but minimum resolution for Phase and "
			         "Frequency Correct PWM mode of timer1 is "
			         "2-bit, i.e. ICR1 = 0x0003", icr1);
			MSIM_LOG_ERROR(mcu->log);
			top = 0x0003U;
		} else {
			top = icr1;
		}
		break;
	case 9:
		if (ocr1a_buf < 0x0003U) {
			snprintf(mcu->log, sizeof mcu->log, "OCR1A = %" PRIu32
			         ", but minimum resolution for Phase and "
			         "Frequency Correct PWM mode of timer1 is "
			         "2-bit, i.e. OCR1A = 0x0003", ocr1a_buf);
			MSIM_LOG_ERROR(mcu->log);
			top = 0x0003U;
		} else {
			top = ocr1a_buf;
		}
		break;
	default:
		/* All other values of WGM13:0 are not allowed in
		 * Phase and Frequency Correct PWM mode. */
		snprintf(mcu->log, sizeof mcu->log, "WGM13:0=%" PRIu8
		         " is not allowed in Phase and Frequency Correct PWM "
		         "mode of timer1; It looks like a bug (please report "
		         "it at trac.mcusim.org)", wgm1);
		MSIM_LOG_FATAL(mcu->log);
		err = 1;
		break;
	}

	if ((err == 0U) && ((*ticks) < (presc-1U))) {
		(*ticks)++;
	} else if ((err == 0U) && ((*ticks) > (presc-1U))) {
		snprintf(mcu->log, sizeof mcu->log, "number of timer1 ticks=%"
		         PRIu32 " should be <= (prescaler-1)=%" PRIu32
		         "; timer1 will not be updated", *ticks, (presc-1U));
		MSIM_LOG_ERROR(mcu->log);
	} else if (err == 0U) {
		if ((tcnt1 == top) || (tcnt1 == 0xFFFFU)) {
			if ((wgm1 == 9U) && (tcnt1 == ocr1a_buf)) {
				DM(TIFR) |= (1<<OCF1A);
				timer1_oc1_pcpwm(mcu, com1a, com1b, A_CHAN,
				                 COMP_MATCH_UPCNT);
			}
			if (tcnt1 == ocr1b) {
				DM(TIFR) |= (1<<OCF1B);
				timer1_oc1_pcpwm(mcu, com1a, com1b, B_CHAN,
				                 COMP_MATCH_UPCNT);
			}
			if ((wgm1 == 8U) && (tcnt1 == icr1)) {
				DM(TIFR) |= (1<<ICF1);
			}

			cnt_down = 1;
			tcnt1--;
		} else if (tcnt1 == 0U) {
			cnt_down = 0;
			/* Update OCR1x values at BOTTOM */
			ocr1a_buf = ((DM(OCR1AH)<<8)&0xFF00) |
			            (DM(OCR1AL)&0x00FF);
			ocr1b_buf = ((DM(OCR1BH)<<8)&0xFF00) |
			            (DM(OCR1BL)&0x00FF);
			/* Set Timer/Counter1 overflow flag at BOTTOM only */
			DM(TIFR) |= (1<<TOV1);
			tcnt1++;
		} else {
			if (tcnt1 == ocr1a_buf) {
				DM(TIFR) |= (1<<OCF1A);
				timer1_oc1_pcpwm(mcu, com1a, com1b, A_CHAN,
				                 (cnt_down == 1)
				                 ? COMP_MATCH_DOWNCNT
				                 : COMP_MATCH_UPCNT);
			} else {
				/* Do nothing in this case */
			}
			if (tcnt1 == ocr1b_buf) {
				DM(TIFR) |= (1<<OCF1B);
				timer1_oc1_pcpwm(mcu, com1a, com1b, B_CHAN,
				                 (cnt_down == 1)
				                 ? COMP_MATCH_DOWNCNT
				                 : COMP_MATCH_UPCNT);
			} else {
				/* Do nothing in this case */
			}
			if (tcnt1 == icr1) {
				DM(TIFR) |= (1<<ICF1);
			} else {
				/* Do nothing in this case */
			}
			if (cnt_down == 1) {
				tcnt1--;
			} else {
				tcnt1++;
			}
		}

		DM(TCNT1H) = (tcnt1>>8)&0xFFU;
		DM(TCNT1L) = tcnt1&0xFFU;
		(*ticks) = 0;
	} else {
		MSIM_LOG_WARN("timer1 in Phase and Frequency Correct mode was "
		              "not updated due to error occured earlier");
	}
}

static void timer1_oc1_nonpwm(struct MSIM_AVR *mcu, uint8_t com1a,
                              uint8_t com1b, uint8_t chan)
{
	uint8_t pin, com;

	/* Check Data Direction Register first. DDRB1 or DDRB2 should
	 * be set to enable the output driver (according to a datasheet). */
	if (chan == (uint8_t)A_CHAN) {
		pin = PB1;
		com = com1a;
	} else if (chan == (uint8_t)B_CHAN) {
		pin = PB2;
		com = com1b;
	} else {
		MSIM_LOG_ERROR("unsupported channel of Output Compare unit is "
		               "used in timer1; It looks like a bug (please "
		               "report it at trac.mcusim.org)");
		com = NOT_CONNECTED;
	}

	if ((com != NOT_CONNECTED) && (!IS_SET(DM(DDRB), pin))) {
		com = NOT_CONNECTED;
	}

	/* Update Output Compare pin (OC1A or OC1B) */
	if (com != NOT_CONNECTED) {
		switch (com) {
		case 1:
			if (IS_SET(DM(PORTB), pin) == 1) {
				CLEAR(DM(PORTB), pin);
			} else {
				SET(DM(PORTB), pin);
			}
			break;
		case 2:
			CLEAR(DM(PORTB), pin);
			break;
		case 3:
			SET(DM(PORTB), pin);
			break;
		case 0:
		default:
			/* OC1A/OC1B disconnected, do nothing */
			break;
		}
	}
}

static void timer1_oc1_fastpwm(struct MSIM_AVR *mcu, uint8_t com1a,
                               uint8_t com1b, uint8_t chan, uint8_t state)
{
	uint8_t pin, com;
	uint8_t wgm1;

	wgm1 = (uint8_t) (((DM(TCCR1B)>>WGM13)&1)<<3) |
	       (uint8_t) (((DM(TCCR1B)>>WGM12)&1)<<2) |
	       (uint8_t) (((DM(TCCR1A)>>WGM11)&1)<<1) |
	       (uint8_t) (((DM(TCCR1A)>>WGM10)&1));

	/* Check Data Direction Register first. DDRB1 or DDRB2 should
	 * be set to enable the output driver (according to a datasheet). */
	if (chan == (uint8_t)A_CHAN) {
		pin = PB1;
		com = com1a;
	} else if (chan == (uint8_t)B_CHAN) {
		pin = PB2;
		com = com1b;
	} else {
		MSIM_LOG_ERROR("unsupported channel of Output Compare unit is "
		               "used in timer1; It looks like a bug (please "
		               "report it at trac.mcusim.org)");
		com = NOT_CONNECTED;
	}

	if ((com != NOT_CONNECTED) && (!IS_SET(DM(DDRB), pin))) {
		com = NOT_CONNECTED;
	}

	/* Update Output Compare pin (OC1A or OC1B) */
	if (com != NOT_CONNECTED) {
		switch (com) {
		case 1:
			/* Only OC1A will be toggled on Compare Match
			 * with WGM13:0 = 15 */
			if ((wgm1 == 15U) && (pin == PB1) &&
			                (state == (uint8_t)COMPARE_MATCH)) {
				if (IS_SET(DM(PORTB), pin) == 1) {
					CLEAR(DM(PORTB), pin);
				} else {
					SET(DM(PORTB), pin);
				}
			}
			break;
		case 2:
			if (state == (uint8_t)COMPARE_MATCH) {
				CLEAR(DM(PORTB), pin);
			} else {
				SET(DM(PORTB), pin);
			}
			break;
		case 3:
			if (state == (uint8_t)COMPARE_MATCH) {
				SET(DM(PORTB), pin);
			} else {
				CLEAR(DM(PORTB), pin);
			}
			break;
		case 0:
		default:
			/* OC1A/OC1B disconnected, do nothing */
			break;
		}
	}
}

static void timer1_oc1_pcpwm(struct MSIM_AVR *mcu, uint8_t com1a,
                             uint8_t com1b, uint8_t chan, uint8_t s)
{
	uint8_t pin, com;
	uint8_t wgm1;

	wgm1 = (uint8_t) (((DM(TCCR1B)>>WGM13)&1)<<3) |
	       (uint8_t) (((DM(TCCR1B)>>WGM12)&1)<<2) |
	       (uint8_t) (((DM(TCCR1A)>>WGM11)&1)<<1) |
	       (uint8_t) (((DM(TCCR1A)>>WGM10)&1));

	/* Check Data Direction Register first. DDRB1 or DDRB2 should
	 * be set to enable the output driver (according to a datasheet). */
	if (chan == (uint8_t)A_CHAN) {
		pin = PB1;
		com = com1a;
	} else if (chan == (uint8_t)B_CHAN) {
		pin = PB2;
		com = com1b;
	} else {
		MSIM_LOG_ERROR("unknown channel of the Output Compare "
		               "unit is used in timer1; it looks like a bug "
		               "(please report it at trac.mcusim.org)");
		com = NOT_CONNECTED;
	}
	if (s == (uint8_t)COMPARE_MATCH) {
		MSIM_LOG_ERROR("COMP_MATCH_UPCNT or COMP_MATCH_DOWNCNT should "
		               "be used instead of COMPARE_MATCH state in "
		               "Phase Correct mode of timer1");
		com = NOT_CONNECTED;
	}
	if ((com != NOT_CONNECTED) && (!IS_SET(DM(DDRB), pin))) {
		com = NOT_CONNECTED;
	}

	/* Update Output Compare pin (OC1A or OC1B) */
	if (com != NOT_CONNECTED) {
		switch (com) {
		case 1:
			/* Only OC1A will be toggled on Compare Match
			 * with WGM13:0 = 9 or 14 */
			if (((s == (uint8_t)COMP_MATCH_DOWNCNT) ||
			                (s == (uint8_t)COMP_MATCH_UPCNT)) &&
			                ((wgm1 == 9U) || (wgm1 == 14U)) &&
			                (pin == PB1)) {
				if (IS_SET(DM(PORTB), pin) == 1) {
					CLEAR(DM(PORTB), pin);
				} else {
					SET(DM(PORTB), pin);
				}
			}
			break;
		case 2:
			if (s == (uint8_t)COMP_MATCH_UPCNT) {
				CLEAR(DM(PORTB), pin);
			} else if (s == (uint8_t)COMP_MATCH_DOWNCNT) {
				SET(DM(PORTB), pin);
			} else {
				/* Do nothing in this case. */
			}
			break;
		case 3:
			if (s == (uint8_t)COMP_MATCH_UPCNT) {
				SET(DM(PORTB), pin);
			} else if (s == (uint8_t)COMP_MATCH_DOWNCNT) {
				CLEAR(DM(PORTB), pin);
			} else {
				/* Do nothing in this case. */
			}
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
		snprintf(mcu->log, sizeof mcu->log, "number of timer2 ticks=%"
		         PRIu32 " should be <= (prescaler-1)=%" PRIu32 "; "
		         "timer2 will not be updated", *ticks, (presc-1U));
		MSIM_LOG_ERROR(mcu->log);
	} else {
		if (DM(TCNT2) == 0xFF) {
			/* Reset Timer/Counter2 */
			DM(TCNT2) = 0;
			/* Set Timer/Counter2 overflow flag */
			DM(TIFR) |= (1<<TOV2);
		} else if (DM(TCNT2) == DM(OCR2)) {
			DM(TIFR) |= (1<<OCF2);
			timer2_oc2_nonpwm(mcu, com2);
			DM(TCNT2)++;
		} else {
			DM(TCNT2)++;
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
		snprintf(mcu->log, sizeof mcu->log, "number of timer2 ticks=%"
		         PRIu32 " should be <= (prescaler-1)=%" PRIu32 "; "
		         "timer2 will not be updated", *ticks, (presc-1U));
		MSIM_LOG_ERROR(mcu->log);
	} else {
		if (DM(TCNT2) == DM(OCR2)) {
			/* Reset Timer/Counter2 */
			DM(TCNT2) = 0;
			/* Set Timer/Counter2 output compare flag */
			DM(TIFR) |= (1<<OCF2);

			timer2_oc2_nonpwm(mcu, com2);
		} else {
			DM(TCNT2)++;
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
		snprintf(mcu->log, sizeof mcu->log, "number of timer2 ticks=%"
		         PRIu32 " should be <= (prescaler-1)=%" PRIu32 "; "
		         "timer2 will not be updated", *ticks, (presc-1U));
		MSIM_LOG_ERROR(mcu->log);
	} else {
		if (DM(TCNT2) == 0xFF) {
			DM(TCNT2) = 0;
			ocr2_buf = DM(OCR2);
			timer2_oc2_fastpwm(mcu, com2, SET_TO_BOTTOM);
			DM(TIFR) |= (1<<TOV2);
		} else if (DM(TCNT2) == ocr2_buf) {
			DM(TIFR) |= (1<<OCF2);
			timer2_oc2_fastpwm(mcu, com2, COMPARE_MATCH);
			DM(TCNT2)++;
		} else {
			DM(TCNT2)++;
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
		snprintf(mcu->log, sizeof mcu->log, "number of timer2 ticks=%"
		         PRIu32 " should be <= (prescaler-1)=%" PRIu32 "; "
		         "timer2 will not be updated", *ticks, (presc-1U));
		MSIM_LOG_ERROR(mcu->log);
	} else {
		if (DM(TCNT2) == 0xFF) {
			if (ocr2_buf == 0xFFU) {
				DM(TIFR) |= (1<<OCF2);
			}
			if (missed_cm || ((ocr2_buf == 0xFFU) &&
			                  (DM(OCR2) < 0xFF))) {
				timer2_oc2_pcpwm(mcu, com2, COMP_MATCH_UPCNT);
			}

			cnt_down = 1;
			ocr2_buf = DM(OCR2);
			if (ocr2_buf == 0xFFU) {
				timer2_oc2_pcpwm(mcu, com2,
				                 COMP_MATCH_DOWNCNT);
			}
			DM(TCNT2)--;
		} else if (DM(TCNT2) == 0) {
			cnt_down = 0;
			DM(TIFR) |= (1<<TOV2);
			DM(TCNT2)++;
		} else if (DM(TCNT2) == ocr2_buf) {
			DM(TIFR) |= (1<<OCF2);
			timer2_oc2_pcpwm(mcu, com2, cnt_down
			                 ? COMP_MATCH_DOWNCNT
			                 : COMP_MATCH_UPCNT);
			if (!cnt_down) {
				DM(TCNT2)++;
			} else {
				DM(TCNT2)--;
			}
		} else {
			if (!cnt_down) {
				DM(TCNT2)++;
			} else {
				DM(TCNT2)--;
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
	if (!IS_SET(DM(DDRB), PB3)) {
		com2_v = NOT_CONNECTED;
	} else {
		com2_v = com2;
	}

	/* Update Output Compare pin (OC2) */
	switch (com2) {
	case 1:
		if (IS_SET(DM(PORTB), PB3) == 1) {
			CLEAR(DM(PORTB), PB3);
		} else {
			SET(DM(PORTB), PB3);
		}
		break;
	case 2:
		CLEAR(DM(PORTB), PB3);
		break;
	case 3:
		SET(DM(PORTB), PB3);
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
	if (!IS_SET(DM(DDRB), PB3)) {
		com2_v = NOT_CONNECTED;
	} else {
		com2_v = com2;
	}

	/* Update Output Compare pin (OC2) */
	switch (com2) {
	case 1:
		if (state == (uint8_t)COMPARE_MATCH) {
			if (IS_SET(DM(PORTB), PB3) == 1) {
				CLEAR(DM(PORTB), PB3);
			} else {
				SET(DM(PORTB), PB3);
			}
		}
		break;
	case 2:		/* Non-inverting compare output mode */
		if (state == (uint8_t)COMPARE_MATCH) {
			CLEAR(DM(PORTB), PB3);
		} else {
			SET(DM(PORTB), PB3);
		}
		break;
	case 3:		/* Inverting compare output mode */
		if (state == (uint8_t)COMPARE_MATCH) {
			SET(DM(PORTB), PB3);
		} else {
			CLEAR(DM(PORTB), PB3);
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
	if (!IS_SET(DM(DDRB), PB3)) {
		com2_v = NOT_CONNECTED;
	} else {
		com2_v = com2;
	}

	/* Update Output Compare pin (OC2) */
	switch (com2_v) {
	case 1:
		MSIM_LOG_WARN("COM21:0=1 is reserved in Phase Correct "
		              "PWM mode");
		break;
	case 2:		/* Non-inverting compare output mode */
		if (state == (uint8_t)COMP_MATCH_UPCNT) {
			CLEAR(DM(PORTB), PB3);
		} else {
			SET(DM(PORTB), PB3);
		}
		break;
	case 3:		/* Inverting compare output mode */
		if (state == (uint8_t)COMP_MATCH_UPCNT) {
			SET(DM(PORTB), PB3);
		} else {
			CLEAR(DM(PORTB), PB3);
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
		snprintf(mcu->log, sizeof mcu->log, "fuse #%u is not "
		         "supported by %s", fuse_n, mcu->name);
		MSIM_LOG_ERROR(mcu->log);
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
					mcu->freq = 1000000;	/* max 1 MHz */
					break;
				case 2:
					mcu->freq = 2000000;	/* max 2 MHz */
					break;
				case 3:
					mcu->freq = 4000000;	/* max 4 MHz */
					break;
				case 4:
					mcu->freq = 8000000;	/* max 8 MHz */
					break;
				default:
					/* Shouldn't happen! */
					snprintf(mcu->log, sizeof mcu->log,
					         "CKSEL = %" PRIu8 ", but it "
					         "should be within [1,4] "
					         "inclusively", cksel);
					MSIM_LOG_ERROR(mcu->log);
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
					snprintf(mcu->log, sizeof mcu->log,
					         "CKSEL = %" PRIu8 ", but it "
					         "should be within [5,8] "
					         "inclusively", cksel);
					MSIM_LOG_ERROR(mcu->log);
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
					snprintf(mcu->log, sizeof mcu->log,
					         "(CKSEL>>1) = %" PRIu8 ", "
					         "but it should be within "
					         "[5,7] inclusively", cksel);
					MSIM_LOG_ERROR(mcu->log);
					break;
				}
				if (!ckopt) {
					/* max 16 MHz */
					mcu->freq = 16000000;
				}
			} else {
				/* Shouldn't happen! */
				snprintf(mcu->log, sizeof mcu->log, "CKSEL = %"
				         PRIu8 ", but it should be within "
				         "[0,15] inclusively", cksel);
				MSIM_LOG_ERROR(mcu->log);
			}
			break;
		case FUSE_HIGH:
			/* Do we have WDT always on? */
			if (((fuse_v>>6)&1) == 0) {
				mcu->wdt.always_on = 1;
				mcu->wdt.on = 1;
				MSIM_LOG_WARN("WDT is always ON (WDTON bit is "
				              "programmed/cleared)");
			} else {
				mcu->wdt.always_on = 0;
				mcu->wdt.on = 0;
			}

			bootsz = (fuse_v>>1)&0x3U;
			ckopt = (fuse_v>>4)&0x1U;
			switch (bootsz) {
			case 3:
				mcu->bls.start = 0x1F00;
				mcu->bls.end = 0x1FFF;
				mcu->bls.size = 256;
				break;
			case 2:
				mcu->bls.start = 0x1E00;
				mcu->bls.end = 0x1FFF;
				mcu->bls.size = 512;
				break;
			case 1:
				mcu->bls.start = 0x1C00;
				mcu->bls.end = 0x1FFF;
				mcu->bls.size = 1024;
				break;
			case 0:
				mcu->bls.start = 0x1800;
				mcu->bls.end = 0x1FFF;
				mcu->bls.size = 2048;
				break;
			default:
				/* Shouldn't happen! */
				snprintf(mcu->log, sizeof mcu->log, "BOOTSZ = "
				         "%" PRIu8 ", but it should be within "
				         "[0,3] inclusively", bootsz);
				MSIM_LOG_ERROR(mcu->log);
				break;
			}

			if ((fuse_v&1U) == 1U) {
				/* BOOTRST is 1(unprogrammed) */
				mcu->intr.reset_pc = 0x0000;
				mcu->pc = 0x0000;
			} else {
				/* BOOTRST is 0(programmed) */
				mcu->intr.reset_pc = mcu->bls.start;
				mcu->pc = mcu->bls.start;
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
				snprintf(mcu->log, sizeof mcu->log, "CKSEL = %"
				         PRIu8 ", but it should be within "
				         "[5,7] inclusively", cksel);
				MSIM_LOG_ERROR(mcu->log);
				break;
			}
			if (!ckopt) {
				mcu->freq = 16000000;	/* max 16 MHz */
			}

			break;
		default:			/* Should not happen */
			snprintf(mcu->log, sizeof mcu->log, "Unknown fuse = %"
			         PRIu32 ", %s will not be modified",
			         fuse_n, mcu->name);
			MSIM_LOG_ERROR(mcu->log);
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

int MSIM_M8APassIRQs(struct MSIM_AVR *mcu)
{
	uint8_t timsk;
	uint8_t tifr;

	timsk = DM(TIMSK);		/* Timer Interrupt Mask Register */
	tifr = DM(TIFR);		/* Timer Interrupt Flag Register */

	/* Timer0 interrupts */
	if (((timsk>>TOIE0)&1U) && ((tifr>>TOV0)&1U)) {
		mcu->intr.irq[TIMER0_OVF_vect_num-1] = 1;
		mcu->dm[TIFR] &= ~(1<<TOV0);
	}

	/* Timer2 interrupts */
	if (((timsk>>TOIE2)&1U) && ((tifr>>TOV2)&1U)) {
		mcu->intr.irq[TIMER2_OVF_vect_num-1] = 1;
		mcu->dm[TIFR] &= ~(1<<TOV2);
	}
	if (((timsk>>OCIE2)&1U) && ((tifr>>OCF2)&1U)) {
		mcu->intr.irq[TIMER2_COMP_vect_num-1] = 1;
		mcu->dm[TIFR] &= ~(1<<OCF2);
	}

	/* Timer1 interrupts */
	if (((timsk>>TOIE1)&1U) && ((tifr>>TOV1)&1U)) {
		mcu->intr.irq[TIMER1_OVF_vect_num-1] = 1;
		mcu->dm[TIFR] &= ~(1<<TOV1);
	}
	if (((timsk>>OCIE1A)&1U) && ((tifr>>OCF1A)&1U)) {
		mcu->intr.irq[TIMER1_COMPA_vect_num-1] = 1;
		mcu->dm[TIFR] &= ~(1<<OCF1A);
	}
	if (((timsk>>OCIE1B)&1U) && ((tifr>>OCF1B)&1U)) {
		mcu->intr.irq[TIMER1_COMPB_vect_num-1] = 1;
		mcu->dm[TIFR] &= ~(1<<OCF1B);
	}
	if (((timsk>>TICIE1)&1U) && ((tifr>>ICF1)&1U)) {
		mcu->intr.irq[TIMER1_CAPT_vect_num-1] = 1;
		mcu->dm[TIFR] &= ~(1<<ICF1);
	}

	/* USART interrupts */
	if (((DM(UCSRB)>>UDRIE)&1U) && ((DM(UCSRA)>>UDRE)&1U)) {
		mcu->intr.irq[USART_UDRE_vect_num-1] = 1;
		/* 24.6.3. Transmitter Flags and Interrupts
		 *
		 * When interrupt-driven data transmission is used, the Data
		 * Register empty Interrupt routine must either write new data
		 * to UDR in order to clear UDRE or disable the Data Register
		 * empty Interrupt, otherwise a new interrupt will occur once
		 * the interrupt routine terminates.
		 *
		 * DM(UCSRA) &= ~(1<<UDRE); */
	}
	if (((DM(UCSRB)>>TXCIE)&1U) && ((DM(UCSRA)>>TXC)&1U)) {
		mcu->intr.irq[USART_TXC_vect_num-1] = 1;
		DM(UCSRA) &= ~(1<<TXC);
	}
	if (((DM(UCSRB)>>RXCIE)&1U) && ((DM(UCSRA)>>RXC)&1U)) {
		mcu->intr.irq[USART_RXC_vect_num-1] = 1;
		/* RXC flag should not be cleared automatically, but by
		 * reading UDR register. This behavior will have to be
		 * adjusted further.
		 * See 24.7.3. Receive Compete Flag and Interrupt */
		DM(UCSRA) &= ~(1<<RXC);
	}

	return 0;
}

