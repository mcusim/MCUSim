/*
 * AVRSim - Simulator for AVR microcontrollers.
 * This software is a part of MCUSim, interactive simulator for
 * microcontrollers.
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
#ifndef MSIM_AVR_SIMCORE_H_
#define MSIM_AVR_SIMCORE_H_ 1

#include <stdio.h>
#include <stdint.h>

#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/bootloader.h"
#include "mcusim/avr/sim/init.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Main simulation routine. It performs a required number of
 * steps (instructions).
 *
 * Zero number of steps could be used to run an infinite simulation
 * (until the end of the program or first breakpoint appeared).
 * The infinite simulation could be interrupted by the execution process
 * reached the given address. Addresses within the program space are taken
 * into account, only.*/
int MSIM_SimulateAVR(struct MSIM_AVR *mcu, unsigned long steps,
		     unsigned long addr);

/* Initializes an MCU into specific model determined by the given name.
 * It is, generally, a good idea to prepare specific MCU model using this
 * function instead of MSIM_XXXInit() ones. */
int MSIM_InitAVR(struct MSIM_AVR *mcu, const char *mcu_name,
		 unsigned char *pm, unsigned long pm_size,
		 unsigned char *dm, unsigned long dm_size,
		 FILE *fp);

/* Functions to work with a stack inside MCU */
void MSIM_StackPush(struct MSIM_AVR *mcu, uint8_t val);

uint8_t MSIM_StackPop(struct MSIM_AVR *mcu);

/* Functions to update/read SREG bits */
void MSIM_UpdateSREGFlag(struct MSIM_AVR *mcu, enum MSIM_AVRSREGFlag flag,
			 unsigned char set_f);
unsigned char MSIM_ReadSREGFlag(struct MSIM_AVR *mcu,
				enum MSIM_AVRSREGFlag flag);

/* Prints instructions from the program memory of the MCU.
 *
 * Required interval of instructions can be specified via
 * [start_addr, end_addr] (steps = 0) or as a number of instructions
 * from the current program counter (steps != 0). */
int MSIM_PrintInstructions(struct MSIM_AVR *mcu, unsigned long start_addr,
			   unsigned long end_addr, unsigned long steps);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_SIMCORE_H_ */
