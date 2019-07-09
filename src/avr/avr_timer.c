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

/* A model-independent AVR timer. */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "mcusim/mcusim.h"
#include "mcusim/avr/sim/timer.h"
#include "mcusim/avr/sim/private/macro.h"
#include "mcusim/avr/sim/private/io_macro.h"
#include "mcusim/bit/private/macro.h"

#define FAKE_WGM8 {							\
	.kind = WGM_NORMAL,						\
	.size = 8,							\
	.top = 0xFF,							\
	.updocr_at = UPD_ATIMMEDIATE,					\
	.settov_at = UPD_ATBOTTOM,					\
};

#define FAKE_WGM16 {							\
	.kind = WGM_NORMAL,						\
	.size = 16,							\
	.top = 0xFFFF,							\
	.updocr_at = UPD_ATIMMEDIATE,					\
	.settov_at = UPD_ATBOTTOM,					\
};

static int	update_timer(MSIM_AVR *, MSIM_AVR_TMR *);
static void	mode_nonpwm_pwm(MSIM_AVR *, MSIM_AVR_TMR *);
static void	update_ocr_buffers(MSIM_AVR *, MSIM_AVR_TMR *);
static int	update_ocr_buffer(MSIM_AVR *, MSIM_AVR_TMR *, uint32_t);

static void	int_reset_pending(MSIM_AVR *, MSIM_AVR_TMR *);
static void	int_raise_pending(MSIM_AVR *, MSIM_AVR_TMR *);
static void	trigger_oc_pin(MSIM_AVR *, MSIM_AVR_TMR *, MSIM_AVR_TMR_COMP *,
                               uint32_t, uint32_t, uint8_t);

static void	update_wgm_buffers(MSIM_AVR *, MSIM_AVR_TMR *);
static int	update_wgm_buffer(MSIM_AVR *, MSIM_AVR_TMR *, uint32_t);
static void	update_icp_value(MSIM_AVR *, MSIM_AVR_TMR *);

int
MSIM_AVR_TMRUpdate(struct MSIM_AVR *mcu)
{
	int rc = 0;

	for (uint32_t i = 0; i < MSIM_AVR_MAXTMRS; i++) {
		MSIM_AVR_TMR *tmr = &mcu->timers[i];

		if (IS_IONOBITA(tmr->tcnt)) {
			break;
		}

		rc = update_timer(mcu, tmr);
		if (rc != 0) {
			break;
		}
	}

	return rc;
}

static int
update_timer(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr)
{
	struct MSIM_AVR_TMR_WGM wgm8 = FAKE_WGM8;
	struct MSIM_AVR_TMR_WGM wgm16 = FAKE_WGM16;
	uint32_t wgm, dis;
	int rc = 0;

	do {
		/* Timer can be undefined... */
		if (IS_IONOBITA(tmr->cs)) {
			tmr->scnt = 0;
			tmr->presc = 1;
			break;
		}
		/* ... or disabled by firmware */
		if (!IS_IONOBYTE(tmr->disabled)) {
			dis = IOBIT_RD(mcu, &tmr->disabled);
			if (dis != 0U) {
				tmr->scnt = 0;
				tmr->presc = 1;
				update_ocr_buffers(mcu, tmr);
				update_wgm_buffers(mcu, tmr);
				int_reset_pending(mcu, tmr);
				break;
			}
		}

		/* Obtain timer's Clock Source */
		uint32_t cs = IOBIT_RDA(mcu, tmr->cs, ARRSZ(tmr->cs));
		if (cs >= ARRSZ(tmr->cs_div)) {
			tmr->scnt = 0;
			break;
		}
		if (cs == 0U) {
			tmr->scnt = 0;
			tmr->presc = 1;
			update_ocr_buffers(mcu, tmr);
			update_wgm_buffers(mcu, tmr);
			int_reset_pending(mcu, tmr);
			break;
		} else {
			tmr->presc = 1<<(tmr->cs_div[cs]);
		}

		/* Obtain timer's Waveform Generation Mode */
		if (IS_IONOBITA(tmr->wgm)) {
			tmr->wgmval = (tmr->size == 16) ? &wgm16 : &wgm8;
			tmr->wgmi = -1;
		} else {
			wgm = IOBIT_RDA(mcu, tmr->wgm, ARRSZ(tmr->wgm));
			tmr->wgmval = &tmr->wgm_op[wgm];
			tmr->wgmi = (int32_t)wgm;
		}

		/* Clock source changed */
		if (tmr->presc != (1<<(tmr->cs_div[cs]))) {
			tmr->presc = 1<<(tmr->cs_div[cs]);
		}

		switch (tmr->wgmval->kind) {
		case WGM_NORMAL:
		case WGM_CTC:
		case WGM_FASTPWM:
		case WGM_PCPWM:
		case WGM_PFCPWM:
			mode_nonpwm_pwm(mcu, tmr);
			break;
		default:
			break;
		}
	} while (0);

	/* "Old" value of the Input Capture pin should be updated anyway. */
	update_icp_value(mcu, tmr);

	return rc;
}

static void
mode_nonpwm_pwm(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr)
{
	struct MSIM_AVR_TMR_WGM *wgm = tmr->wgmval;
	struct MSIM_AVR_TMR_COMP *comp;
	uint32_t tcnt = IOBIT_RDA(mcu, tmr->tcnt, ARRSZ(tmr->tcnt));
	uint32_t ocr, top = wgm->top;
	uint32_t icp, ices;
	uint8_t dual_slope = 0;
	uint8_t cd = tmr->cnt_dir;

	/* Does timer use a dual-slope operation mode?
	 * Counting direction matters in this case. */
	dual_slope = ((tmr->wgmval->kind == WGM_PCPWM) ||
	              (tmr->wgmval->kind == WGM_PFCPWM)) ? 1 : 0;

	/* Raise pending interrupts */
	int_raise_pending(mcu, tmr);

	/* Obtain TOP value from a register */
	if (!IS_IONOBITA(wgm->rtop)) {
		top = (wgm->updocr_at == UPD_ATIMMEDIATE)
		      ? IOBIT_RDA(mcu, wgm->rtop, ARRSZ(wgm->rtop))
		      : wgm->rtop_buf;
	}

	/* Input Capture unit watches an ICP (input capture pin) or ACO
	 * (analog comparator output). */
	if (!IS_IONOBIT(tmr->icp)) {
		icp = (uint8_t)IOBIT_RD(mcu, &tmr->icp);
		ices = IOBIT_RDA(mcu, tmr->ices, ARRSZ(tmr->ices));

		if (((ices == 0U) && IS_FALL(tmr->icpval, icp, 0)) ||
		                ((ices == 1U) && IS_RISE(tmr->icpval, icp, 0))) {
			/* Input Capture flag raised */
			IOBIT_WR(mcu, &tmr->iv_ic.raised, 1);

			/* Copy counter value to ICR */
			if ((IOBIT_CMPA(wgm->rtop, tmr->icr,
			                ARRSZ(wgm->rtop)))) {
				IOBIT_WRA(mcu, tmr->icr, ARRSZ(tmr->icr),
				          tcnt);
			}
		}
	}

	if (tmr->scnt < (tmr->presc-1U)) {
		tmr->scnt++;
	} else {
		/* Update buffers at TOP/MAX */
		if ((cd == CNT_UP) && (tcnt == (top-1))) {
			if ((wgm->updocr_at == UPD_ATTOP) ||
			                (wgm->updocr_at == UPD_ATMAX)) {
				update_ocr_buffers(mcu, tmr);
				update_wgm_buffers(mcu, tmr);
			}

			/* Raise TOV flag at TOP or MAX */
			if ((wgm->settov_at == UPD_ATTOP) ||
			                (wgm->settov_at == UPD_ATMAX)) {
				IOBIT_WR(mcu, &tmr->iv_ovf.raised, 1);
			}
		} else {
			; /* Nothing to be done */
		}

		/* Output Compare and Compare Match units.
		 * Compares current timer value with OC registers. */
		for (uint32_t i = 0; i < ARRSZ(tmr->comp); i++) {
			comp = &tmr->comp[i];
			if (IS_NOCOMP(comp)) {
				break;
			}

			ocr = (wgm->updocr_at == UPD_ATIMMEDIATE)
			      ? IOBIT_RDA(mcu, comp->ocr, ARRSZ(comp->ocr))
			      : comp->ocr_buf;

			if (((cd == CNT_UP) && (tcnt == (ocr-1))) ||
			                ((cd == CNT_DOWN) &&
			                 (tcnt == (ocr+1)))) {
				/* Compare match. Interrupt flag should be set
				 * at the next timer clock cycle! */
				comp->iv.pending = 1;

				/* Trigger OC pin at Compare Match */
				trigger_oc_pin(mcu, tmr, comp, tcnt, top,
				               UPD_ATCM);
			}
		}

		/* Update buffers at BOTTOM */
		if (((dual_slope == 0U) && (tcnt == top)) ||
		                ((dual_slope == 1U) &&
		                 (cd == CNT_DOWN) && (tcnt == 1U))) {
			/* Raise TOV flag at BOTTOM */
			if (wgm->settov_at == UPD_ATBOTTOM) {
				IOBIT_WR(mcu, &tmr->iv_ovf.raised, 1);
			}

			/* Update buffers at BOTTOM */
			if (wgm->updocr_at == UPD_ATBOTTOM) {
				update_ocr_buffers(mcu, tmr);
				update_wgm_buffers(mcu, tmr);
			}

			/* Trigger OC pin at BOTTOM */
			for (uint32_t i = 0; i < ARRSZ(tmr->comp); i++) {
				comp = &tmr->comp[i];
				if (IS_NOCOMP(comp)) {
					break;
				}
				trigger_oc_pin(mcu, tmr, comp, tcnt, top,
				               UPD_ATBOTTOM);
			}
		} else {
			; /* Nothing to be done */
		}

		/* Counter Unit. */
		if ((dual_slope == 0U) && (tcnt == top)) {
			tcnt = 0;
		} else if ((dual_slope == 1U) && (tcnt == top)) {
			tmr->cnt_dir = CNT_DOWN;
			tcnt--;
		} else if ((dual_slope == 1U) && (tcnt == 0)) {
			tmr->cnt_dir = CNT_UP;
			tcnt++;
		} else {
			if (tmr->cnt_dir == CNT_UP) {
				tcnt++;
			} else {
				tcnt--;
			}
		}

		/* Reset system clock counter and update timer's register. */
		tmr->scnt = 0;
		IOBIT_WRA(mcu, tmr->tcnt, ARRSZ(tmr->tcnt), tcnt);
	}
}

static void
trigger_oc_pin(struct MSIM_AVR *mcu, MSIM_AVR_TMR *tmr, MSIM_AVR_TMR_COMP *comp,
               uint32_t tcnt, uint32_t top, uint8_t at)
{
	int32_t wgmi = tmr->wgmi;
	uint32_t com = IOBIT_RD(mcu, &comp->com);
	uint32_t ddp = IOBIT_RD(mcu, &comp->ddp); /* Data direction (pin) */
	uint8_t com_op = comp->com_op[wgmi][com]; /* Current COMP operation */

	do {
		if (ddp == 0U) {
			break;
		}

		if (tmr->cnt_dir == CNT_UP) {
			if (at == UPD_ATCM) {
				switch (com_op) {
				case COM_TGONCM:
					IOBIT_TG(mcu, &comp->pin);
					break;
				case COM_CLONCM:
				case COM_CLONCM_STATBOT:
					IOBIT_WR(mcu, &comp->pin, 0);
					break;
				case COM_CLONUP_STONDOWN:
					if ((tcnt != (top-1)) && (tcnt != 1)) {
						IOBIT_WR(mcu, &comp->pin, 0);
					}
					break;
				case COM_STONCM:
				case COM_STONCM_CLATBOT:
					IOBIT_WR(mcu, &comp->pin, 1);
					break;
				case COM_STONUP_CLONDOWN:
					if ((tcnt != (top-1)) && (tcnt != 1)) {
						IOBIT_WR(mcu, &comp->pin, 1);
					}
					break;
				case COM_DISC:
				default:
					break;
				}
			} else if (at == UPD_ATBOTTOM) {
				switch (com_op) {
				case COM_CLONCM_STATBOT:
					IOBIT_WR(mcu, &comp->pin, 1);
					break;
				case COM_STONCM_CLATBOT:
					IOBIT_WR(mcu, &comp->pin, 0);
					break;
				default:
					break;
				}
			} else {
				/* Nothing to be toggled in this case */
			}
		} else if (at == UPD_ATCM) { /* Down counting */
			switch (com_op) {
			case COM_CLONUP_STONDOWN:
				if ((tcnt != (top-1)) && (tcnt != 1)) {
					IOBIT_WR(mcu, &comp->pin, 1);
				}
				break;
			case COM_STONUP_CLONDOWN:
				if ((tcnt != (top-1)) && (tcnt != 1)) {
					IOBIT_WR(mcu, &comp->pin, 0);
				}
				break;
			default:
				break;
			}
		} else {
			/* Nothing to be toggled in this case */
		}
	} while (0);
}

static void
update_ocr_buffers(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr)
{
	for (uint32_t i = 0; i < ARRSZ(tmr->comp); i++) {
		int rc = update_ocr_buffer(mcu, tmr, i);
		if (rc != 0) {
			break;
		}
	}
}

static int
update_ocr_buffer(struct MSIM_AVR *mcu, MSIM_AVR_TMR *tmr, uint32_t i)
{
	struct MSIM_AVR_TMR_COMP *comp = &tmr->comp[i];
	int rc = 0;

	if (IS_NOCOMP(comp)) {
		rc = 1;
	} else {
		comp->ocr_buf = IOBIT_RDA(mcu, comp->ocr, ARRSZ(comp->ocr));
	}

	return rc;
}

static void
update_wgm_buffers(struct MSIM_AVR *mcu, MSIM_AVR_TMR *tmr)
{
	for (uint32_t i = 0; i < ARRSZ(tmr->wgm_op); i++) {
		int rc = update_wgm_buffer(mcu, tmr, i);
		if (rc != 0) {
			break;
		}
	}
}

static int
update_wgm_buffer(struct MSIM_AVR *mcu, MSIM_AVR_TMR *tmr, uint32_t i)
{
	struct MSIM_AVR_TMR_WGM *wgm = &tmr->wgm_op[i];
	int rc = 0;

	if (IS_NOWGM(wgm)) {
		rc = 1;
	} else {
		wgm->rtop_buf = IOBIT_RDA(mcu, wgm->rtop, ARRSZ(wgm->rtop));
	}

	return rc;
}

static void
update_icp_value(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr)
{
	if (!IS_IONOBIT(tmr->icp)) {
		tmr->icpval = (uint8_t)IOBIT_RD(mcu, &tmr->icp);
	}
}

/* Reset pedning interrupts */
static void
int_reset_pending(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr)
{
	struct MSIM_AVR_TMR_COMP *comp;

	for (uint32_t i = 0; i < ARRSZ(tmr->comp); i++) {
		comp = &tmr->comp[i];
		if (IS_NOCOMP(comp)) {
			break;
		} else {
			comp->iv.pending = 0;
		}
	}
}

/* Raise pending interrupts */
static void
int_raise_pending(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr)
{
	struct MSIM_AVR_TMR_COMP *comp;

	if (tmr->scnt == (tmr->presc-2U)) {
		for (uint32_t i = 0; i < ARRSZ(tmr->comp); i++) {
			comp = &tmr->comp[i];
			if (IS_NOCOMP(comp)) {
				break;
			}

			if (comp->iv.pending != 0U) {
				IOBIT_WR(mcu, &comp->iv.raised, 1);
				comp->iv.pending = 0;
			}
		}
	}
}
