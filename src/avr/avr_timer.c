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
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "mcusim/mcusim.h"
#include "mcusim/avr/sim/timer.h"
#include "mcusim/avr/sim/private/macro.h"
#include "mcusim/avr/sim/private/io_macro.h"

#define FAKE_WGM8 {							\
	.kind = WGM_NORMAL,						\
	.size = 8,							\
	.top = 0xFF,							\
	.updocr_at = UPD_ATIMMEDIATE,					\
	.settov_at = UPD_ATMAX,						\
};

#define FAKE_WGM16 {							\
	.kind = WGM_NORMAL,						\
	.size = 16,							\
	.top = 0xFFFF,							\
	.updocr_at = UPD_ATIMMEDIATE,					\
	.settov_at = UPD_ATMAX,						\
};

static int update_timer(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr);
static void mode_normal(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr);

static void update_ocr_buffers(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr);
static int update_ocr_buffer(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr,
                             uint32_t i);
static void reset_pending_ocint(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr);

static void trigger_oc_pin(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr,
                           struct MSIM_AVR_TMR_COMP *comp);

int MSIM_AVR_TMRUpdate(struct MSIM_AVR *mcu)
{
	struct MSIM_AVR_TMR *tmr;
	int rc = 0;

	for (uint32_t i = 0; i < MSIM_AVR_MAXTMRS; i++) {
		tmr = &mcu->timers[i];
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

static int update_timer(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr)
{
	struct MSIM_AVR_TMR_WGM wgm8 = FAKE_WGM8;
	struct MSIM_AVR_TMR_WGM wgm16 = FAKE_WGM16;
	uint32_t cs, wgm, dis;
	int rc = 0;

	do {
		/* Timer can be undefined or disabled by firmware */
		if (IS_IONOBITA(tmr->cs)) {
			tmr->scnt = 0;
			break;
		}
		if (!IS_IONOBYTE(tmr->disabled)) {
			dis = IOBIT_RD(mcu, &tmr->disabled);
			if (dis != 0U) {
				tmr->scnt = 0;
				break;
			}
		}

		/* Obtain timer's clock source */
		cs = IOBIT_RDA(mcu, tmr->cs, ARR_LEN(tmr->cs));
		if (cs >= ARR_LEN(tmr->cs_div)) {
			tmr->scnt = 0;
			break;
		}
		if (cs == 0U) {
			/* Stopped mode */
			tmr->scnt = 0;
			tmr->presc = 1;
			update_ocr_buffers(mcu, tmr);
			reset_pending_ocint(mcu, tmr);
			break;
		}
		tmr->presc = 1<<(tmr->cs_div[cs]);

		/* Obtain timer's waveform generation mode */
		if (IS_IONOBITA(tmr->wgm)) {
			tmr->wgmval = (tmr->size == 16) ? &wgm16 : &wgm8;
			tmr->wgmi = -1;
		} else {
			wgm = IOBIT_RDA(mcu, tmr->wgm, ARR_LEN(tmr->wgm));
			tmr->wgmval = &tmr->wgm_op[wgm];
			tmr->wgmi = (int32_t)wgm;
		}

		/* Clock source changed */
		if (tmr->presc != (1<<(tmr->cs_div[cs]))) {
			tmr->presc = 1<<(tmr->cs_div[cs]);
		}

		switch (tmr->wgmval->kind) {
		case WGM_NORMAL:
			mode_normal(mcu, tmr);
			break;
		default:
			break;
		}
	} while (0);

	return rc;
}

static void mode_normal(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr)
{
	struct MSIM_AVR_TMR_WGM *wgm = tmr->wgmval;
	struct MSIM_AVR_TMR_COMP *comp;
	uint32_t tcnt, ocr;
	uint32_t top = wgm->top;

	/* Raise pending interrupts */
	if (tmr->scnt == (tmr->presc-2U)) {
		for (uint32_t i = 0; i < ARR_LEN(tmr->comp); i++) {
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

	if (tmr->scnt < (tmr->presc-1U)) {
		tmr->scnt++;
	} else {
		/* Current timer value */
		tcnt = IOBIT_RDA(mcu, tmr->tcnt, ARR_LEN(tmr->tcnt));

		/* Output Compare and Compare Match units.
		 * Compares current timer value with OC registers. */
		for (uint32_t i = 0; i < ARR_LEN(tmr->comp); i++) {
			comp = &tmr->comp[i];
			if (IS_NOCOMP(comp)) {
				break;
			}

			/* Raise pending interrupt */
			if (comp->iv.pending != 0U) {
				IOBIT_WR(mcu, &comp->iv.raised, 1);
				comp->iv.pending = 0;
			}

			ocr = (wgm->updocr_at == UPD_ATIMMEDIATE)
			      ? IOBIT_RDA(mcu, comp->ocr, ARR_LEN(comp->ocr))
			      : comp->ocr_buf;
			if (tcnt == ocr) {
				/* Compare match. Interrupt flag should be set
				 * at the next timer clock cycle! */
				comp->iv.pending = 1;
				trigger_oc_pin(mcu, tmr, comp);
			}
		}

		/* Input Capture Unit.
		 * Watches an ICP (input capture pin) or ACO (analog comparator
		 * output).
		 *
		 * It's active only in the Waveform Generation modes which
		 * utilize the ICP register as their TOP value. */
		/* ... */

		/* Counter Unit. */
		if (tcnt == top) {
			tcnt = 0;
			IOBIT_WR(mcu, &tmr->iv_ovf.raised, 1);
		} else {
			tcnt++;
		}
		tmr->scnt = 0;
		IOBIT_WRA(mcu, tmr->tcnt, ARR_LEN(tmr->tcnt), tcnt);
	}
}

static void trigger_oc_pin(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr,
                           struct MSIM_AVR_TMR_COMP *comp)
{
}

static void update_ocr_buffers(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr)
{
	int rc;

	for (uint32_t i = 0; i < ARR_LEN(tmr->comp); i++) {
		rc = update_ocr_buffer(mcu, tmr, i);
		if (rc != 0) {
			break;
		}
	}
}

static int update_ocr_buffer(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr,
                             uint32_t i)
{
	struct MSIM_AVR_TMR_COMP *comp = &tmr->comp[i];
	int rc = 0;

	if (IS_NOCOMP(comp)) {
		rc = 1;
	} else {
		comp->ocr_buf = IOBIT_RDA(mcu, comp->ocr, ARR_LEN(comp->ocr));
	}

	return rc;
}

static void reset_pending_ocint(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr)
{
	struct MSIM_AVR_TMR_COMP *comp;

	for (uint32_t i = 0; i < ARR_LEN(tmr->comp); i++) {
		comp = &tmr->comp[i];
		if (IS_NOCOMP(comp)) {
			break;
		} else {
			comp->iv.pending = 0;
		}
	}
}
