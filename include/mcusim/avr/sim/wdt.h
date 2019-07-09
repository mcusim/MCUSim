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

/*
 * Model-independent AVR watchdog timer.
 * It's supposed to be suitable for any AVR MCU.
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
