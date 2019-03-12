/*
 * Copyright 2017-2019 The MCUSim Project.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the MCUSim or its parts nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
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
 */
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "mcusim/mcusim.h"
#include "mcusim/log.h"
#include "mcusim/avr/sim/m328/m328p.h"
#include "mcusim/avr/sim/mcu_init.h"

#define FUSE_LOW		0
#define FUSE_HIGH		1
#define FUSE_EXT		2
#define IS_SET(byte, bit)	(((byte)&(1UL<<(bit)))>>(bit))
#define IS_RISE(init, val, bit)	((!((init>>bit)&1)) & ((val>>bit)&1))
#define IS_FALL(init, val, bit)	(((init>>bit)&1) & (!((val>>bit)&1)))
#define CLEAR(byte, bit)	((byte)=(uint8_t)((byte)&(uint8_t)(~(1<<(bit)))))
#define SET(byte, bit)		((byte)=(uint8_t)((byte)|(uint8_t)(1<<(bit))))

#define DM(v)			(mcu->dm[v])

#define NOT_CONNECTED		0xFFU
#define TC0_TOP			0xFFU
#define TC0_BOTTOM		0x00U

/* Two arbitrary constants to mark two distinct output compare channels of
 * the microcontroller. A_CHAN - OCRnA  B_CHAN - OCRnB */
#define A_CHAN			79
#define B_CHAN			80

/* Initial PORTD and PIND values and to track PD4(T0) and PD5(T1). */
static uint8_t init_portd;
static uint8_t init_pind;

/* Timer may start counting from a value higher then the one on OCR0 and
 * for that reason misses the Compare Match. This flag is set in this case. */
static uint8_t missed_cm = 0;

static void tick_timer0(struct MSIM_AVR *mcu);
static void update_watched(struct MSIM_AVR *mcu);

/* Timer/Counter0 modes of operation */
static void timer0_normal(struct MSIM_AVR *mcu,
                          uint32_t presc, uint32_t *ticks,
                          uint8_t wgm0, uint8_t com0a, uint8_t com0b);

static void timer0_ctc(struct MSIM_AVR *mcu,
                       uint32_t presc, uint32_t *ticks,
                       uint8_t wgm0, uint8_t com0a, uint8_t com0b);

/* Timer/Counter0 helper functions */
static void timer0_oc0_nonpwm(struct MSIM_AVR *mcu, uint8_t com0a,
                              uint8_t com0b, uint8_t chan);

int MSIM_M328PInit(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args)
{
	int r = mcu_init(mcu, args);

	if (r == 0) {
		update_watched(mcu);
	}
	return r;
}

int MSIM_M328PTickPerf(struct MSIM_AVR *mcu)
{
	tick_timer0(mcu);

	/* Update watched values after all of the peripherals. */
	update_watched(mcu);

	return 0;
}

static void tick_timer0(struct MSIM_AVR *mcu)
{
	/* 8-bit Timer/Counter0 with PWM */

	static uint32_t tc0_presc;
	static uint32_t tc0_ticks;
	static uint8_t old_wgm0;
	uint8_t cs0;			/* Clock Select bits CS0[2:0] */
	uint8_t wgm0;			/* Waveform Generation  */
	uint8_t com0a;			/* Compare Output mode  */
	uint8_t com0b;			/* Compare Output mode  */
	uint32_t presc;			/* Prescaler value */

	cs0 = DM(TCCR0B)&0x7;

	wgm0 = (uint8_t)((uint8_t)((DM(TCCR0A)>>WGM00)&1) |
	                 (uint8_t)(((DM(TCCR0A)>>WGM01)&1)<<1) |
	                 (uint8_t)(((DM(TCCR0B)>>WGM02)&1)<<2));

	com0a = (uint8_t)((uint8_t)((DM(TCCR0A)>>COM0A0)&1) |
	                  (uint8_t)(((DM(TCCR0A)>>COM0A1)&1)<<1));

	com0b = (uint8_t)((uint8_t)((DM(TCCR0A)>>COM0B0)&1) |
	                  (uint8_t)(((DM(TCCR0A)>>COM0B1)&1)<<1));

	switch (cs0) {
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
	case 0x6:			/* Ext. clock on T0(PD4) (fall) */
		if (IS_FALL(init_portd, DM(PORTD), PD4) ||
		                IS_FALL(init_pind, DM(PIND), PD4)) {
			if (DM(TCNT0) == 0xFF) {
				/* Reset Timer/Counter0 */
				DM(TCNT0) = 0;
				/* Timer/Counter0 overflow occured */
				DM(TIFR0) |= (1<<TOV0);
			} else {	/* Count UP on tick */
				DM(TCNT0)++;
			}
		}
		tc0_presc = 0;
		tc0_ticks = 0;
		return;
	case 0x7:			/* Ext. clock on T0(PD4) (rise) */
		if (IS_RISE(init_portd, DM(PORTD), PD4) ||
		                IS_RISE(init_pind, DM(PIND), PD4)) {
			if (DM(TCNT0) == 0xFF) {
				/* Reset Timer/Counter0 */
				DM(TCNT0) = 0;
				/* Timer/Counter0 overflow occured */
				DM(TIFR0) |= (1<<TOV0);
			} else {	/* Count UP on tick */
				DM(TCNT0)++;
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
		if ((tc0_presc == 0) && (DM(TCNT0) > DM(OCR0A))) {
			missed_cm = 1;
		}
		tc0_presc = presc;
		tc0_ticks = 0;
	}

	switch (wgm0) {
	case 0:
		timer0_normal(mcu, tc0_presc, &tc0_ticks, wgm0, com0a, com0b);
		break;
	case 1:
		/* timer0 pcpwm mode */
		break;
	case 2:
		timer0_ctc(mcu, tc0_presc, &tc0_ticks, wgm0, com0a, com0b);
		break;
	case 3:
		/* timer0 fastpwm mode */
		break;
	default:
		if (wgm0 != old_wgm0) {
			snprintf(mcu->log, sizeof mcu->log, "selected mode "
			         "WGM21:20 = %u of the Timer/Counter2 is not "
			         "supported", wgm0);
			MSIM_LOG_WARN(mcu->log);
			old_wgm0 = wgm0;
		}
		tc0_presc = 0;
		tc0_ticks = 0;
		break;
	}
}

static void timer0_normal(struct MSIM_AVR *mcu,
                          uint32_t presc, uint32_t *ticks,
                          uint8_t wgm0, uint8_t com0a, uint8_t com0b)
{
	if ((*ticks) < (presc-1U)) {
		(*ticks)++;
	} else if ((*ticks) > (presc-1U)) {
		snprintf(mcu->log, sizeof mcu->log, "number of Timer1 "
		         "ticks=%" PRIu32 " should be <= (prescaler-1)=%"
		         PRIu32 "; timer1 will not be updated!",
		         *ticks, (presc-1U));
		MSIM_LOG_ERROR(mcu->log);
	} else {
		if (DM(TCNT0) == TC0_TOP) {
			/* Reset TimerCounter0  */
			DM(TCNT0) = TC0_BOTTOM;
			/* Set TimerCounter0 overflow flag  */
			DM(TIFR0) |= DM(1<<TOV0);
		} else if (DM(TCNT0) == DM(OCR0A)) {
			/* Set TC0 Output Compare A Flag  */
			DM(TIFR0) |= (DM(1<<OCF0A));
			/* Manipulate on the OCR0A (PD6) pin  */
			timer0_oc0_nonpwm(mcu, com0a, com0b, A_CHAN);
			DM(TCNT0)++;

		} else if (DM(TCNT0) == DM(OCR0B)) {
			/* Set TC0 Output Compare B Flag */
			DM(TIFR0) |= (DM(1<<OCF0B));
			/* Manipulate on the OCR0A (PD5) pin  */
			timer0_oc0_nonpwm(mcu, com0a, com0b, B_CHAN);
			DM(TCNT0)++;

		} else {
			DM(TCNT0)++;
		}
		*ticks = 0;
	}
}

static void timer0_ctc(struct MSIM_AVR *mcu,
                       uint32_t presc, uint32_t *ticks,
                       uint8_t wgm0, uint8_t com0a, uint8_t com0b)
{
	if ((*ticks) < (presc-1U)) {
		(*ticks)++;
	} else if ((*ticks) > (presc-1U)) {
		snprintf(mcu->log, sizeof mcu->log, "number of Timer1 "
		         "ticks=%" PRIu32 " should be <= (prescaler-1)=%"
		         PRIu32 "; timer1 will not be updated!",
		         *ticks, (presc-1U));
		MSIM_LOG_ERROR(mcu->log);
	} else {
		/* Max Timer/Counter value or value defined by user */
		if ((DM(TCNT0) == TC0_TOP) || (DM(TCNT0) == DM(OCR0A))) {
			/* Generate TOV flag always and only on 0xFF */
			if (DM(TCNT0) == TC0_TOP) {
				/* Set TimerCounter0 overflow flag  */
				DM(TIFR0) |= DM(1<<TOV0);
			}
			/* Generate Interrupt on TOP value */
			if (DM(TCNT0) == DM(OCR0A)) {
				/* Set TC0 Output Compare A Flag */
				DM(TIFR0) |= (1<<OCF0A);
				/* Manipulate on the OCR0A (PD6) pin  */
				timer0_oc0_nonpwm(mcu, com0a, com0b, A_CHAN);
			}
			if (DM(TCNT0) == DM(OCR0B)) {
				/* Set TC0 Output Compare B Flag */
				DM(TIFR0) |= (1<<OCF0B);
				/* Manipulate on the OCR0B (PD5) pin  */
				timer0_oc0_nonpwm(mcu, com0a, com0b, B_CHAN);
			}
			/* Reset TimerCounter0  */
			DM(TCNT0) = TC0_BOTTOM;
		} else {
			DM(TCNT0)++;
		}
		*ticks = 0;
	}
}


static void timer0_oc0_nonpwm(struct MSIM_AVR *mcu, uint8_t com0a,
                              uint8_t com0b, uint8_t chan)
{
	uint8_t pin, com;

	/* 	 Check Data Direction Register first. DDRB1 or DDRB2 should
		be set to enable the output driver (according to a datasheet).  */
	if (chan == (uint8_t)A_CHAN) {
		pin = PD6;
		com = com0a;
	} else if (chan == (uint8_t)B_CHAN) {
		pin = PD5;
		com = com0b;
	} else {
		MSIM_LOG_ERROR("unsupported channel of Output Compare unit is"
		               "used in timer0; It looks like a bug (please"
		               "report it at trac.mcusim.org)");
		com = NOT_CONNECTED;
	}

	/* Note that the Data Direction Register (DDR)
	   bit corresponding to the OCR0x pin must be set in
	   order to enable the output driver.  */
	if ((com != NOT_CONNECTED) && (!IS_SET(DM(DDRD), pin))) {
		com = NOT_CONNECTED;
	}

	/* Update Output Compare pin (OC0A or OC0B) */
	if (com != NOT_CONNECTED) {
		switch (com) {
		case 1:  /* Toggle pin  */
			if (IS_SET(DM(PORTD), pin) == 1) {
				CLEAR(DM(PORTD), pin);
			} else {
				SET(DM(PORTD), pin);
			}
			break;
		case 2:
			CLEAR(DM(PORTD), pin);
			break;
		case 3:
			SET(DM(PORTD), pin);
			break;
		case 0:
		default:
			/* OC0A/OC0B disconnected, do nothing  */
			break;
		}
	}
}

static void update_watched(struct MSIM_AVR *mcu)
{
	init_portd = DM(PORTD);
	init_pind = DM(PIND);
}

int MSIM_M328PSetFuse(struct MSIM_AVR *mcu, uint32_t fuse_n, uint8_t fuse_v)
{
	uint8_t cksel, bootsz;
	uint8_t err;

	err = 0;
	if (fuse_n > 2U) {
		snprintf(mcu->log, sizeof mcu->log, "fuse #%u is not "
		         "supported by %s", fuse_n, mcu->name);
		MSIM_LOG_ERROR(mcu->log);
		err = 1;
	}

	if (err == 0U) {
		mcu->fuse[fuse_n] = fuse_v;

		switch (fuse_n) {
		case FUSE_LOW:
			cksel = fuse_v&0xFU;

			if (cksel == 0U) {
				mcu->clk_source = AVR_EXT_CLK;
			} else if (cksel == 1U) {
				snprintf(mcu->log, sizeof mcu->log,
				         "CKSEL = %" PRIu8 ", is  "
				         "reserved on ", cksel);
				MSIM_LOG_ERROR(mcu->log);
				err = 1;
				break;
			} else if (cksel == 2U) {
				mcu->clk_source = AVR_INT_CAL_RC_CLK;
				/* max 8 MHz */
				mcu->freq = 8000000;
			} else if (cksel == 3U) {
				mcu->clk_source = AVR_INT_128K_RC_CLK;
				/* max 128 kHz */
				mcu->freq = 128000;
			}  else if ((cksel == 4U) || (cksel == 5U)) {
				mcu->clk_source = AVR_EXT_LOWF_CRYSTAL_CLK;
				switch (cksel) {
				case 4:
					/* max 1 MHz */
					mcu->freq = 1000000;
					break;
				case 5:
					/* max 32,768 kHz */
					mcu->freq = 32768;
					break;
				default:
					/* Should not happen! */
					snprintf(mcu->log, sizeof mcu->log,
					         "CKSEL = %" PRIu8 ", but it "
					         "should be within [4,5] "
					         "inclusively", cksel);
					MSIM_LOG_ERROR(mcu->log);
					err = 1;
					break;
				}
			} else if ((cksel == 6U) || (cksel == 7U)) {
				mcu->clk_source = AVR_FULLSWING_CRYSTAL_CLK;
				mcu->freq = 20000000; /* max 20 MHz */
			} else if ((cksel >= 8U) && (cksel <= 15U)) {
				mcu->clk_source = AVR_LOWP_CRYSTAL_CLK;

				/* CKSEL0 can be used to adjust start-up time
				 * and additional delay from MCU reset. */

				/* CKSEL3:1 sets frequency range */
				cksel = cksel&0xEU;
				switch (cksel) {
				case 8:
					mcu->freq = 900000; /* max 0.9MHz */
					break;
				case 10:
					mcu->freq = 3000000; /* max 3MHz */
					break;
				case 12:
					mcu->freq = 8000000; /* max 8MHz */
					break;
				case 14:
					mcu->freq = 16000000; /* max 16MHz */
					break;
				default:
					/* Should not happen! */
					snprintf(mcu->log, sizeof mcu->log,
					         "CKSEL = %" PRIu8 ", but it "
					         "should be 8, 11, 13 or 14"
					         "to select a correct frequency"
					         "range", cksel);
					MSIM_LOG_ERROR(mcu->log);
					err = 1;
					break;
				}
			} else {
				/* Should not happen! */
			}
			break;
		case FUSE_HIGH:
			bootsz = (fuse_v>>1)&0x3U;

			switch (bootsz) {
			case 3:
				mcu->bls.start = 0x7E00; /* first byte */
				mcu->bls.end = 0x7FFF; /* last byte */
				mcu->bls.size = 512; /* bytes! */
				break;
			case 2:
				mcu->bls.start = 0x7C00;
				mcu->bls.end = 0x7FFF;
				mcu->bls.size = 1024; /* bytes */
				break;
			case 1:
				mcu->bls.start = 0x7800;
				mcu->bls.end = 0x7FFF;
				mcu->bls.size = 2048; /* bytes */
				break;
			case 0:
				mcu->bls.start = 0x7000;
				mcu->bls.end = 0x7FFF;
				mcu->bls.size = 4096; /* bytes */
				break;
			default:
				/* Should not happen! */
				snprintf(mcu->log, sizeof mcu->log,
				         "BOOTSZ1:0 = %" PRIu8 ", but it "
				         "should be in [0,3] "
				         "inclusively", bootsz);
				MSIM_LOG_ERROR(mcu->log);
				err = 1;
				break;
			}

			if ((fuse_v&1U) == 1U) {
				mcu->intr.reset_pc = 0x0000;
				mcu->pc = 0x0000;
			} else {
				mcu->intr.reset_pc = mcu->bls.start;
				mcu->pc = mcu->bls.start;
			}

			break;
		case FUSE_EXT:
			break;
		default:
			/* Should not happen */
			err = 1;
			break;
		}
	}

	return err;
}

int MSIM_M328PSetLock(struct MSIM_AVR *mcu, uint8_t lock_v)
{
	/* It's waiting to be implemented. */
	return 0;
}

