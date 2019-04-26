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
#ifndef MSIM_AVR_M8A_H_
#define MSIM_AVR_M8A_H_ 1

/* Include headers specific to the ATMega8A */
#define _SFR_ASM_COMPAT 1
#define __AVR_ATmega8A__ 1

#include <stdio.h>
#include <stdint.h>

#include "mcusim/avr/io.h"
#include "mcusim/avr/sim/m8/m8_ioregs.h"
#include "mcusim/avr/sim/io_regs.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"
#include "mcusim/avr/sim/timer.h"
#include "mcusim/avr/sim/private/io_macro.h"

#define MCU_NAME	"ATmega8A"
#define RESET_PC	0x0000	/* Reset vector address (in bytes) */
#define IVT_ADDR	0x0002	/* Interrupt vectors address (in bytes) */
#define PC_BITS		12	/* PC bit capacity */
#define LBITS_DEFAULT	0x3F	/* Default lock bits */
#define CLK_SOURCE	AVR_INT_CAL_RC_CLK /* Calibrated Internal RC */
#define CLK_FREQ	1000000	/* Oscillator frequency, in Hz */
#define GP_REGS		32	/* # of GP registers */
#define IO_REGS		64	/* # of I/O registers */
#define BLS_START	0x1800	/* First address in BLS, in bytes */
#define BLS_END		0x1FFF	/* Last address in BLS, in bytes */
#define BLS_SIZE	2048	/* BLS size, in bytes */

/* Missing pins definitions */
#define ICP1		0
#define OC1A		1
#define OC1B		2
#define OC2		3

#define TICK_PERF_F	MSIM_M8AUpdate
#define SET_FUSE_F	MSIM_M8ASetFuse
#define SET_LOCK_F	MSIM_M8ASetLock
#define PASS_IRQS_F	MSIM_M8APassIRQs
#define RESET_SPM_F	MSIM_M8AResetSPM

int MSIM_M8AUpdate(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);
int MSIM_M8ASetFuse(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);
int MSIM_M8ASetLock(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);
int MSIM_M8APassIRQs(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);
int MSIM_M8AResetSPM(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);

const struct MSIM_AVR ORIG_MCU = {
	.timers = {
		[0] = {
			/* ---------------- Basic config ------------------- */
			.tcnt = { IOBYTE(TCNT0) },
			.disabled = IONOBIT(),
			/* ------------- Clock select config --------------- */
			.cs = {
				IOBIT(TCCR0, CS00), IOBIT(TCCR0, CS01),
				IOBIT(TCCR0, CS02)
			},
			.cs_div = { 0, 0, 3, 6, 8, 10 }, /* Power of 2 */
			/* ------- Waveform generation mode config --------- */
			.wgm = IONOBITA(),
			.wgm_op = NOWGMA(),
			/* ------------ Input capture config --------------- */
			.icr = IONOBITA(),
			.icp = IONOBIT(),
			.ices = IONOBITA(),
			/* ------------- Interrupts config ----------------- */
			.iv_ovf = {
				.enable = IOBIT(TIMSK, TOIE0),
				.raised = IOBIT(TIFR, TOV0),
				.vector = TIMER0_OVF_vect_num
			},
			.iv_ic = NOINTV(),
			/* ----------- Output compare config --------------- */
			.comp = NOCOMPA(),
		},
		[1] = {
			/* ---------------- Basic config ------------------- */
			.tcnt = { IOBYTE(TCNT1L), IOBYTE(TCNT1H) },
			.disabled = IONOBIT(),
			/* ------------- Clock select config --------------- */
			.cs = {
				IOBIT(TCCR1B, CS10), IOBIT(TCCR1B, CS11),
				IOBIT(TCCR1B, CS12)
			},
			.cs_div = { 0, 0, 3, 6, 8, 10 }, /* Power of 2 */
			/* ------- Waveform generation mode config --------- */
			.wgm = {
				IOBIT(TCCR1A, WGM10), IOBIT(TCCR1A, WGM11),
				IOBIT(TCCR1B, WGM12), IOBIT(TCCR1B, WGM13)
			},
			.wgm_op = {
				[0] = {
					.kind = WGM_NORMAL,
					.size = 16,
					.top = 0xFFFF,
					.updocr_at = UPD_ATIMMEDIATE,
					.settov_at = UPD_ATMAX,
				},
				[1] = {
					.kind = WGM_PCPWM8,
					.size = 8,
					.top = 0x00FF,
					.updocr_at = UPD_ATTOP,
					.settov_at = UPD_ATBOTTOM,
				},
				[2] = {
					.kind = WGM_PCPWM9,
					.size = 9,
					.top = 0x01FF,
					.updocr_at = UPD_ATTOP,
					.settov_at = UPD_ATBOTTOM,
				},
				[3] = {
					.kind = WGM_PCPWM10,
					.size = 10,
					.top = 0x03FF,
					.updocr_at = UPD_ATTOP,
					.settov_at = UPD_ATBOTTOM,
				},
				[4] = {
					.kind = WGM_CTC,
					.rtop = {
						IOBYTE(OCR1AL), IOBYTE(OCR1AH)
					},
					.updocr_at = UPD_ATIMMEDIATE,
					.settov_at = UPD_ATMAX,
				},
				[5] = {
					.kind = WGM_FASTPWM8,
					.size = 8,
					.top = 0x00FF,
					.updocr_at = UPD_ATBOTTOM,
					.settov_at = UPD_ATTOP,
				},
				[6] = {
					.kind = WGM_FASTPWM9,
					.size = 9,
					.top = 0x01FF,
					.updocr_at = UPD_ATBOTTOM,
					.settov_at = UPD_ATTOP,
				},
				[7] = {
					.kind = WGM_FASTPWM10,
					.size = 10,
					.top = 0x01FF,
					.updocr_at = UPD_ATBOTTOM,
					.settov_at = UPD_ATTOP,
				},
				[8] = {
					.kind = WGM_PFCPWM,
					.rtop = {
						IOBYTE(ICR1L), IOBYTE(ICR1H)
					},
					.updocr_at = UPD_ATBOTTOM,
					.settov_at = UPD_ATBOTTOM,
				},
				[9] = {
					.kind = WGM_PFCPWM,
					.rtop = {
						IOBYTE(OCR1AL), IOBYTE(OCR1AH)
					},
					.updocr_at = UPD_ATBOTTOM,
					.settov_at = UPD_ATBOTTOM,
				},
				[10] = {
					.kind = WGM_PCPWM,
					.rtop = {
						IOBYTE(ICR1L), IOBYTE(ICR1H)
					},
					.updocr_at = UPD_ATTOP,
					.settov_at = UPD_ATBOTTOM,
				},
				[11] = {
					.kind = WGM_PCPWM,
					.rtop = {
						IOBYTE(OCR1AL), IOBYTE(OCR1AH)
					},
					.updocr_at = UPD_ATTOP,
					.settov_at = UPD_ATBOTTOM,
				},
				[12] = {
					.kind = WGM_CTC,
					.rtop = {
						IOBYTE(ICR1L), IOBYTE(ICR1H)
					},
					.updocr_at = UPD_ATIMMEDIATE,
					.settov_at = UPD_ATMAX,
				},
				[13] = {
					.kind = WGM_NONE,
				},
				[14] = {
					.kind = WGM_FASTPWM,
					.rtop = {
						IOBYTE(ICR1L), IOBYTE(ICR1H)
					},
					.updocr_at = UPD_ATBOTTOM,
					.settov_at = UPD_ATTOP,
				},
				[15] = {
					.kind = WGM_FASTPWM,
					.rtop = {
						IOBYTE(OCR1AL), IOBYTE(OCR1AH)
					},
					.updocr_at = UPD_ATBOTTOM,
					.settov_at = UPD_ATTOP,
				},
			},
			/* ------------ Input capture config --------------- */
			.icr = { IOBYTE(ICR1L), IOBYTE(ICR1H) },
			.icp = IOBIT(PORTB, ICP1),
			.ices = { IOBIT(TCCR1B, ICES1) },
			/* ------------- Interrupts config ----------------- */
			.iv_ovf = {
				.enable = IOBIT(TIMSK, TOIE1),
				.raised = IOBIT(TIFR, TOV1),
				.vector = TIMER1_OVF_vect_num
			},
			.iv_ic = {
				.enable = IOBIT(TIMSK, TICIE1),
				.raised = IOBIT(TIFR, ICF1),
				.vector = TIMER1_CAPT_vect_num
			},
			/* ----------- Output compare config --------------- */
			.comp = {
				[0] = {
					.ocr = {
						IOBYTE(OCR1AL), IOBYTE(OCR1AH)
					},
					.pin = IOBIT(PORTB, OC1A),
					.ddp = IOBIT(DDRB, OC1A),
					.com = IOBITS(TCCR1A, COM1A0, 0x3),
					.iv = {
						.enable = IOBIT(TIMSK, OCIE1A),
						.raised = IOBIT(TIFR, OCF1A),
						.vector = TIMER1_COMPA_vect_num
					},
				},
				[1] = {
					.ocr = {
						IOBYTE(OCR1BL), IOBYTE(OCR1BH)
					},
					.pin = IOBIT(PORTB, OC1B),
					.ddp = IOBIT(DDRB, OC1B),
					.com = IOBITS(TCCR1A, COM1B0, 0x3),
					.iv = {
						.enable = IOBIT(TIMSK, OCIE1B),
						.raised = IOBIT(TIFR, OCF1B),
						.vector = TIMER1_COMPB_vect_num
					},
				},
			},
		},
		[2] = {
			/* ---------------- Basic config ------------------- */
			.tcnt = { IOBYTE(TCNT2) },
			.disabled = IONOBIT(),
			/* ------------- Clock select config --------------- */
			.cs = {
				IOBIT(TCCR1B, CS10), IOBIT(TCCR1B, CS11),
				IOBIT(TCCR1B, CS12)
			},
			.cs_div = { 0, 0, 3, 5, 6, 7, 8, 10 }, /* Power of 2 */
			/* ------- Waveform generation mode config --------- */
			.wgm = { IOBIT(TCCR2, WGM20), IOBIT(TCCR2, WGM21) },
			.wgm_op = {
				[0] = {
					.kind = WGM_NORMAL,
					.size = 8,
					.top = 0xFF,
					.updocr_at = UPD_ATIMMEDIATE,
					.settov_at = UPD_ATMAX,
				},
				[1] = {
					.kind = WGM_PCPWM,
					.size = 8,
					.top = 0xFF,
					.updocr_at = UPD_ATTOP,
					.settov_at = UPD_ATBOTTOM,
				},
				[2] = {
					.kind = WGM_CTC,
					.rtop = { IOBYTE(OCR2) },
					.updocr_at = UPD_ATIMMEDIATE,
					.settov_at = UPD_ATMAX,
				},
				[3] = {
					.kind = WGM_FASTPWM,
					.size = 8,
					.top = 0xFF,
					.updocr_at = UPD_ATBOTTOM,
					.settov_at = UPD_ATMAX,
				},
			},
			/* ------------ Input capture config --------------- */
			.icr = IONOBITA(),
			.icp = IONOBIT(),
			.ices = IONOBITA(),
			/* ------------- Interrupts config ----------------- */
			.iv_ovf = {
				.enable = IOBIT(TIMSK, TOIE2),
				.raised = IOBIT(TIFR, TOV2),
				.vector = TIMER2_OVF_vect_num
			},
			.iv_ic = NOINTV(),
			/* ----------- Output compare config --------------- */
			.comp = {
				[0] = {
					.ocr = { IOBYTE(OCR2) },
					.pin = IOBIT(PORTB, OC2),
					.ddp = IOBIT(DDRB, OC2),
					.com = IOBITS(TCCR2, COM20, 0x3),
					.iv = {
						.enable = IOBIT(TIMSK, OCIE2),
						.raised = IOBIT(TIFR, OCF2),
						.vector = TIMER2_COMP_vect_num
					},
				},
			},
		},
	}
};

#endif /* MSIM_AVR_M8A_H_ */

