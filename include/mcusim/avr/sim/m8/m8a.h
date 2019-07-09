/*
 * This file is part of MCUSim, an XSPICE library with microcontrollers.
 *
 * Copyright (C) 2017-2019 MCUSim Developers, see AUTHORS.txt for contributors.
 *
 * MCUSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MCUSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

int
MSIM_M8AUpdate(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);
int
MSIM_M8ASetFuse(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);
int
MSIM_M8ASetLock(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);
int
MSIM_M8AResetSPM(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);

const static struct MSIM_AVR ORIG_M8A = {
	.name = "ATmega8A",
	.signature = { SIGNATURE_0, SIGNATURE_1, SIGNATURE_2 },
	.pc = 0x0000,
	.pc_bits = 12,
	.freq = 1000000,
	.clk_source = AVR_INT_CAL_RC_CLK,
	.lockbits = 0x3F,
	.regs_num = 32,
	.ioregs_num = 64,
	.xmega = 0,
	.reduced_core = 0,
	.spm_pagesize = SPM_PAGESIZE,
	.flashstart = FLASHSTART,
	.flashend = FLASHEND,
	.ramstart = RAMSTART,
	.ramend = RAMEND,
	.ramsize = RAMSIZE,
	.e2start = E2START,
	.e2end = E2END,
	.e2size = E2SIZE,
	.e2pagesize = E2PAGESIZE,
	.sfr_off = __SFR_OFFSET,
	.set_fusef = MSIM_M8ASetFuse,
	.set_lockf = MSIM_M8ASetLock,
	.tick_perf = MSIM_M8AUpdate,
	.reset_spm = MSIM_M8AResetSPM,
	.fuse = { LFUSE_DEFAULT, HFUSE_DEFAULT, 0xFF },
	.bls = {
		.start = 0x1800,
		.end = 0x1FFF,
		.size = 2048,
	},
	.intr = {
		.reset_pc = 0x0000,
		.ivt = 0x0002,
	},
	.timers = {
		[0] = {
			/* ---------------- Basic config ------------------- */
			.tcnt = { IOBYTE(TCNT0) },
			.disabled = IONOBIT(),
			.size = 8,
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
			.size = 16,
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
					.kind = WGM_PCPWM,
					.size = 8,
					.top = 0x00FF,
					.updocr_at = UPD_ATTOP,
					.settov_at = UPD_ATBOTTOM,
				},
				[2] = {
					.kind = WGM_PCPWM,
					.size = 9,
					.top = 0x01FF,
					.updocr_at = UPD_ATTOP,
					.settov_at = UPD_ATBOTTOM,
				},
				[3] = {
					.kind = WGM_PCPWM,
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
					.kind = WGM_FASTPWM,
					.size = 8,
					.top = 0x00FF,
					.updocr_at = UPD_ATBOTTOM,
					.settov_at = UPD_ATTOP,
				},
				[6] = {
					.kind = WGM_FASTPWM,
					.size = 9,
					.top = 0x01FF,
					.updocr_at = UPD_ATBOTTOM,
					.settov_at = UPD_ATTOP,
				},
				[7] = {
					.kind = WGM_FASTPWM,
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
			.icp = IOBIT(PORTB, PB0),
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
					.pin = IOBIT(PORTB, PB1),
					.ddp = IOBIT(DDRB, PB1),
					.com = IOBITS(TCCR1A, COM1A0, 0x3, 2),
					.com_op = {
						[0] = {
							COM_DISC,
							COM_TGONCM,
							COM_CLONCM,
							COM_STONCM,
						},
						[1] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[2] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[3] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[4] = {
							COM_DISC,
							COM_TGONCM,
							COM_CLONCM,
							COM_STONCM,
						},
						[5] = {
							COM_DISC,
							COM_DISC,
							COM_CLONCM_STATBOT,
							COM_STONCM_CLATBOT,
						},
						[6] = {
							COM_DISC,
							COM_DISC,
							COM_CLONCM_STATBOT,
							COM_STONCM_CLATBOT,
						},
						[7] = {
							COM_DISC,
							COM_DISC,
							COM_CLONCM_STATBOT,
							COM_STONCM_CLATBOT,
						},
						[8] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[9] = {
							COM_DISC,
							COM_TGONCM,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[10] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[11] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[12] = {
							COM_DISC,
							COM_TGONCM,
							COM_CLONCM,
							COM_STONCM,
						},
						[14] = {
							COM_DISC,
							COM_DISC,
							COM_CLONCM_STATBOT,
							COM_STONCM_CLATBOT,
						},
						[15] = {
							COM_DISC,
							COM_TGONCM,
							COM_CLONCM_STATBOT,
							COM_STONCM_CLATBOT,
						},
					},
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
					.pin = IOBIT(PORTB, PB2),
					.ddp = IOBIT(DDRB, PB2),
					.com = IOBITS(TCCR1A, COM1B0, 0x3, 2),
					.com_op = {
						[0] = {
							COM_DISC,
							COM_TGONCM,
							COM_CLONCM,
							COM_STONCM,
						},
						[1] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[2] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[3] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[4] = {
							COM_DISC,
							COM_TGONCM,
							COM_CLONCM,
							COM_STONCM,
						},
						[5] = {
							COM_DISC,
							COM_DISC,
							COM_CLONCM_STATBOT,
							COM_STONCM_CLATBOT,
						},
						[6] = {
							COM_DISC,
							COM_DISC,
							COM_CLONCM_STATBOT,
							COM_STONCM_CLATBOT,
						},
						[7] = {
							COM_DISC,
							COM_DISC,
							COM_CLONCM_STATBOT,
							COM_STONCM_CLATBOT,
						},
						[8] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[9] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[10] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[11] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[12] = {
							COM_DISC,
							COM_TGONCM,
							COM_CLONCM,
							COM_STONCM,
						},
						[14] = {
							COM_DISC,
							COM_DISC,
							COM_CLONCM_STATBOT,
							COM_STONCM_CLATBOT,
						},
						[15] = {
							COM_DISC,
							COM_DISC,
							COM_CLONCM_STATBOT,
							COM_STONCM_CLATBOT,
						},
					},
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
			.size = 8,
			/* ------------- Clock select config --------------- */
			.cs = {
				IOBIT(TCCR2, CS20), IOBIT(TCCR2, CS21),
				IOBIT(TCCR2, CS22)
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
					.pin = IOBIT(PORTB, PB3),
					.ddp = IOBIT(DDRB, PB3),
					.com = IOBITS(TCCR2, COM20, 0x3, 2),
					.com_op = {
						[0] = {
							COM_DISC,
							COM_TGONCM,
							COM_CLONCM,
							COM_STONCM,
						},
						[1] = {
							COM_DISC,
							COM_DISC,
							COM_CLONUP_STONDOWN,
							COM_STONUP_CLONDOWN,
						},
						[2] = {
							COM_DISC,
							COM_TGONCM,
							COM_CLONCM,
							COM_STONCM,
						},
						[3] = {
							COM_DISC,
							COM_DISC,
							COM_CLONCM_STATBOT,
							COM_STONCM_CLATBOT,
						},
					},
					.iv = {
						.enable = IOBIT(TIMSK, OCIE2),
						.raised = IOBIT(TIFR, OCF2),
						.vector = TIMER2_COMP_vect_num
					},
				},
			},
		},
	},
	.wdt = {
		.wdton = FBIT(1, WDTON),
		.wde = IOBIT(WDTCR, WDE),
		.wdie = IONOBIT(),
		.ce = IOBIT(WDTCR, WDCE),
		.oscf = 1000000,
		.wdp = {
			IOBIT(WDTCR, WDP0), IOBIT(WDTCR, WDP1),
			IOBIT(WDTCR, WDP2)
		},
		.wdp_op = { 16, 32, 64, 128, 256, 512, 1024, 2048 },
		.iv_tout = NOINTV(),
		.iv_sysr = {
			.enable = IONOBIT(),
			.raised = IONOBIT(),
			.vector = 0 /* Reset vector */
		},
	}
};

#endif /* MSIM_AVR_M8A_H_ */
