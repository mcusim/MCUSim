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
#ifndef MSIM_AVR_INTERRUPT_H_
#define MSIM_AVR_INTERRUPT_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "mcusim/avr/sim/io.h"

/* AVR IRQ limit, i.e. maximum number of interrupt vectors. */
#define MSIM_AVR_IRQNUM			64

/* Forward declaration of the structure to describe AVR microcontroller
 * instance. */
struct MSIM_AVR;

/* Main structure to describe AVR interrupts within the simulated AVR
 * instance (reset address, IRQs, etc.) */
typedef struct MSIM_AVR_INT {
	uint32_t reset_pc;		/* Reset address */
	uint32_t ivt;			/* Interrupt vectors table address */
	uint8_t irq[MSIM_AVR_IRQNUM];	/* Flags for interrupt requests */
	uint8_t exec_main;		/* Exe instruction from the main
					   program after an exit from ISR */
	uint8_t trap_at_isr;		/* Flag to enter stopped mode when
					   interrupt occured */
} MSIM_AVR_INT;

/* Interrupt vector */
typedef struct MSIM_AVR_INTVec {
	MSIM_AVR_IOBit enable;		/* Interrupt "enabled" flag */
	MSIM_AVR_IOBit raised;		/* Interrupt flag */
	uint8_t vector;			/* Interrupt address (in IVT) */
	uint8_t pending;		/* Pending interrupt flag */
} MSIM_AVR_INTVec;

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_INTERRUPT_H_ */

