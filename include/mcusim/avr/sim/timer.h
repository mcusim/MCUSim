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

/* Maximum output compare channels. */
#define MSIM_AVR_TMR_MAXOC		64
/* Maximum prescaler values. */
#define MSIM_AVR_TMR_MAXPRESC		64
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

/* The main AVR timer structure. */
struct MSIM_AVR_TMR {
	uint32_t max;		/* Maximum value of the timer. */
	uint32_t top;		/* Current TOP value of the timer. */
	uint32_t bot;		/* Current BOTTOM value of the timer. */

	uint32_t sval;		/* Current timer value (prescaler is 1) */
	uint32_t val;		/* Current timer value */
	uint32_t presc;		/* Current prescaler */
	uint8_t mode;		/* Current timer mode */

	uint32_t oc[MSIM_AVR_TMR_MAXOC]; /* Output compare values */
	uint32_t oc_buf[MSIM_AVR_TMR_MAXOC]; /* Output compare buffers */
	uint32_t oclen;		/* Number of output compare channels */

	/* Pairs of the arbitrary values and prescaler values.
	 * It helps to map an arbitrary value of the register which controls
	 * the timer's prescaler to the actual prescaler value. */
	struct {
		uint32_t key;
		int32_t presc;
	} pv[MSIM_AVR_TMR_MAXPRESC];
	uint32_t pvlen;		/* Number of the prescaler values. */
};

int MSIM_AVR_TMRUpdate(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_TIMER_H_ */
