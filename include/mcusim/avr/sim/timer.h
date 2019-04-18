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

/* Maxiimum output compare channels. */
#define MSIM_AVR_TMR_MAXOC		64

#ifdef __cplusplus
extern "C" {
#endif

#include "mcusim/mcusim.h"

/* Forward declaration of the structures. */
struct MSIM_AVR;
struct MSIM_AVR_TMR;
struct MSIM_AVR_TMRParms;

/* Callback function type for the AVR timer. */
typedef int (*MSIM_AVR_TMRFunc_f)(struct MSIM_AVR *mcu,
                                  struct MSIM_AVR_TMR *tmr,
                                  struct MSIM_AVR_TMRParms *p);

/* Timer events. */
enum MSIM_AVR_TMREvent {
	TMR_E_UPDATED,		/* Timer has been updated. */
	TMR_E_OCM		/* Output compare match. */
};

/* The main AVR timer structure. */
struct MSIM_AVR_TMR {
	uint32_t id;		/* Timer ID. */
	uint32_t tmax;		/* Maximum (MAX) value of the timer. */
	uint32_t ttop;		/* Current TOP value of the timer. */
	uint32_t tbot;		/* Current BOTTOM value of the timer. */
	uint8_t stop_mode;
	uint32_t presc;		/* Current prescaler. */
	uint32_t ticks;		/* Number of internal ticks of the timer. */

	/* Output compare values for the different compare channels. */
	uint32_t oc[MSIM_AVR_TMR_MAXOC];
	/* Buffer for the output compare values. */
	uint32_t oc_buf[MSIM_AVR_TMR_MAXOC];
};

/* Parameters for the callback functions. */
struct MSIM_AVR_TMRParms {
	uint32_t dummy;
};

int MSIM_AVR_TMRInit(struct MSIM_AVR_TMR *tmr);
int MSIM_AVR_TMRUpdate(struct MSIM_AVR *mcu, struct MSIM_AVR_TMR *tmr);
int MSIM_AVR_TMROnEvent(enum MSIM_AVR_TMREvent e, MSIM_AVR_TMRFunc_f h);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_TIMER_H_ */
