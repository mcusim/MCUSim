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
 *
 * General-purpose AVR timer. It's supposed to be suitable for any AVR MCU.
 */
#ifndef MSIM_AVR_TIMER_H_
#define MSIM_AVR_TIMER_H_ 1

#define MSIM_AVR_TMR_MAXWGM		(64)
#define MSIM_AVR_TMR_MAXCOMP		(64)
#define MSIM_AVR_TMR_STOPMODE		(-75)
#define MSIM_AVR_TMR_EXTCLK_RISE	(-76)
#define MSIM_AVR_TMR_EXTCLK_FALL	(-77)

/* Return codes of the timer functions. */
#define MSIM_AVR_TMR_OK			0
#define MSIM_AVR_TMR_NULL		75

#ifdef __cplusplus
extern "C" {
#endif

#include "mcusim/mcusim.h"
#include "mcusim/avr/sim/io.h"
#include "mcusim/avr/sim/interrupt.h"

enum {
	MSIM_AVR_TMR_WGM_None = 0,
	MSIM_AVR_TMR_WGM_Normal,	/* Normal mode */
	MSIM_AVR_TMR_WGM_CTC,		/* Clear timer on Compare Match */
	MSIM_AVR_TMR_WGM_PWM,		/* PWM */

	MSIM_AVR_TMR_WGM_FastPWM8,	/* Fast PWM, 8-bit */
	MSIM_AVR_TMR_WGM_FastPWM9,	/* Fast PWM, 9-bit */
	MSIM_AVR_TMR_WGM_FastPWM10,	/* Fast PWM, 10-bit */
	MSIM_AVR_TMR_WGM_FastPWM,	/* Fast PWM */

	MSIM_AVR_TMR_WGM_PCPWM8,	/* Phase Correct PWM, 8-bit */
	MSIM_AVR_TMR_WGM_PCPWM9,	/* Phase Correct PWM, 9-bit */
	MSIM_AVR_TMR_WGM_PCPWM10,	/* Phase Correct PWM, 10-bit */
	MSIM_AVR_TMR_WGM_PCPWM,		/* Phase Correct PWM */

	MSIM_AVR_TMR_WGM_PFCPWM,	/* Phase and frequency correct PWM */
};

/* Helps to update buffered values. */
enum {
	MSIM_AVR_TMR_UPD_ATNONE = 0,		/* Unknown value */
	MSIM_AVR_TMR_UPD_ATMAX,			/* At MAX value */
	MSIM_AVR_TMR_UPD_ATTOP,			/* At TOP value */
	MSIM_AVR_TMR_UPD_ATBOTTOM,		/* At BOTTOM value */
	MSIM_AVR_TMR_UPD_ATIMMEDIATE,		/* Immediately */
};

/* Helps to describe what to do with a pin of the output compare module. */
enum {
	MSIM_AVR_TMR_COM_DISC = 0,		/* Disconnected */
	MSIM_AVR_TMR_COM_TGONCM,		/* Toggle on Compare Match */
	MSIM_AVR_TMR_COM_CLONCM,		/* Clear on Compare Match */
	MSIM_AVR_TMR_COM_STONCM,		/* Set on Compare Match */
	/* Clear on Compare Match, Set at Bottom */
	MSIM_AVR_TMR_COM_CLONCM_STATBOT,
	/* Set on Compare Match, Clear at Bottom */
	MSIM_AVR_TMR_COM_STONCM_CLATBOT,
	/* Clear on Compare Match (up-cnt), Set on Compare Match (down-cnt) */
	MSIM_AVR_TMR_COM_CLONUP_STONDOWN,
	/* Set on Compare Match (up-cnt), Clear on Compare Match (down-cnt) */
	MSIM_AVR_TMR_COM_STONUP_CLONDOWN,
};

/* Waveform generation mode */
struct MSIM_AVR_TMR_WGM {
	uint8_t kind;				/* WGM type */
	uint8_t size;				/* Size, in bits */
	struct MSIM_AVR_IOBit rtop[4];		/* Register as TOP value */
	int32_t top;				/* Fixed TOP value */
	uint32_t bottom;			/* Fixed BOTTOM value */
	uint8_t updocr_at;			/* Update OCR at */
	uint8_t settov_at;			/* Set TOV at */
};

/* Timer comparator */
struct MSIM_AVR_TMR_COMP {
	struct MSIM_AVR_TMR *owner;		/* Parent timer */
	struct MSIM_AVR_IOBit ocr[4];		/* Comparator register */
	struct MSIM_AVR_IOBit pin;		/* Pin to output waveform */
	struct MSIM_AVR_IOBit ddp;		/* Data direction for pin */

	struct MSIM_AVR_IOBit com;		/* Comparator output mode */
	uint8_t com_op[16][16];			/* mode: [WGM][COM] */

	struct MSIM_AVR_INTVec iv;		/* Interrupt vector */
};

/* The main AVR timer structure. */
struct MSIM_AVR_TMR {
	struct MSIM_AVR_IOBit tcnt[4];		/* Timer counter */
	struct MSIM_AVR_IOBit disabled;		/* "disabled" bit */

	struct MSIM_AVR_IOBit cs[4];		/* Clock source */
	uint8_t cs_div[16];			/* CS bits to prescaler */
	uint32_t cs_dival;			/* Current prescaler */

	struct MSIM_AVR_IOBit ec_pin;		/* External clock pin */
	uint8_t ec_vold;			/* Old value of the ec pin */
	uint32_t ec_flags;			/* External clock flags */

	struct MSIM_AVR_IOBit wgm[4];		/* Waveform generation mode */
	struct MSIM_AVR_TMR_WGM wgm_op[MSIM_AVR_TMR_MAXWGM]; /* WGM types */
	uint8_t wgm_mode;			/* Current WGM type (index) */

	struct MSIM_AVR_IOBit icr[4];		/* Input capture register */
	struct MSIM_AVR_IOBit icp;		/* Input capture pin */
	struct MSIM_AVR_IOBit ices[4];		/* Input capture edge select */

	struct MSIM_AVR_INTVec iv_ovf;		/* Overflow */
	struct MSIM_AVR_INTVec iv_ic;		/* Input capture */

	struct MSIM_AVR_TMR_COMP comp[MSIM_AVR_TMR_MAXCOMP];
};

int MSIM_AVR_TMRUpdate(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_TIMER_H_ */
