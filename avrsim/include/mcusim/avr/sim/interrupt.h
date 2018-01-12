/*
 * AVRSim - Simulator for AVR microcontrollers.
 * This software is a part of MCUSim, simulator for microcontrollers.
 * Copyright (C) 2017 Dmitry Salychev <darkness.bsd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MSIM_AVR_INTERRUPT_H_
#define MSIM_AVR_INTERRUPT_H_ 1

#define AVR_IRQ_NUM		64	/* AVR IRQ limit, i.e. maximum number
					   of interrupt vectors */

/*
 * Main structure to describe AVR interrupts within the simulated AVR
 * instance (reset address, IRQs, etc.)
 */
struct MSIM_AVRInt {
	unsigned long reset_pc;		/* Reset address */
	unsigned long ivt;		/* Interrupt vectors table address */
	unsigned char irq[AVR_IRQ_NUM];	/* Flags for interrupt requests */
	unsigned char exec_main;	/* Flag to execute one more
					   instruction from the main
					   program after an exit from ISR */
	unsigned char trap_at_isr;	/* Flag to enter stopped mode when
					   interrupt occured */
};

#endif /* MSIM_AVR_INTERRUPT_H_ */
