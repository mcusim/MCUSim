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
 * General-purpose AVR timer.
 *
 * It's supposed to be suitable for any AVR MCU.
 */
#ifndef MSIM_AVR_TIMER_H_
#define MSIM_AVR_TIMER_H_ 1

#define MSIM_AVR_TMR_MAXCOMP		(64)	/* Max output compare chan. */
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
	MSIM_AVR_TMR_WGM_None,
	MSIM_AVR_TMR_WGM_Normal,	/* Normal mode */
	MSIM_AVR_TMR_WGM_CTC,		/* Clear timer on Compare Match */
	MSIM_AVR_TMR_WGM_PWM,		/* PWM */
	MSIM_AVR_TMR_WGM_FastPWM,	/* Fast PWM */
	MSIM_AVR_TMR_WGM_PCPWM,		/* Phase Correct PWM */
	MSIM_AVR_TMR_WGM_PFCPWM,	/* Phase and frequency correct PWM */
};

struct MSIM_AVR_TMR_WGM {
	uint32_t top;
	uint32_t bottom;
	uint8_t size;
	uint8_t kind;
};

/* Timer comparator */
struct MSIM_AVR_TMR_COMP {
	struct MSIM_AVR_TMR *owner;		/* Parent timer */
	struct MSIM_AVR_INTVec interrupt;	/* Interrupt vector */
	struct MSIM_AVR_IOBit ocr[2];		/* Comparator register */
	struct MSIM_AVR_IOBit com;		/* Comparator output mode */
	struct MSIM_AVR_IOBit com_pin;
};

/* The main AVR timer structure. */
struct MSIM_AVR_TMR {
	struct MSIM_AVR_IOBit tcnt[4];		/* Timer counter */
	struct MSIM_AVR_IOBit icr[4];		/* Input capture */
	struct MSIM_AVR_IOBit disabled;		/* "disabled" bit */

	struct MSIM_AVR_IOBit cs[4];		/* Clock source */
	uint8_t cs_div[16];			/* CS bits to prescaler */
	uint32_t cs_dival;			/* Current prescaler */

	struct MSIM_AVR_IOBit ec_pin;		/* External clock pin */
	uint8_t ec_vold;			/* Old value of the ec pin */
	uint32_t ec_flags;

	struct MSIM_AVR_IOBit wgm[4];		/* Waveform generation mode */
	struct MSIM_AVR_TMR_WGM wgm_op[16];	/* WGM types */
	struct MSIM_AVR_TMR_WGM wgm_mode;	/* Current WGM type */

	struct MSIM_AVR_INTVec overflow;
	struct MSIM_AVR_INTVec in_capture;

	struct MSIM_AVR_TMR_COMP comp[MSIM_AVR_TMR_MAXCOMP];
};

int MSIM_AVR_TMRUpdate(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_TIMER_H_ */
