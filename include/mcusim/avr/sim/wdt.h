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
 * General-purpose AVR watchdog timer. It's supposed to be suitable for any
 * AVR MCU.
 */
#ifndef MSIM_AVR_WDT_H_
#define MSIM_AVR_WDT_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "mcusim/mcusim.h"
#include "mcusim/avr/sim/io.h"
#include "mcusim/avr/sim/interrupt.h"

/* AVR Watchdog Timer */
typedef struct MSIM_AVR_WDT {
	MSIM_AVR_IOFuse wdton;		/* WDT always-on bit */
	MSIM_AVR_IOBit wde;		/* WDT system reset enable bit */
	MSIM_AVR_IOBit wdie;		/* WDT interrupt enable bit */
	MSIM_AVR_IOBit ce;		/* Change Enable bit */

	uint32_t oscf;			/* Oscillator's frequency, in Hz */
	uint32_t oscp;			/* Oscillator's prescaler */
	uint32_t scnt;			/* System clock counter */

	MSIM_AVR_IOBit wdp[4];		/* Watchdog prescaler */
	uint32_t wdp_op[16];		/* Prescalers (# of cycles) */
	uint32_t wdpval;		/* Current prescaler */

	MSIM_AVR_INTVec iv_tout;	/* Timeout interrupt vector */
	MSIM_AVR_INTVec iv_sysr;	/* System reset vector */
} MSIM_AVR_WDT;

int MSIM_AVR_WDTUpdate(struct MSIM_AVR *mcu);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_WDT_H_ */
